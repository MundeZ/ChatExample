#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "startscreen.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_privateSendMessageButton_clicked() {

}


void MainWindow::on_sendMessageButton_clicked() {

}

