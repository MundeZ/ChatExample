#ifndef REGISTRATIONFORM_H
#define REGISTRATIONFORM_H

#include <QWidget>

namespace Ui {
class RegistrationForm;
}

class RegistrationForm : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrationForm(QWidget *parent = nullptr);
    ~RegistrationForm();

private slots:
    void on_loginButton_clicked();
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
signals:
    void loginRequested();

private:
    Ui::RegistrationForm *ui;
};

#endif // REGISTRATIONFORM_H
