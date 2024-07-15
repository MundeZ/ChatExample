#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QDialog>
#include "loginform.h"
#include "registrationform.h"
#include "mainwindow.h"
#include "connect.h"

namespace Ui {
class StartScreen;
}

class StartScreen : public QDialog
{
    Q_OBJECT

public:
    explicit StartScreen(QWidget *parent = nullptr);
    ~StartScreen();

private slots:
    void setLoginForm();
    void setRegistrationForm();
    void setMainWindow();

private:
    Ui::StartScreen *ui;
    LoginForm *loginForm;
    RegistrationForm *registrationForm;
    MainWindow *mainWindow;
    Connect connectToServer;
};

#endif // STARTSCREEN_H
