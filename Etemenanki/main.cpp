#include "Etemenanki.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Etemenanki w;
    w.show();
    return a.exec();
}
