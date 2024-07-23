#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

MainWindow::MainWindow(QWidget *parent, Connect* connectToServer)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , connectToServer(connectToServer)
    , messageTimer(new QTimer(this))
{
    ui->setupUi(this);

    // Создаем компоновщик для widget_1
    buttonLayout = new QVBoxLayout(ui->widget_1);
    ui->widget_1->setLayout(buttonLayout);

    // Настройка таймера для проверки сообщений
    connect(messageTimer, &QTimer::timeout, this, &MainWindow::checkMessage);

    // Настройка QFutureWatcher для обработки асинхронных ответов
    connect(&messageWatcher, &QFutureWatcher<QString>::finished, this, [this]() {
        handleMessageResponse(messageWatcher.result());
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setCurrentUserForMessage(QString name) {
    CurrentUserForMessage_ = name;

    // При смене пользователя, показать историю сообщений для него
    if (chatHistory.find(name) != chatHistory.end()) {
        ui->dispayMessageTextBrowser->setText(chatHistory[name]);
    } else {
        ui->dispayMessageTextBrowser->clear();
    }
}

void MainWindow::removeCurrentUserForMessage() {
    CurrentUserForMessage_ = "";
}

void MainWindow::findUserInLineEdit() {
    std::string data = ui->findUserLineEdit->text().toStdString();

    QFuture<void> future = QtConcurrent::run([this, data]() {
        std::string response = connectToServer->requestToServer(connectToServer->getApi(FIND_USER), data, "", "", "");
        QMetaObject::invokeMethod(this, [this, response, data]() {
            if(response == "OK") {
                createUserInLeftMenu(data);
            } else {
                ui->findUserLineEdit->setText(QString::fromStdString(response));
            }
        });
    });
}

void MainWindow::createUserInLeftMenu(std::string name) {
    QString name_1 = QString::fromStdString(name);
    QPushButton* button1 = new QPushButton(name_1, ui->widget_1);
    connect(button1, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    buttonLayout->addWidget(button1);

    // Запуск проверки сообщений, если окно существует
    if (ui->dispayMessageTextBrowser) {
        messageTimer->start(2000);
    }
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
        removeCurrentUserForMessage();
        setCurrentUserForMessage(buttonText);

        // Немедленно проверяем сообщения при выборе пользователя
        QFuture<void> future = QtConcurrent::run([this]() {
            QString response = QString::fromStdString(connectToServer->responseFromServer());
            QMetaObject::invokeMethod(this, [this, response]() {
                handleMessageResponse(response);
            });
        });
    }
}

void MainWindow::on_searchUserPushButton_clicked() {
    findUserInLineEdit();
}

void MainWindow::on_sendMessagePushButton_clicked() {
    std::string recipient = CurrentUserForMessage_.toStdString();
    std::string data = ui->inputMessageTextEdit->toPlainText().toStdString();
    if (!CurrentUserForMessage_.isEmpty()) {
        QFuture<void> future = QtConcurrent::run([this, recipient, data]() {
            std::string response = connectToServer->requestToServer(connectToServer->getApi(MESSAGE), "", "", data, recipient);
            QString msg = QString::fromStdString(response);
            QMetaObject::invokeMethod(this, [this, response]() {
                handleMessageResponse(QString::fromStdString(response));
            });
        });
    } else {
        QString msgError = "No user selected for messaging";
        ui->dispayMessageTextBrowser->append(msgError);
    }
}

void MainWindow::checkMessage() {
    if (ui->dispayMessageTextBrowser) {
        QFuture<void> future = QtConcurrent::run([this]() {
            QString response = QString::fromStdString(connectToServer->responseFromServer());
            QMetaObject::invokeMethod(this, [this, response]() {
                handleMessageResponse(response);
            });
        });
    } else {
        messageTimer->stop();  // Останавливаем таймер, если окно закрыто
    }
}

void MainWindow::handleMessageResponse(QString message) { // сообщение приходит в формате "Sender: Message"
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
