#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QScreen>
#include "mythread.h"
#include "pack.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QBuffer>
#include <QByteArray>
#include <QMessageBox>
#include "userinterface.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void regist_back(Pack& pack);
    void login_back(Pack& pack);


private:
    Ui::Widget *ui;
    QScreen* screen;//这个类的对象不能自主构建，必须通过其他方法返回
    myThread* t;

    QTcpSocket* client;
    QHostAddress addr;
    quint16 port;

    UserInterface* user_interface;
    QString user_name;

public slots:
    void onReadyRead();
//    void onNewConnection();
//    void rcvPic(QPixmap pic);

private slots:
    //void on_pushButton_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
};
#endif // WIDGET_H
