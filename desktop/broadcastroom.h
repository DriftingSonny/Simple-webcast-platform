#ifndef BROADCASTROOM_H
#define BROADCASTROOM_H

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
#include <QImageReader>


namespace Ui {
class BroadCastRoom;
}

class BroadCastRoom : public QWidget
{
    Q_OBJECT

public:
    ~BroadCastRoom();
    static BroadCastRoom* create_broadcast_room(const QString& user_name,QTcpSocket* client,bool id,QWidget *parent = nullptr);

    static void send_image_back(QTcpSocket *client, Pack &pack);
    void setPixmap(QPixmap pix);


private slots:
    void on_pushButton_2_clicked();
    void rcvPic(QPixmap pic);

private:
    Ui::BroadCastRoom *ui;
    myThread* t;
    explicit BroadCastRoom(const QString& user_name,QTcpSocket* client,bool id,QWidget *parent = nullptr);
    static BroadCastRoom* instance;
    QString user_name;
    QTcpSocket* client;
    bool id;// id==true 表示是主播，id==false 表示是观众
    QLabel* label;

    QTcpServer* server;
    QHostAddress addr;
    quint16 port;

    QByteArray image_arr;//缓存图片的属性

    int readed_size_size;//已读取的协议包尺寸的大小
    int unreaded_size_size;//未读取的协议包尺寸的大小
    int size_buf;//缓存已经读取的协议包尺寸的数据

    int readed_pack_size;//已读取的协议包本身的大小
    int unreaded_pack_size;//未读取的协议包本身的大小
    Pack pack_buf;//用来缓存已经读取的协议包的数据

public slots:
    void onReadyRead();
    void onNewConnection();

};

#endif // BROADCASTROOM_H
