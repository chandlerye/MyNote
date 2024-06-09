#include "mainwindow.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Form_test b;
    // b.show();

    MainWindow w;
    w.show();
    return a.exec();
}
