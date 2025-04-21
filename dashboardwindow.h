#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include <QDialog>
#include "user.h"

namespace Ui {
class DashboardWindow;
}

class DashboardWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DashboardWindow(User &user, QWidget *parent = nullptr);
    ~DashboardWindow();

private slots:
    void on_pushButton_ViewInfo_clicked();
    void on_pushButton_Logout_clicked();
    void on_pushButton_ChangePassword_clicked();

private:
    Ui::DashboardWindow *ui;
    User &loggedInUser;  // ✅ reference to original user
};

#endif // DASHBOARDWINDOW_H
