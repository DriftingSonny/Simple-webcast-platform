#include "pack.h"

Pack::Pack()
{
    count = 0;
    pack_size = 6;//协议包整体大小初始化为变长包的定长部分的大小
    memset(buf,0,4096);
}

void Pack::setType(Type type)
{
    this->type = type;
}

void Pack::setErr(Err err)
{
    this->err = err;
}

Type Pack::getType()
{
    return (Type)type;
}

Err Pack::getErr()
{
    return (Err)err;
}

// 所有想要在网络中发送的数据，全都通过append方法，写入buf里面去
void Pack::append(const QString &data)
{
    short size = strlen(data.toLocal8Bit().data());// 先计算一下即将存入buf的数据有多长
    memcpy(buf+count,&size,2);//将数据的大小size，存入buf未使用的最靠前的2个字节里面
    count += 2;//写入size之后，buf就用掉了2个字节，所以count要自增2
    
    memcpy(buf+count,data.toLocal8Bit().data(),size);
    count += size;

    pack_size = 6 + count;//pack_size 永远等于 协议包的定长部分大小 + 变长部分大小
}

Pack &Pack::operator<<(const QString &data)
{
    append(data);
    return *this;
}

// 读取buf里面存放的所有数据，并且将每一个数据构建成QString后，存入一个链表里面，最后返回该链表
QStringList Pack::readAll()
{
    QStringList list;
    int readed_size = 0;//记录已经从buf里面读取了多少字节的变量，以便下一次读取的时候，从正确的内存地址开始读取
    while(1){
        short size = 0;
        memcpy(&size,buf+readed_size,2);//先读取2个字节，以便之后即将读取的数据有多大
        readed_size += 2;
        if(size==0){
            //如果size没变，说明读取到了2个空字节，基本能够说明已经将所有数据读取完毕了
            break;
        }
        char temp[size+1];//准备一个size+1大小的数组，用来接受即将读取的大小为size的数据
        memset(temp,0,size+1);
        memcpy(temp,buf+readed_size,size);
        readed_size += size;
        
        QString data = QString::fromLocal8Bit(temp);//以支持中的形式，将读取到的数据，构建成QString
        list << data;
    }
    clear();
    return list;
}

char *Pack::data()
{
    return (char*)this;
}

int Pack::size()
{
    //return 2 + count; // 2 + strlen(count)
    return pack_size;
}

void Pack::clear()
{
    count = 0;
    pack_size = 6;
    memset(buf,0,4096);
}

void Pack::append(const char *data, int size)
{
    memset(buf,0,4096);
    memcpy(buf,data,size);
    pack_size = size + 6;
}

QByteArray Pack::readImage()
{
    QByteArray arr = QByteArray::fromBase64(buf);
    return arr;
}
