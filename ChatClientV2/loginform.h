#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr);
    ~LoginForm();

signals:
    void registerRequested();

private slots:
    void on_registrationPushButton_clicked();
    void on_enterToChatPushButton_clicked();

private:
    Ui::LoginForm *ui;
};

#endif // LOGINFORM_H
