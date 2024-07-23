#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "connect.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <map>

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
    void createUserInLeftMenu(std::string name);
    QPushButton* findButtonByName(const QString& name);

    void checkMessage();
    void handleMessageResponse(QString message);
    void processIncomingMessage(const QString& sender, const QString& message);

private slots:
    void onButtonClicked();
    void on_searchUserPushButton_clicked();
    void on_sendMessagePushButton_clicked();

private:
    QString CurrentUserForMessage_;
    Ui::MainWindow *ui;
    Connect* connectToServer;

    QVBoxLayout* buttonLayout;
    QTimer* messageTimer;
    QFutureWatcher<QString> messageWatcher;
    std::map<QString, QString> chatHistory; // история чата для каждого пользователя
};

#endif // MAINWINDOW_H
