#include <QCoreApplication>

#include "wesley.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Wesley w;
    w.start();

    return a.exec();
}
