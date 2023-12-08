#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <QWidget>
#include <QTcpSocket>
#include "broadcastroom.h"
#include "pack.h"

namespace Ui {
class UserInterface;
}

class UserInterface : public QWidget
{
    Q_OBJECT

public:
    ~UserInterface();
    static UserInterface* create_user_interface(const QString& user_name,QTcpSocket* client,QWidget* parnet = nullptr);

    void onReadyRead(Pack& pack);
    void create_room_back(Pack& pack);
    void has_new_room(Pack& pack);
    void flush_all_room_back(Pack& pack);
    void send_image_back(Pack &pack);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::UserInterface *ui;
    explicit UserInterface(const QString& user_name,QTcpSocket* client,QWidget *parent = nullptr);
    static UserInterface* instance;
    QTcpSocket* client;
    QString user_name;
    BroadCastRoom* room;
    QByteArray image_arr;//缓存图片的属性
};

#endif // USERINTERFACE_H
