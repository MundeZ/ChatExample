#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

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

    // Создаем компоновщик для widget_1
    buttonLayout = new QVBoxLayout(ui->widget_1);
    ui->widget_1->setLayout(buttonLayout);

    // Настройка таймера для проверки сообщений
    connect(messageTimer, &QTimer::timeout, this, &MainWindow::checkMessage);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setCurrentUserForMessage(const QString& name) {
    CurrentUserForMessage_ = name;

    // Показываем историю сообщений для текущего пользователя
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
        std::string response = connectToServer->requestToServer(connectToServer->getApi(FIND_USER), data.toStdString(), "", "", "");
        QString result = QString::fromStdString(response);
        QMetaObject::invokeMethod(this, [this, result, data]() {
                if (result == "OK") {
                    createUserInLeftMenu(data);
                } else {
                    ui->findUserLineEdit->setText(result);
                }
            }, Qt::QueuedConnection);
    });

    userSearchWatcher->setFuture(future);
}

void MainWindow::createUserInLeftMenu(const QString& name) {
    QMetaObject::invokeMethod(this, [this, name]() {
            QPushButton* existingButton = findButtonByName(name);
            if (!existingButton) {
                QPushButton* button = new QPushButton(name, ui->widget_1);
                connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
                buttonLayout->addWidget(button);
                button->show();
            }
        }, Qt::QueuedConnection);
}

QPushButton* MainWindow::findButtonByName(const QString& name) {
    QList<QPushButton*> buttons = ui->widget_1->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->text() == name) {
            return button;
        }
    }
    return nullptr;
}

void MainWindow::onButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString buttonText = button->text();
        if (CurrentUserForMessage_ != buttonText) {
            stopMessageCheck();
            setCurrentUserForMessage(buttonText);

            QFuture<void> future = QtConcurrent::run([this]() {
                std::string response = connectToServer->responseFromServer();
                QString result = QString::fromStdString(response);
                QMetaObject::invokeMethod(this, [this, result]() {
                        handleMessageResponse(result);
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
            std::string response = connectToServer->requestToServer(connectToServer->getApi(MESSAGE), "", "", data.toStdString(), recipient.toStdString());
            QString result = QString::fromStdString(response);
            QMetaObject::invokeMethod(this, [this, result]() {
                    handleMessageResponse(result);
                }, Qt::QueuedConnection);
        });

        messageSendWatcher->setFuture(future);
    } else {
        QString msgError = "No user selected for messaging";
        ui->dispayMessageTextBrowser->append(msgError);
    }
}

void MainWindow::startMessageCheck() {
    if (!isCheckingMessages) {
        isCheckingMessages = true;
        messageTimer->start(2000); // Проверяем сообщения каждые 2 секунды
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
            std::string response = connectToServer->responseFromServer();
            QString result = QString::fromStdString(response);
            QMetaObject::invokeMethod(this, [this, result]() {
                    handleMessageResponse(result);
                }, Qt::QueuedConnection);
        });

        messageSendWatcher->setFuture(future);
    } else {
        stopMessageCheck();
    }
}

void MainWindow::handleMessageResponse(const QString& message) {
    if (!message.isEmpty()) {
        QStringList parts = message.split(": ", Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            QString sender = parts[0];
            QString msg = parts[1];
            processIncomingMessage(sender, msg);
        } else {
            ui->dispayMessageTextBrowser->append(message);
        }
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
