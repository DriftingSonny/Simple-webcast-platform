#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , addr("127.0.0.1")
    , port(12345)
{
    ui->setupUi(this);
    client = new QTcpSocket(this);
    client->connectToHost(addr,port);
    QObject::connect(client,&QTcpSocket::readyRead,this,&Widget::onReadyRead);
    ui->lineEdit_2->setEchoMode(QLineEdit::Password);
}

Widget::~Widget()
{
    t->terminate();
    delete ui;
}

/*
    客户端也是需要处理 分包 粘包问题的，你们自己去处理
*/
void Widget::onReadyRead()
{
    Pack pack;
    while(1){
        int size = 0;
        int res = 0;
        res = client->read((char*)&size,4);
        if(size==0){break;}
        client->read(pack.data()+4,size-4);
        switch(pack.getType()){
            case TYPE_SEND_IMAGE:
                user_interface->onReadyRead(pack);
                break;
            case TYPE_REGIST:
                regist_back(pack);
                break;
            case TYPE_LOGIN:
                login_back(pack);
                break;
            case TYPE_CREAET_ROOM:
                user_interface->onReadyRead(pack);
                break;
            case TYPE_HAS_NEW_ROOM:
                user_interface->onReadyRead(pack);
                break;
            case TYPE_FLUSH_ALL_ROOM:
                user_interface->onReadyRead(pack);
                break;
        }
    }
}


void Widget::regist_back(Pack &pack)
{
    if(pack.getErr() != NO_ERR){
        QMessageBox::critical(this,"注册","该账号已存在");
    }else{
        QMessageBox::information(this,"注册","注册成功");
    }
}

void Widget::login_back(Pack &pack)
{
    if(pack.getErr() != NO_ERR){
        QMessageBox::critical(this,"登录","账号或密码错误");
    }else{
        QMessageBox::information(this,"登录","登录成功");
        user_interface = UserInterface::create_user_interface(user_name,client);
        // 用户界面创建出来之后，就要隐藏当前界面，显示用户界面
        user_interface->show();
        this->hide();
    }
}


// 注册
void Widget::on_pushButton_clicked()
{
    QString name = ui->lineEdit->text();
    QString pswd = ui->lineEdit_2->text();
    Pack pack;
    pack.setType(TYPE_REGIST);
    pack << name << pswd;
    client->write(pack.data(),pack.size());
}

// 登录
void Widget::on_pushButton_2_clicked()
{
    QString name = ui->lineEdit->text();
    QString pswd = ui->lineEdit_2->text();

    user_name = name;

    Pack pack;
    pack.setType(TYPE_LOGIN);
    pack << name << pswd;
    client->write(pack.data(),pack.size());
}

void Widget::on_pushButton_3_clicked()
{
    if (ui->lineEdit_2->echoMode()==QLineEdit::Normal) {
        ui->lineEdit_2->setEchoMode(QLineEdit::Password);
        ui->pushButton_3->setText("显示密码");
    } else if(ui->lineEdit_2->echoMode()==QLineEdit::Password){
        ui->lineEdit_2->setEchoMode(QLineEdit::Normal);
        ui->pushButton_3->setText("隐藏密码");
    }
}
