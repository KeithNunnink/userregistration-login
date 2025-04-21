#pragma message("✅ registerwindow.cpp is compiling")

#include "registerwindow.h"
#include "ui_registerwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

RegisterWindow::RegisterWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterWindow)
{
    ui->setupUi(this);
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

// When the user clicks "Register"
void RegisterWindow::on_pushButton_Submit_clicked()
{
    QString username = ui->lineEdit_Username->text().trimmed();
    QString password = ui->lineEdit_Password->text().trimmed();
    QString email = ui->lineEdit_Email->text().trimmed();
    QString studentID = ui->lineEdit_StudentID->text().trimmed();

    if (username.isEmpty() || password.isEmpty() || email.isEmpty() || studentID.isEmpty()) {
        QMessageBox::warning(this, "Registration Failed", "All fields must be filled out.");
        return;
    }

    // 🔍 Debug output
    qDebug() << "[DEBUG] Register form submitted:";
    qDebug() << "Username:" << username << "Password:" << password;
    qDebug() << "Email:" << email << "StudentID:" << studentID;

    emit registrationComplete(username, password, email, studentID);

    this->close();
}
