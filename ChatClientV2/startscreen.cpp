#include "startscreen.h"
#include "ui_startscreen.h"

StartScreen::StartScreen(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StartScreen)
{
    ui->setupUi(this);

    loginForm = new LoginForm(this);
    registrationForm = new RegistrationForm(this);
    mainWindow = new MainWindow(this);

    ui->stackedWidget->addWidget(loginForm);
    ui->stackedWidget->addWidget(registrationForm);
    ui->stackedWidget->addWidget(mainWindow);

    connect(loginForm, &LoginForm::registerRequested, this, &StartScreen::setRegistrationForm);
    connect(loginForm, &LoginForm::enterToChatRequested, this, &StartScreen::setMainWindow);
    connect(registrationForm, &RegistrationForm::loginRequested, this, &StartScreen::setLoginForm);
    connect(registrationForm, &RegistrationForm::enterToChatRequested, this, &StartScreen::setMainWindow);

    ui->stackedWidget->setCurrentWidget(loginForm); // Устанавливаем начальную страницу
}

StartScreen::~StartScreen() {
    delete ui;
}

void StartScreen::setLoginForm() {
    ui->stackedWidget->setCurrentWidget(loginForm);
}

void StartScreen::setRegistrationForm() {
    ui->stackedWidget->setCurrentWidget(registrationForm);
}

void StartScreen::setMainWindow() {
    ui->stackedWidget->setCurrentWidget(mainWindow);
}
