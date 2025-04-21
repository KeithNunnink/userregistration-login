#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dashboardwindow.h"
#include "admindashboard.h"
#include "passwordutils.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QFileInfo> // Include QFileInfo for file path operations
#include <QDebug>

const QString USER_DATA_FILE = "data.csv";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Replace any line like this:
    // loadUserData();
    // With:
    loadUsersFromCsv();

    connect(ui->pushButton_Register, &QPushButton::clicked, this, &MainWindow::openRegisterWindow);
    connect(ui->pushButton_Login, &QPushButton::clicked, this, &MainWindow::loginUser);
    connect(ui->pushButton_ForgotPassword, &QPushButton::clicked, this, &MainWindow::forgotPassword);
}

MainWindow::~MainWindow()
{
    // Save user data on application exit
    saveUserData();
    delete ui;
}

void MainWindow::openRegisterWindow()
{
    registerWindow = new RegisterWindow(this);

    connect(registerWindow, &RegisterWindow::registrationComplete,
            this, &MainWindow::handleRegistration);

    registerWindow->exec();
}

void MainWindow::handleRegistration(QString username, QString password, QString email, QString studentID)
{
    username = username.trimmed().toLower(); // Convert to lowercase
    password = password.trimmed();
    email = email.trimmed();
    studentID = studentID.trimmed();

    for (const User &user : registeredUsers) {
        if (user.username.toLower() == username) {
            QMessageBox::warning(this, "Registration Failed", "Username already exists.");
            return;
        }
    }

    // Hash the password before storing
    QString hashedPassword = PasswordUtils::hashPassword(password);

    User newUser;
    newUser.username = username;
    newUser.password = hashedPassword;  // Store hashed password
    newUser.email = email;
    newUser.studentID = studentID;

    registeredUsers.append(newUser);

    // Save user data after registration
    saveUserData();

    // Display the full path of the .csv file
    QString fullPath = QFileInfo(USER_DATA_FILE).absoluteFilePath();
    QMessageBox::information(this, "Registration Successful",
                             "Registration complete. You may now log in.\n\nUser data saved to:\n" + fullPath);

    qDebug() << "[REGISTERED]" << newUser.username << newUser.password;
    qDebug() << "[DEBUG] User registered:" << newUser.username
             << "| Total users now:" << registeredUsers.size();

    QMessageBox::information(this, "Success", "Registration complete. You may now log in.");
}

void MainWindow::loginUser()
{
    QString uName = ui->lineEdit_Username->text().trimmed().toLower(); // Convert to lowercase
    QString uPassword = ui->lineEdit_Password->text().trimmed();

    qDebug() << "[DEBUG] Login attempt:" << uName << uPassword;
    qDebug() << "[DEBUG] Registered users:" << registeredUsers.size();

    for (User &user : registeredUsers) {
        qDebug() << "[LOGIN ATTEMPT]" << uName << uPassword;
        qDebug() << "[CHECKING AGAINST]" << user.username << user.password;

        if (uName == user.username.toLower()) { // Case-insensitive comparison
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            
            // Check if account is locked
            if (user.accountLocked) {
                user.loginAttempts.append(timestamp + " - LOCKED");
                QMessageBox::warning(this, "Account Locked", 
                    "This account has been locked due to too many failed login attempts. "
                    "Please contact an administrator to unlock it.");
                saveUsersToCsv(); // Save updated login attempts
                return;
            }

            // TEMPORARY FIX: Check for both hashed and unhashed passwords for backward compatibility
            bool passwordMatch = false;
            
            // First check if stored password is already hashed (starts with a hash pattern)
            if (user.password.length() >= 64) { // SHA-256 hash is 64 hex chars
                // Compare hashed passwords
                QString hashedPassword = PasswordUtils::hashPassword(uPassword);
                passwordMatch = (hashedPassword == user.password);
            } else {
                // Legacy check - direct comparison for non-hashed passwords
                passwordMatch = (uPassword == user.password);
                
                // If match found with unhashed password, update to hashed version
                if (passwordMatch) {
                    user.password = PasswordUtils::hashPassword(uPassword);
                    saveUsersToCsv(); // Save the updated hashed password
                }
            }
            
            if (passwordMatch) {
                user.loginAttempts.append(timestamp + " - SUCCESS");
                user.failedAttempts = 0; // Reset failed attempts on success
                qDebug() << "[DEBUG] Login success!";
                saveUsersToCsv(); // Save updated login attempts and reset counter
                
                ui->lineEdit_Username->clear();
                ui->lineEdit_Password->clear();
                if (user.admin == true)
                {
                    //Admin window show
                    AdminDashboard *dashboard = new AdminDashboard(registeredUsers, this);
                    connect(dashboard, &AdminDashboard::passwordChanged, this, &MainWindow::saveUserData);
                    dashboard->setModal(true);
                    dashboard->exec();
                } else {
                    DashboardWindow *dashboard = new DashboardWindow(user, this);
                    dashboard->exec();
                }
                return;
            } else {
                // Increment failed attempts
                user.failedAttempts++;
                user.loginAttempts.append(timestamp + " - FAILED");
                
                // Check if account should be locked
                if (user.failedAttempts >= 3) {
                    user.accountLocked = true;
                    user.loginAttempts.append(timestamp + " - ACCOUNT LOCKED");
                    QMessageBox::warning(this, "Account Locked", 
                        "Your account has been locked due to too many failed login attempts. "
                        "Please contact an administrator to unlock it.");
                } else {
                    QMessageBox::warning(this, "Login Failed", 
                        QString("Incorrect password. %1 attempts remaining before account lockout.")
                        .arg(3 - user.failedAttempts));
                }
                
                saveUserData(); // Save updated failed attempts
                return;
            }
        }
    }

    QMessageBox::warning(this, "Login Failed", "Invalid username or password!");
}

