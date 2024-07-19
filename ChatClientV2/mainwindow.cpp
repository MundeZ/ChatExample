#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent, Connect* connectToServer)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , connectToServer(connectToServer)
{
    ui->setupUi(this);

    // Создаем компоновщик для widget_1
    buttonLayout = new QVBoxLayout(ui->widget_1);



    ui->widget_1->setLayout(buttonLayout);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setCurrentUserForMessage(QString name) {
    CurrentUserForMessage_ = name;
}

void MainWindow::removeCurrentUserForMessage() {
    CurrentUserForMessage_ = nullptr;
}

void MainWindow::findUserInLineEdit() { // ищем юзера
    std::string data = ui->findUserLineEdit->text().toStdString();
    std::string response = connectToServer->requestToServer(connectToServer->getApi(LOGIN), data, "", "", "");

    if(response == "OK") {
        createUserInLeftMenu(data);
    } else {
        ui->findUserLineEdit->setText(QString::fromStdString(response));
    }
}

void MainWindow::createUserInLeftMenu(std::string name) { // создаем кнопку с ником
    QString name_1 = QString::fromStdString(name);
    QPushButton* button1 = new QPushButton(name_1, ui->widget_1);
    connect(button1, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    buttonLayout->addWidget(button1);
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
    }
}


void MainWindow::on_searchUserPushButton_clicked() {
    findUserInLineEdit();
}


void MainWindow::on_sendMessagePushButton_clicked() { // тут реализовать отправку сообщения
    std::string recipient =  CurrentUserForMessage_.toStdString();
    std::string data = ui->inputMessageTextEdit->toPlainText().toStdString();
    connectToServer->requestToServer(connectToServer->getApi(MESSAGE),"","",data, recipient);
}

