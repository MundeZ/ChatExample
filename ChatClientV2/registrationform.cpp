#include "registrationform.h"
#include "ui_registrationform.h"

RegistrationForm::RegistrationForm(QWidget *parent, Connect* connectToServer)
    : QWidget(parent)
    , ui(new Ui::RegistrationForm)
    , connectToServer(connectToServer)
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

    if (password == confirm) {
        if (!login.empty()) {
            std::map<std::string, std::string> response = connectToServer->requestToServerRegistration(
                connectToServer->getApi(REGISTRATION), login, password);

            auto apiIt = response.find("api");
            auto messageIt = response.find("response_message");

            if (apiIt != response.end() && apiIt->second == "Registration" &&
                messageIt != response.end() && messageIt->second == "OK") {
                emit enterToChatRequested();
            } else {
                std::string errorMessage = (messageIt != response.end()) ? messageIt->second : "Unknown error";
                ui->textBrowser->setText(QString::fromStdString(errorMessage));
            }
        } else {
            ui->textBrowser->setText("Login can't be empty");
        }
    } else {
        ui->textBrowser->setText("Passwords do not match.");
    }
}
