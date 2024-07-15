#include "registrationform.h"
#include "ui_registrationform.h"

RegistrationForm::RegistrationForm(QWidget *parent, Connect* connectToServer)
    : QWidget(parent)
    , ui(new Ui::RegistrationForm)
    , connectToServer(connectToServer) // Инициализация указателя
{
    ui->setupUi(this);
}

RegistrationForm::~RegistrationForm() {
    delete ui;
}

void RegistrationForm::on_loginButton_clicked() {
    emit loginRequested();
}

void RegistrationForm::on_enterToChatPushButton_clicked() {
    std::string login = ui->loginEdit->text().toStdString();
    std::string password = ui->passwordEdit->text().toStdString();
    std::string confirm = ui->confirmEdit->text().toStdString();

    if (password == confirm ) {
        if(!login.empty()) {
            std::string response = connectToServer->requestToServer(connectToServer->getApi(REGISTRATION), login, password, "", "");
            if (response == "OK") {
                emit enterToChatRequested();
            } else {
                ui->textBrowser->setText(QString::fromStdString(response));
            }
        } else {
            ui->textBrowser->setText("login can't be empty");
        }

    } else {
        ui->textBrowser->setText("Passwords do not match.");
    }
}
