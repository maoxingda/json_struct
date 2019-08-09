#include "jstructhelpergui.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    jstructhelpergui w;
    w.show();
    return a.exec();
}
