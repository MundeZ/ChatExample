#include "loginform.h"
#include "ui_loginform.h"

LoginForm::LoginForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginForm)
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
    emit enterToChatRequested();
}
