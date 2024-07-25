#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent, Connect* connectToServer)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , connectToServer(connectToServer)
    , messageTimer(new QTimer(this))
    , userSearchWatcher(new QFutureWatcher<void>(this))
    , messageSendWatcher(new QFutureWatcher<void>(this))
    , isCheckingMessages(false)
{
    ui->setupUi(this);
    buttonLayout = new QVBoxLayout(ui->widget_1);
    ui->widget_1->setLayout(buttonLayout);
    connect(messageTimer, &QTimer::timeout, this, &MainWindow::checkMessage);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setCurrentUserForMessage(const QString& name) {
    CurrentUserForMessage_ = name;
    ui->dispayMessageTextBrowser->clear();
    if (chatHistory.find(name) != chatHistory.end()) {
        ui->dispayMessageTextBrowser->setText(chatHistory[name]);
    }
}

void MainWindow::removeCurrentUserForMessage() {
    CurrentUserForMessage_.clear();
}

void MainWindow::findUserInLineEdit() {
    QString data = ui->findUserLineEdit->text();

    QFuture<void> future = QtConcurrent::run([this, data]() {
        auto response = connectToServer->requestToServerFindUser(connectToServer->getApi(FIND_USER), data.toStdString());
        QMetaObject::invokeMethod(this, [this, response, data]() {
                auto apiIt = response.find("api");
                auto messageIt = response.find("response_message");
                if (apiIt != response.end() && apiIt->second == "FindUser" &&
                    messageIt != response.end() && messageIt->second == "OK") {
                    createUserInLeftMenu(data);
                } else {
                    ui->findUserLineEdit->setText(QString::fromStdString(messageIt->second));
                }
            }, Qt::QueuedConnection);
    });

    userSearchWatcher->setFuture(future);
}

void MainWindow::createUserInLeftMenu(const QString& name) {
    QMetaObject::invokeMethod(this, [this, name]() {
            if (!findButtonByName(name)) {
                auto button = new QPushButton(name, ui->widget_1);
                connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
                buttonLayout->addWidget(button);
                button->show();
            }
        }, Qt::QueuedConnection);
}

QPushButton* MainWindow::findButtonByName(const QString& name) {
    for (auto* button : ui->widget_1->findChildren<QPushButton*>()) {
        if (button->text() == name) {
            return button;
        }
    }
    return nullptr;
}

void MainWindow::onButtonClicked() {
    auto* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString buttonText = button->text();
        if (CurrentUserForMessage_ != buttonText) {
            stopMessageCheck();
            setCurrentUserForMessage(buttonText);

            QFuture<void> future = QtConcurrent::run([this]() {
                auto response = connectToServer->responseFromServer();
                QMetaObject::invokeMethod(this, [this, response]() {
                        handleMessageResponse(response);
                    }, Qt::QueuedConnection);
            });

            messageSendWatcher->setFuture(future);
            startMessageCheck();
        }
    }
}

void MainWindow::on_searchUserPushButton_clicked() {
    stopMessageCheck();
    findUserInLineEdit();
}

void MainWindow::on_sendMessagePushButton_clicked() {
    QString recipient = CurrentUserForMessage_;
    QString data = ui->inputMessageTextEdit->toPlainText();
    if (!recipient.isEmpty()) {
        QFuture<void> future = QtConcurrent::run([this, recipient, data]() {
            auto response = connectToServer->requestToServerMessage(connectToServer->getApi(MESSAGE), data.toStdString(), recipient.toStdString());
            QMetaObject::invokeMethod(this, [this, response]() {
                    handleMessageResponse(response);
                }, Qt::QueuedConnection);
        });

        messageSendWatcher->setFuture(future);
    } else {
        ui->dispayMessageTextBrowser->append("No user selected for messaging");
    }
}

void MainWindow::startMessageCheck() {
    if (!isCheckingMessages) {
        isCheckingMessages = true;
        messageTimer->start(2000); // Check messages every 2 seconds
    }
}

void MainWindow::stopMessageCheck() {
    if (isCheckingMessages) {
        isCheckingMessages = false;
        messageTimer->stop();
    }
}

void MainWindow::checkMessage() {
    if (!CurrentUserForMessage_.isEmpty()) {
        QFuture<void> future = QtConcurrent::run([this]() {
            auto response = connectToServer->responseFromServer();
            QMetaObject::invokeMethod(this, [this, response]() {
                    handleMessageResponse(response);
                }, Qt::QueuedConnection);
        });

        messageSendWatcher->setFuture(future);
    } else {
        stopMessageCheck();
    }
}

void MainWindow::handleMessageResponse(const std::map<std::string, std::string>& response) {
    auto apiIt = response.find("api");
    auto messageIt = response.find("response_message");
    if (apiIt != response.end() && messageIt != response.end()) {
        QString api = QString::fromStdString(apiIt->second);
        QString message = QString::fromStdString(messageIt->second);
        if (api == "Message") {
            QStringList parts = message.split(": ", Qt::SkipEmptyParts);
            if (parts.size() == 2) {
                QString sender = parts[0];
                QString msg = parts[1];
                processIncomingMessage(sender, msg);
            } else {
                ui->dispayMessageTextBrowser->append(message);
            }
        } else {
            ui->dispayMessageTextBrowser->append(message);
        }
    } else {
        ui->dispayMessageTextBrowser->append("Unexpected response format");
    }
}

void MainWindow::processIncomingMessage(const QString& sender, const QString& message) {
    if (chatHistory.find(sender) != chatHistory.end()) {
        chatHistory[sender] += "\n" + sender + ": " + message;
    } else {
        chatHistory[sender] = sender + ": " + message;
    }

    if (CurrentUserForMessage_ == sender) {
        ui->dispayMessageTextBrowser->append(sender + ": " + message);
    }
}
