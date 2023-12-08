#include "widget.h"

#include <QApplication>

UserInterface* UserInterface::instance = NULL;
BroadCastRoom* BroadCastRoom::instance = NULL;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
