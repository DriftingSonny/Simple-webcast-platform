#include "widget.h"
#include "ui_widget.h"
#include <QTimer>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , addr("127.0.0.1")
    , port(12345)
    , room_id(2305000)
{
    ui->setupUi(this);
    server = new QTcpServer(this);
    server->listen(addr,port);
    QObject::connect(server,&QTcpServer::newConnection,this,&Widget::onNewConnection);
    readed_pack_size = 0;
    readed_size_size = 0;//一旦发生分包，这2个数据有一个肯定不为0

    db = new DbHelper("QMYSQL","alot2305","huang1201");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::rcvPic(QPixmap pic, QTcpSocket *client)
{
    qDebug() << __func__ << client;

    for(QTcpSocket *currentClient : clientList)
    {
        if(currentClient == client)
        {
            continue;
        }

        pic = pic.scaled(ui->label->size());
        QImage image = pic.toImage();// 准备即将发送的一帧图片
    //    ui->label->setPixmap(pic);
        /*
            将QImage写入到一个char字符里面，这个转换路径是这样子的
            先将QImage 写入 QByteArray
            再将 QByteArray 里面的数据首地址开始，拷贝到 pack.buf里面去
        */
        /*
            QImage怎么转换成QByteArray？
            QImage里面有一个叫做 save(QIODevice* device),可以将这一帧图片的所有信息保存到device里面
            QIODevice有一个派生类叫做QBuffer，QBuffer有一个构造函数，要求传入一个 QByteArray地址，作为则个QBuffer的缓存区，也就是说，写入QBuffer的数据，其实就是写入 QByteArray 里面了
        */
        QByteArray arr;//阶段性目的是将 image 写入到 arr里面去
        QBuffer buf(&arr);//以arr为缓存区构建QBuffer对象buf，写入buf里面的数据，实际上就是写入了arr里面
        image.save(&buf,"JPEG");//将image保存到buf里面，实际上最终保存到arr里面了

        Pack pack;//准备协议包，最终目的是将image写到pack的buf里面去
        pack.setType(TYPE_SEND_IMAGE);
        arr = arr.toBase64();//图片想要在网络中发送，必须转换成Base64的格式，再接收到记得接受完成之后，转回普通格式后，再去拼接图片
        int image_size = arr.size();//记录一下一帧图片的总大小，由于我们的pack包其实只能存放4096字节，而一帧jpeg格式的图片，大概在2w到5w字节左右，所以一个pack包是绝对绝对放不下一帧图片的。一般都是通过多个pack包才能将一帧图片发送完成，什么时候一帧图片发送完毕，就需要依赖图片总大小，以及已经发送的图片大小，自己去算
        int writed_size = 0;//一帧图片已经发送的大小，每次发出去一个包，这个数据都要自增4096

        // 死循环发送图片，注意每次发完一个包，write_size 自增4096,
        // 注意，每次即将发送新的一个包的时候，判断一下，新包发出去之后，writed_size 是否会大于 image_size,如果大于了，说明这是一帧图片的最后一包
        while(1){
            if(writed_size + 4096 <= image_size){
                // 说明即将发送的包，不是一帧图片的最后一包
                pack.setErr(NO_ERR);//使用err去区分一帧图片的前面几个包和最后一个包，因为在服务器，如果判定接受到包是一帧图片的最后一个包，则需要将前面所有图片拼接后显示出来
                pack.append(arr.data()+writed_size,4096);//由于不是一帧图片的最后一包，所以将图片中最靠前的未发送的4096个字节写入pack中后，进行发送
                currentClient->write(pack.data(),pack.size());
                qDebug() << __func__ << currentClient;
            }else{
                // 说明是一帧图片的最后一包了,最后一包只需要发送 "图片总大小 - 已发送的图片大小" 就好
                pack.setErr(ERR_LAST_PIX);
                pack.append(arr.data()+writed_size,image_size - writed_size);
                currentClient->write(pack.data(),pack.size());
                break;
            }
            writed_size += 4096;
        }
    }

}

void Widget::rcv_image(Pack &pack, QTcpSocket* client)
{
    QByteArray arr = pack.readImage();//读取一帧图片中的一部分
    //将一帧图片中不完整的一部分，进行缓存，直到接收到一帧图片的最后一包的时候，才将缓存区的图片显示到窗口里面
    image_arr.append(arr);//将读取到的一部分图片缓存到 image_arr 里面去
    if(pack.getErr() == ERR_LAST_PIX){
        // 这是一帧图片的最后一包了，image_arr 已经构成了一帧完整的图片，所以可以直接进行显示了
        QBuffer buf(&image_arr);//以image_arr作为缓存区，构建QBuffer对象buf，实际上image_arr里面的数据，就是buf里面的数据
        QImageReader reader(&buf,"JPEG");//使用存有一帧图片信息的buf，去构建QImageReader 对象reader，并且图片示意 jpeg 的格式，存入reader里面
        QImage image = reader.read();//以QImage形式读取一帧图片
        QPixmap pic = QPixmap::fromImage(image);
        pic = pic.scaled(ui->label->size());

        rcvPic(pic, client);

        ui->label->setPixmap(pic);
        // 此时一帧图片已经设置到窗口里面了，所以缓存图片的image_arr可以清理一下了
        image_arr.clear();
    }
}

void Widget::regist(Pack &pack, QTcpSocket *client)
{
    QStringList list = pack.readAll();
    QString name = list[0];
    QString pswd = list[1];
    bool res = db->regist(name,pswd);
    if(res==false){
        pack.setErr(ERR_NAME);
    }else{
        pack.setErr(NO_ERR);
    }
    client->write(pack.data(),pack.size());
}

void Widget::login(Pack &pack, QTcpSocket *client)
{
    QStringList list = pack.readAll();
    QString name = list[0];
    QString pswd = list[1];
    bool res = db->login(name,pswd);
    if(res==false){
        pack.setErr(ERR_NAME);
    }else{
        pack.setErr(NO_ERR);
        ClientInof info;
        info.user_name = name;
        map.insert(client,info);//登录成功了，说明在线了，所以要将套接字和用户名做一下管理
    }
    client->write(pack.data(),pack.size());

    QTimer::singleShot(3000, [=](){
        clientList << client;
    });
}

void Widget::create_room(Pack &pack, QTcpSocket *client)
{
    this->room_id ++;
    QString create_room_id = QString::number(room_id);
    QStringList list = pack.readAll();
    QString user_name = list[0];
    map[client].create_room_id = create_room_id;

    pack << create_room_id;
    client->write(pack.data(),pack.size());

    // 服务器要将新创建出来的房间号，同步给其他所有的在线用户，让他们在他们的listWidget里面做显示
    pack.setType(TYPE_HAS_NEW_ROOM);
    for(auto it=map.begin();it!=map.end();it++){
        it.key()->write(pack.data(),pack.size());
    }
}

void Widget::flush_room(Pack &pack, QTcpSocket *client)
{
    // 查看map里面有哪些用户是创建了直播间的
    // 判断map里面，每一个ClientInfo 里面的 create_room_id 是否为空
    for(auto it = map.begin();it!=map.end();it++){
        if(it.value().create_room_id.isEmpty() == false){
            pack << it.value().create_room_id;
        }
    }
    client->write(pack.data(),pack.size());
}


/*
    如果不处理TCP的分包问题，在传输大文件的时候，就容易产生段错误
    原因就是每一次传输的时候，由于网络延迟的原因，服务器同一段时间内，只能接收到固定字节数的数据，这些数据不一定是n个整包，非常有可能最后一个包被拆分了，这种现象，我们称为分包
    要解决分包，整个思路很简单：每一次读数据的时候，都需要判断一下，应该读取的大小，和我真实读取到的大小是否一致
    注意，变长包使用了2次read，这2次read都需要判断是否发生了分包
*/

void Widget::onReadyRead()
{
    QTcpSocket* client = dynamic_cast<QTcpSocket*>(sender());
    Pack pack;
    while(1){
        int size = 0;
        int res = 0;


        if(readed_size_size != 0){
            res = client->read((char*)&size_buf+readed_size_size,unreaded_size_size);
            if(res==0){break;}
            // 现在size_buf里面，已经拥有确证的协议包尺寸了
            size = size_buf;//因为之后的所有逻辑，都是围绕着size和pack来写的，所以当size_buf完整之后，就应该直接赋值给size，然后等待下一次循环的之后，执行else分支，使用完整的size去处理
            if(size==0){break;}
            // 经过分包处理，现在的size已经是一个完整数据了，那么就可以通过这个size去读取剩余的协议包了，并且读取这个剩余协议包的时候，不用担心再次发生分包
            res = client->read(pack.data()+4,size-4);
            readed_size_size = 0;

        }else if(readed_pack_size != 0){
            res = client->read(pack_buf.data()+readed_pack_size,unreaded_pack_size);
            if(res==0){break;}
            // 将完整的pack_buf 拷贝给 pack，以便之后使用pack去执行switch_case 语句
            memcpy(&pack,&pack_buf,sizeof(Pack));
            readed_pack_size = 0;
        }else{
            res = client->read((char*)&size,4);//读取协议包尺寸
            if(res==0){break;}
            if(res != 4){
                // 分包,记录一下分包的各项数据，缓存已读取的数据
                size_buf = 0;//上一次分包时候的缓存数据清空，以免影响本次数据
                readed_size_size = res;
                unreaded_size_size = 4 - res;
                // 将已经读取的数据，先进行缓存，缓存到 size_buf 里面去
                memcpy(&size_buf,&size,readed_size_size);
                break;// 因为分包绝对是发生在可读取的数据的最后一次读取里面，而且此处的break保证在协议包不完整的情况下，不会去进行处理
            }

            res = client->read(pack.data()+4,size-4);//读取协议包本身
            if(res != size-4){
                // 分包，记录一下分包的各项数据，缓存已经读取的协议包
                memset(&pack_buf,0,sizeof(Pack));//清空上一次分包发生时候的缓存数据
                readed_pack_size = res + 4;
                unreaded_pack_size = size - readed_pack_size;
                memcpy(&pack_buf,&pack,readed_pack_size);
                break;
            }
        }
//        qDebug() << __func__ << client << pack.getType();
        switch(pack.getType()){
            case TYPE_SEND_IMAGE:
                rcv_image(pack, client);
                break;
            case TYPE_REGIST:
                regist(pack,client);
                break;
            case TYPE_LOGIN:
                login(pack,client);
                break;
            case TYPE_CREAET_ROOM:
                create_room(pack,client);
                break;
            case TYPE_FLUSH_ALL_ROOM:
                flush_room(pack,client);
                break;
        }

    }
}

void Widget::onNewConnection()
{
    QTcpSocket* client = server->nextPendingConnection();
//    clientList << client;
    qDebug() << __func__ << client;
    QObject::connect(client,&QTcpSocket::readyRead,this,&Widget::onReadyRead);
}

