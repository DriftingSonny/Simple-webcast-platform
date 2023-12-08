#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "C:\\Users\\h\\Documents\\project\\desktop\\pack.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QBuffer>
#include <QImageReader>
#include "dbhelper.h"
#include "clientinof.h"
#include <QMap>



QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void rcv_image(Pack& pack, QTcpSocket* client);
    void regist(Pack& pack,QTcpSocket* client);
    void login(Pack& pack,QTcpSocket* client);
    void create_room(Pack& pack,QTcpSocket* client);
    void flush_room(Pack& pack,QTcpSocket* client);


    void rcvPic(QPixmap pic, QTcpSocket *client);
private:
    Ui::Widget *ui;
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

    DbHelper* db;
    int room_id;// 用来记录创建直播间的房间号，并自增
    QMap<QTcpSocket*,ClientInof> map;//将在线用户的套接字，用户名，创建的房间号，加入的房间号，统一管理
    QList<QTcpSocket*> clientList;

public slots:
    void onReadyRead();
    void onNewConnection();
};
#endif // WIDGET_H
