#include <QApplication>
#include <QDesktopServices>
#include "globalaccel.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    GlobalAccel ga;
	ga.bindDefault ();

    return a.exec();
}
