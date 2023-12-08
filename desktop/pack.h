#ifndef PACK_H
#define PACK_H

/*
    协议包：里面会存放所有想要发送的数据，然后使用套接字里面 write(char* buf,int size) 将整个协议包一起发送出去
    定长包：优点只有1个：好学，简单
        缺点很多：浪费流量，很不灵活

    所以我们要重新设计定长包，将他改成变长包的形式，并且尽可能的节省流量
*/

#include <QObject>
#include <cstring>
#include <QStringList>
#include <QByteArray>

typedef unsigned char uchar;

enum Type{
    TYPE_REGIST,
    TYPE_LOGIN,
    TYPE_SEND_IMAGE,
    TYPE_CREAET_ROOM,
    TYPE_JOIN_ROOM,
    TYPE_HAS_NEW_ROOM,
    TYPE_FLUSH_ALL_ROOM
};

enum Err{
    NO_ERR,
    ERR_NAME,
    ERR_PSWD,
    ERR_LAST_PIX
};

class Pack
{
    int pack_size;//用来记录协议包的整体大小，以便在读取协议包的时候，先读取4个字节，知道协议包多大，再根据协议包的整体大小，-4个字节之后，读取协议包的剩余数据
    uchar type;// uchar类型的数据，在 0~255 的取值范围内，和int类型的数据完全兼容
    uchar err;
    char buf[4096];// 所有想要发送的数据，全都往buf里面去存，1个数据的存储逻辑为：前2个字节为数据的长度，后面的字节为数据本身
    // 例如，我想要存储 helloworld这个字符串，我们需要用掉buf里面12个字节，前2个字节存放 helloworld 的长度，也就是short 10，后10个字节，存放helloworld本身
    int count;//用来记录buf内存的使用情况，buf用掉了多少个字节，count就是多少，但是注意count不打算进行发送。所有写在buf后面的属性，全都不会在网络中进行发送。以便节省流量。怎么不发送：每次网络中发送数据的时候，都要明确发送的字节量。我们只要在计算字节量的时候，特意的不去计算count这4个字节就会保证count这个数据不会被发送
public:
    Pack();
    void setType(Type type);
    void setErr(Err err);
    Type getType();
    Err getErr();

    void append(const QString& data);
    Pack& operator<<(const QString& data);
    QStringList readAll();
    char* data();
    int size();
    void clear();

    void append(const char* data,int size);
    QByteArray readImage();

};

#endif // PACK_H
