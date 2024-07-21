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
}

void MainWindow::removeCurrentUserForMessage() {
    CurrentUserForMessage_ = "";
}

void MainWindow::findUserInLineEdit() {
    std::string data = ui->findUserLineEdit->text().toStdString();
    std::string response = connectToServer->requestToServer(connectToServer->getApi(FIND_USER), data, "", "", "");

    if(response == "OK") {
        createUserInLeftMenu(data);
    } else {
        ui->findUserLineEdit->setText(QString::fromStdString(response));
    }
}

void MainWindow::createUserInLeftMenu(std::string name) {
    QString name_1 = QString::fromStdString(name);
    QPushButton* button1 = new QPushButton(name_1, ui->widget_1);
    connect(button1, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    buttonLayout->addWidget(button1);

    // Запуск проверки сообщений, если окно существует
    if (ui->dispayMessageTextBrowser) {
        messageTimer->start(1000);
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
        QFuture<QString> future = QtConcurrent::run([this]() {
            return QString::fromStdString(connectToServer->responseFromServer());
        });
        messageWatcher.setFuture(future);
    }
}

void MainWindow::on_searchUserPushButton_clicked() {
    findUserInLineEdit();
}

void MainWindow::on_sendMessagePushButton_clicked() {
    if (!CurrentUserForMessage_.isEmpty()) {
        std::string recipient = CurrentUserForMessage_.toStdString();
        std::string data = ui->inputMessageTextEdit->toPlainText().toStdString();
        std::string response = connectToServer->requestToServer(connectToServer->getApi(MESSAGE), "", "", data, recipient);
        QString x = QString::fromStdString(response);
        ui->dispayMessageTextBrowser->append(x);
    } else {
        QMessageBox::warning(this, "Error", "No user selected for messaging");
    }
}

void MainWindow::checkMessage() {
    if (ui->dispayMessageTextBrowser) {
        QFuture<QString> future = QtConcurrent::run([this]() {
            return QString::fromStdString(connectToServer->responseFromServer());
        });
        messageWatcher.setFuture(future);
    } else {
        messageTimer->stop();  // Останавливаем таймер, если окно закрыто
    }
}

void MainWindow::handleMessageResponse(QString message) {
    if (!message.isEmpty()) {
        ui->dispayMessageTextBrowser->append(message);
    }
}
