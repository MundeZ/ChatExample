#include <iostream>

//#include "mainwindow.h"
//#include "loginform.h"
#include "startscreen.h"
#include <QApplication>
#include <QTranslator>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StartScreen startScreen;
    startScreen.show();

    return a.exec();
}


