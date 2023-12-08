#include "userinterface.h"
#include "ui_userinterface.h"

UserInterface::UserInterface(const QString &user_name, QTcpSocket *client,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserInterface)
    , user_name(user_name)
    , client(client)
{
    ui->setupUi(this);
    Pack pack;
    pack.setType(TYPE_FLUSH_ALL_ROOM);
    client->write(pack.data(),pack.size());
}

UserInterface::~UserInterface()
{
    delete ui;
}

UserInterface *UserInterface::create_user_interface(const QString &user_name, QTcpSocket *client, QWidget *parent)
{
    if(instance==NULL){
        instance = new UserInterface(user_name,client,parent);
    }
    return instance;
}

void UserInterface::onReadyRead(Pack &pack)
{
    qDebug() << __func__ << pack.getType();
    switch(pack.getType()){
        case TYPE_SEND_IMAGE:
            qDebug() << "TYPE_SEND_IMAGE";
            send_image_back(pack);
            break;
        case TYPE_CREAET_ROOM:
            qDebug() << "创建直播间回包";
            create_room_back(pack);
            break;
        case TYPE_HAS_NEW_ROOM:
            has_new_room(pack);
            break;
        case TYPE_FLUSH_ALL_ROOM:
            flush_all_room_back(pack);
            break;
    }
}

void UserInterface::create_room_back(Pack &pack)
{
    QStringList list = pack.readAll();
    QString create_room_id = list[0];
    ui->listWidget->addItem(create_room_id);
    room = BroadCastRoom::create_broadcast_room(user_name,client,true);
    room->show();
    this->hide();
}

//?
void UserInterface::send_image_back(Pack &pack)
{
    qDebug() << __func__;

    QByteArray arr = pack.readImage();//读取一帧图片中的一部分

    qDebug() << "arr.size()" << arr.size();

    //将一帧图片中不完整的一部分，进行缓存，直到接收到一帧图片的最后一包的时候，才将缓存区的图片显示到窗口里面
    image_arr.append(arr);//将读取到的一部分图片缓存到 image_arr 里面去
    if(pack.getErr() == ERR_LAST_PIX)
    {
        // 这是一帧图片的最后一包了，image_arr 已经构成了一帧完整的图片，所以可以直接进行显示了
        QBuffer buf(&image_arr);//以image_arr作为缓存区，构建QBuffer对象buf，实际上image_arr里面的数据，就是buf里面的数据
        QImageReader reader(&buf,"JPEG");//使用存有一帧图片信息的buf，去构建QImageReader 对象reader，并且图片示意 jpeg 的格式，存入reader里面
        QImage image = reader.read();//以QImage形式读取一帧图片
        QPixmap pic = QPixmap::fromImage(image);
        room->setPixmap(pic);

        // 此时一帧图片已经设置到窗口里面了，所以缓存图片的image_arr可以清理一下了
        image_arr.clear();
    }
}
//?

void UserInterface::has_new_room(Pack &pack)
{
    QStringList list = pack.readAll();
    QString new_room_id = list[0];
    ui->listWidget->addItem(new_room_id);
}

void UserInterface::flush_all_room_back(Pack &pack)
{
    QStringList room_id_list = pack.readAll();
    int len = room_id_list.length();
    for(int i=0;i<len;i++){
        ui->listWidget->addItem(room_id_list[i]);
    }
}



/*
    创建直播间：点击之后不能立刻创建，整个流程应该如下
    1：发送给服务器创建直播间的请求
    2：服务器接收到请求之后，产生一个直播间号码，同步给所有在线用户的直播间列表
    3：主播接受到服务器发送过来的房间号之后，就可以切换到直播间界面了
*/
void UserInterface::on_pushButton_clicked()
{
    Pack pack;
    pack.setType(TYPE_CREAET_ROOM);
    pack << user_name;
    client->write(pack.data(),pack.size());
}

void UserInterface::on_pushButton_2_clicked()
{
    room = BroadCastRoom::create_broadcast_room(user_name,client,false);
    room->show();
    this->hide();
}
