#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    try {
        MainWindow w;
        //w.show(); ... zajistuje dialog pro vyber predmetu, viz konstruktor w
        return a.exec();
    }
    catch(...) {
        return 1;
    }
}
