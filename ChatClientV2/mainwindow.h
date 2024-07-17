#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "connect.h"
#include "qboxlayout.h"
#include "qmessagebox.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, Connect* connectToServer = nullptr);
    ~MainWindow();

    void setCurrentUserForMessage(QString name);
    void removeCurrentUserForMessage();
    void findUserInLineEdit();
    std::string getCurrentUserForMessage();
    void showUsersInBrowser();
    void createUserInLeftMenu(std::string name); // create a button with nickname
    QPushButton* findButtonByName(const QString& name);

private slots:
    void onButtonClicked();
    void on_searchUserPushButton_clicked();
    void on_sendMessagePushButton_clicked();

private:
    QString CurrentUserForMessage_;
    Ui::MainWindow *ui;
    Connect* connectToServer;

    QVBoxLayout* buttonLayout;

};

#endif // MAINWINDOW_H
