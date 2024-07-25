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
        std::map<std::string, std::string> response = connectToServer->requestToServerLogin(connectToServer->getApi(LOGIN), login, password);

        auto apiIt = response.find("api");
        auto messageIt = response.find("response_message");

        if (apiIt != response.end() && apiIt->second == "Login" &&
            messageIt != response.end() && messageIt->second == "OK") {
            emit enterToChatRequested();
        } else {
            std::string errorMessage = (messageIt != response.end()) ? messageIt->second : "Unknown error";
            ui->textBrowser->setText(QString::fromStdString(errorMessage));
        }
    } else {
        ui->textBrowser->setText("Login can't be empty");
    }
}
