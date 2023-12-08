#include "mythread.h"

myThread::myThread(QObject *parent)
    :QThread(parent)
{
    screen = QApplication::primaryScreen();
}

void myThread::run()
{
    while(1){
        msleep(200);
        QPixmap pic = screen->grabWindow(0);
        sndPic(pic);
    }
}
