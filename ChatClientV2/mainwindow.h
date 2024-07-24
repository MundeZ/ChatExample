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

    void setCurrentUserForMessage(const QString& name);
    void removeCurrentUserForMessage();
    void findUserInLineEdit();
    std::string getCurrentUserForMessage() const;
    void showUsersInBrowser();
    void createUserInLeftMenu(const QString& name);
    QPushButton* findButtonByName(const QString& name);

    void startMessageCheck();
    void stopMessageCheck();
    void handleMessageResponse(const QString& message);
    void processIncomingMessage(const QString& sender, const QString& message);

    void checkMessage();

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
    std::map<QString, QString> chatHistory;

    QFutureWatcher<void>* userSearchWatcher;
    QFutureWatcher<void>* messageSendWatcher;
    QFuture<void> messageCheckFuture;
    bool isCheckingMessages;
};

#endif // MAINWINDOW_H
