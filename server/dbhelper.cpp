#include "dbhelper.h"

DbHelper::DbHelper(const QString &driver, const QString &dbname,const QString& pswd)
{
    db = QSqlDatabase::addDatabase(driver);
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword(pswd);
    db.setDatabaseName(dbname);
    bool res = db.open();
    if(res==false){
        qDebug() << db.lastError();
    }else{
        query = new QSqlQuery;
    }
}

bool DbHelper::regist(const QString &name, const QString &pswd)
{
    // 用来做注册登录的表单为 tb2
    query->prepare("insert into tb2(name,pswd) values(:name,:pswd)");
    query->bindValue(":name",name);
    query->bindValue(":pswd",pswd);
    bool res = query->exec();
    if(res==false){
        qDebug() << query->lastError();
    }
    return res;//注意将注册成功与否向外返回，以便外部服务器得知注册是否成功，好告诉客户端
}

bool DbHelper::login(const QString &name, const QString &pswd)
{
    query->prepare("select pswd from tb2 where name=:name");
    query->bindValue(":name",name);
    query->exec();
    bool res = query->next();
    //没找到账号，res会是false,找到了账号res会是true
    if(res==false){
        return false;
    }

    // 找到账号的情况下，判断密码是否正确
    if(pswd != query->value("pswd")){
        return false;
    }

    return true;
}
