#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "registerwindow.h"
#include "user.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loginUser();
    void openRegisterWindow();
    void handleRegistration(QString username, QString password, QString email, QString studentID);
    void forgotPassword(); // ✅ Added for password recovery

private:
    Ui::MainWindow *ui;
    RegisterWindow *registerWindow;
    QList<User> registeredUsers; // ✅ Supports multiple users
    void saveUserData(); // Keep these methods for backward compatibility, but they'll just call our consolidated functions
    void loadUserData(); // Keep these methods for backward compatibility, but they'll just call our consolidated functions
    void saveUsersToCsv();
    void loadUsersFromCsv();
};

#endif // MAINWINDOW_H