void MainWindow::forgotPassword()
{
    bool ok;
    QString email = QInputDialog::getText(this, "Recover Account", "Enter your email:", QLineEdit::Normal, "", &ok);
    if (!ok || email.isEmpty()) return;

    QString studentID = QInputDialog::getText(this, "Recover Account", "Enter your student ID:", QLineEdit::Normal, "", &ok);
    if (!ok || studentID.isEmpty()) return;

    for (User &user : registeredUsers) {
        if (user.email == email.trimmed() && user.studentID == studentID.trimmed()) {
            QString newPassword = QInputDialog::getText(this, "Reset Password", "Enter new password:", QLineEdit::Password, "", &ok);
            if (!ok || newPassword.isEmpty()) {
                QMessageBox::warning(this, "Error", "Password cannot be empty.");
                return;
            }

            user.password = newPassword.trimmed();
            QMessageBox::information(this, "Success", "Your password has been reset.");
            return;
        }
    }

    QMessageBox::warning(this, "Error", "No matching user found.");
}

// Save all registered users to a CSV file
void MainWindow::saveUserData() 
{
    // Simply call our consolidated function instead
    saveUsersToCsv();
}

// Load all registered users from a CSV file
void MainWindow::loadUserData() 
{
    // Simply call our consolidated function instead
    loadUsersFromCsv();
}

void MainWindow::saveUsersToCsv()
{
    QFile file("users.csv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        
        // Write a header row with column names
        out << "Username,Password,Email,StudentID,Admin,FailedAttempts,AccountLocked,LoginAttempts\n";
        
        for (const User &user : registeredUsers) {
            // Write basic user data
            out << user.username << "," 
                << user.password << "," 
                << user.email << "," 
                << user.studentID << "," 
                << (user.admin ? "1" : "0") << "," 
                << user.failedAttempts << "," 
                << (user.accountLocked ? "1" : "0");
                
            // Add login attempts in the same row
            for (const QString &attempt : user.loginAttempts) {
                // Create a copy and escape any commas
                QString escapedAttempt = QString(attempt).replace(",", "\\,");
                out << "," << escapedAttempt;
            }
            out << "\n";
        }
        
        file.close();
    }
}

void MainWindow::loadUsersFromCsv()
{
    QFile file("users.csv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        
        // Skip the header row
        if (!in.atEnd()) in.readLine();
        
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(",");
            
            if (fields.size() >= 7) {  // We expect at least 7 fields
                User user;
                user.username = fields[0];
                user.password = fields[1];
                user.email = fields[2];
                user.studentID = fields[3];
                user.admin = (fields[4] == "1");
                user.failedAttempts = fields[5].toInt();
                user.accountLocked = (fields[6] == "1");
                
                // Load login attempts (if any)
                for (int i = 7; i < fields.size(); i++) {
                    QString attempt = fields[i];
                    // Unescape commas
                    attempt.replace("\\,", ",");
                    user.loginAttempts.append(attempt);
                }
                
                registeredUsers.append(user);
            }
        }
        
        file.close();
    }
}
