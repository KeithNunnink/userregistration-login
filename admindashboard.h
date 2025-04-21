#ifndef ADMINDASHBOARD_H
#define ADMINDASHBOARD_H

#include <QDialog>
#include <QList>
#include "user.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class AdminDashboard;
}
QT_END_NAMESPACE

class AdminDashboard : public QDialog {
    Q_OBJECT

public:
    explicit AdminDashboard(QList<User>& userList, QWidget *parent = nullptr);
    ~AdminDashboard();

private slots:
    void on_pushButton_Logout_clicked();
    void on_pushButton_ViewUser_clicked();
    void on_pushButton_ResetPassword_clicked();
    void on_pushButton_ViewUserHistory_clicked();
    void on_pushButton_DeleteUser_clicked();
    void on_pushButton_ToggleAdmin_clicked();
    void on_pushButton_UnlockAccount_clicked();

signals:
    void passwordChanged();
    void userDeleted();
    void adminStatusChanged();
    void accountUnlocked();

private:
    Ui::AdminDashboard *ui;
    QList<User>& users;  // Reference to original user list
};

#endif // ADMINDASHBOARD_H
