#ifndef DBHELPER_H
#define DBHELPER_H

/*
    数据库刀层：也就是所谓的数据库管理类
    所有操作数据库的函数，全都写在这个类里面
    有什么操作写什么函数，反正只要是操作数据库的，必须都通过这个类来操作
*/
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


class DbHelper
{
    QSqlDatabase db;
    QSqlQuery* query;

public:
    // 使用那种数据库，并且打开指定数据库
    DbHelper(const QString& driver,const QString& dbname,const QString& pswd);
    bool regist(const QString& name,const QString& pswd);
    bool login(const QString& name,const QString& pswd);
};

#endif // DBHELPER_H
