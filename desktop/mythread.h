#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QApplication>
#include <QScreen>
#include <QPixmap>

class myThread:public QThread
{
    Q_OBJECT
private:
    QScreen* screen;

public:
    myThread(QObject* parent = nullptr);

protected:
    virtual void run() override;

signals:
    void sndPic(QPixmap pic);
};

#endif // MYTHREAD_H
