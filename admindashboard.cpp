#include "admindashboard.h"
#include "ui_admindashboard.h"
#include "user.h"
#include "passwordutils.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>  // Add this include for QDateTime

AdminDashboard::AdminDashboard(QList<User> &userList, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::AdminDashboard),
      users(userList)
{
    ui->setupUi(this);
}

AdminDashboard::~AdminDashboard()
{
    delete ui;
}

void AdminDashboard::on_pushButton_ViewUser_clicked()
{
    QString allUserInfo;

    for (const User &user : users) {
        // Find the last successful login by parsing the login attempts
        QString lastLogin = "Never";
        for (int i = user.loginAttempts.size() - 1; i >= 0; i--) {
            if (user.loginAttempts[i].contains("SUCCESS", Qt::CaseInsensitive)) {
                // Extract the timestamp part before " - SUCCESS"
                lastLogin = user.loginAttempts[i].left(user.loginAttempts[i].indexOf(" - "));
                break;
            }
        }

        allUserInfo += QString("Username: %1\nEmail: %2\nStudent ID: %3\nAccount Type: %4\n"
                              "Last Login: %5\nAccount Status: %6\nFailed Attempts: %7\n\n")
            .arg(user.username)
            .arg(user.email)
            .arg(user.studentID)
            .arg(user.admin ? "Admin" : "User")
            .arg(lastLogin)
            .arg(user.accountLocked ? "Locked" : "Active")
            .arg(user.failedAttempts);
    }

    ui->textEdit_UserInfo->setText(allUserInfo);
}

void AdminDashboard::on_pushButton_ResetPassword_clicked()
{
    bool ok;
    QString username = QInputDialog::getText(this, "Reset Password", "Enter username to reset:", QLineEdit::Normal, "", &ok);

    if (!ok || username.isEmpty()) return;
    
    username = username.trimmed().toLower(); // Convert to lowercase

    for (User &user : users) {
        if (user.username.toLower() == username) {
            QString newPassword = QInputDialog::getText(this, "New Password", "Enter new password:", QLineEdit::Password, "", &ok);
            if (ok && !newPassword.isEmpty()) {
                // Hash the new password before storing
                user.password = PasswordUtils::hashPassword(newPassword.trimmed());
                
                // Reset failed attempts and unlock account
                user.failedAttempts = 0;
                user.accountLocked = false;
                
                QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                user.loginAttempts.append(timestamp + " - PASSWORD RESET BY ADMIN");
                
                QMessageBox::information(this, "Success", "Password reset successfully and account unlocked.");
                emit passwordChanged();
            } else {
                QMessageBox::warning(this, "Error", "Password reset cancelled or empty.");
            }
            return;
        }
    }

    QMessageBox::warning(this, "Error", "User not found.");
}

void AdminDashboard::on_pushButton_Logout_clicked()
{
    this->close();
}

void AdminDashboard::on_pushButton_ViewUserHistory_clicked()
{
    bool ok;
    QString username = QInputDialog::getText(this, "View User History", "Enter username:", QLineEdit::Normal, "", &ok);

    if (!ok || username.isEmpty()) return;

    for (const User &user : users) {
        if (user.username == username.trimmed()) {
            QString history = QString("Login History for %1:\n\n").arg(user.username);
            if (user.loginAttempts.isEmpty()) {
                history += "No login history available.";
            } else {
                history += user.loginAttempts.join("\n");
            }
            
            ui->textEdit_UserInfo->setText(history);
            return;
        }
    }

    QMessageBox::warning(this, "Error", "User not found.");
}

void AdminDashboard::on_pushButton_DeleteUser_clicked()
{
    bool ok;
    QString username = QInputDialog::getText(this, "Delete User", "Enter username to delete:", QLineEdit::Normal, "", &ok);

    if (!ok || username.isEmpty()) return;
    
    username = username.trimmed().toLower(); // Convert to lowercase

    // Find the user by username
    for (int i = 0; i < users.size(); i++) {
        if (users[i].username.toLower() == username) {
            // Confirm deletion
            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, 
                "Confirm Deletion", 
                QString("Are you sure you want to delete user '%1'?").arg(username),
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (confirm == QMessageBox::Yes) {
                users.removeAt(i);
                QMessageBox::information(this, "Success", QString("User '%1' has been deleted.").arg(username));
                emit userDeleted();
                // Refresh the user list display
                on_pushButton_ViewUser_clicked();
            }
            return;
        }
    }

    QMessageBox::warning(this, "Error", "User not found.");
}

void AdminDashboard::on_pushButton_ToggleAdmin_clicked()
{
    bool ok;
    QString username = QInputDialog::getText(this, "Change Admin Status", "Enter username:", QLineEdit::Normal, "", &ok);

    if (!ok || username.isEmpty()) return;
    
    username = username.trimmed().toLower(); // Convert to lowercase

    for (User &user : users) {
        if (user.username.toLower() == username) {
            // Get current and new status descriptions
            QString currentStatus = user.admin ? "an administrator" : "a user";
            QString newStatus = user.admin ? "a user" : "an administrator";
            
            // Show confirmation dialog with current and potential new status
            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, 
                "Confirm Admin Status Change",
                QString("The user '%1' is currently %2. Are you sure you want to change them to %3?")
                    .arg(username)
                    .arg(currentStatus)
                    .arg(newStatus),
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (confirm == QMessageBox::Yes) {
                // Toggle admin status
                user.admin = !user.admin;
                QMessageBox::information(this, "Status Changed", 
                    QString("User '%1' is now %2.").arg(username).arg(newStatus));
                
                emit adminStatusChanged();
                
                // Update the display
                QString userInfo = QString("Username: %1\nEmail: %2\nStudent ID: %3\nAdmin: %4")
                    .arg(user.username)
                    .arg(user.email)
                    .arg(user.studentID)
                    .arg(user.admin ? "Yes" : "No");
                
                ui->textEdit_UserInfo->setText(userInfo);
            }
            return;
        }
    }

    QMessageBox::warning(this, "Error", "User not found.");
}

void AdminDashboard::on_pushButton_UnlockAccount_clicked()
{
    bool ok;
    QString username = QInputDialog::getText(this, "Unlock Account", "Enter username to unlock:", QLineEdit::Normal, "", &ok);

    if (!ok || username.isEmpty()) return;
    
    username = username.trimmed().toLower(); // Convert to lowercase

    for (User &user : users) {
        if (user.username.toLower() == username) {
            if (!user.accountLocked && user.failedAttempts == 0) {
                QMessageBox::information(this, "Account Status", "This account is already unlocked and has no failed attempts.");
                return;
            }
            
            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, 
                "Confirm Unlock",
                QString("Are you sure you want to unlock the account for '%1' and reset failed attempts?").arg(username),
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (confirm == QMessageBox::Yes) {
                user.accountLocked = false;
                user.failedAttempts = 0;
                
                // Add entry to login attempts
                QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                user.loginAttempts.append(timestamp + " - ACCOUNT UNLOCKED BY ADMIN");
                
                QMessageBox::information(this, "Success", "Account unlocked successfully.");
                emit accountUnlocked();
                
                // Show the updated user info
                QString userInfo = QString("Username: %1\nEmail: %2\nStudent ID: %3\nAccount Type: %4\nAccount Status: Unlocked\nFailed Attempts: 0")
                    .arg(user.username)
                    .arg(user.email)
                    .arg(user.studentID)
                    .arg(user.admin ? "Admin" : "User");
                
                ui->textEdit_UserInfo->setText(userInfo);
            }
            return;
        }
    }

    QMessageBox::warning(this, "Error", "User not found.");
}
