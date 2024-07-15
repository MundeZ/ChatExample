#include "loginform.h"
#include "ui_loginform.h"

LoginForm::LoginForm(QWidget *parent, Connect* connectToServer)
    : QWidget(parent)
    , ui(new Ui::LoginForm)
    , connectToServer(connectToServer)
{
    ui->setupUi(this);
}

LoginForm::~LoginForm() {
    delete ui;
}

void LoginForm::on_registrationPushButton_clicked() {
    emit registerRequested();
}

void LoginForm::on_enterToChatPushButton_clicked() {
    std::string login = ui->loginEditInLoginWindow->text().toStdString();
    std::string password = ui->passwordEditInLoginWindow->text().toStdString();

    if(!login.empty()) {
        std::string response = connectToServer->requestToServer(connectToServer->getApi(LOGIN), login, password, "", "");
        if (response == "OK") {
            emit enterToChatRequested();
        } else {
            ui->textBrowser->setText(QString::fromStdString(response));
        }
    } else {
        ui->textBrowser->setText("login can't be empty");
    }
}
