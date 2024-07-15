#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include "connect.h"

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr, Connect* connectToServer = nullptr);
    ~LoginForm();

signals:
    void registerRequested();
    void enterToChatRequested();

private slots:
    void on_registrationPushButton_clicked();
    void on_enterToChatPushButton_clicked();

private:
    Ui::LoginForm *ui;
    Connect* connectToServer;
};

#endif // LOGINFORM_H
