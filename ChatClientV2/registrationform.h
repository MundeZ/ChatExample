#ifndef REGISTRATIONFORM_H
#define REGISTRATIONFORM_H

#include <QWidget>
#include "connect.h"

namespace Ui {
class RegistrationForm;
}

class RegistrationForm : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrationForm(QWidget *parent = nullptr, Connect* connectToServer = nullptr);
    ~RegistrationForm();

signals:
    void loginRequested();
    void enterToChatRequested();

private slots:
    void on_loginButton_clicked();
    void on_enterToChatPushButton_clicked();

private:
    Ui::RegistrationForm *ui;
    Connect* connectToServer; // НЕ ЗАБЫВАТЬ ИСПОЛЬЗОВАТЬ УКАЗАТАЛЬ ИНАЧЕ БУДЕТ СОЗДАННО НОВОЕ ПОДКЛЮЧЕНИЕ

};

#endif // REGISTRATIONFORM_H
