#include "OptionStorage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
OptionStorage::OptionStorage(QObject *parent) : QObject(parent){
    QSqlQuery query;
    query.exec("select count(1) as 'count' from sqlite_master where type='table' and name='options';");
    query.next();
    if(query.value("count").toInt()<1){
        qWarning()<<tr("Table %1 is disappeared, try to recreate").arg("options");
        query.exec("CREATE TABLE options ( "
                   "id    INTEGER     PRIMARY KEY AUTOINCREMENT,"
                   "[key] CHAR( 10 )  UNIQUE ON CONFLICT REPLACE,"
                   "value TEXT "
                   ");");
        query.exec("CREATE INDEX key_index ON options ([key]);");
    }
}

bool OptionStorage::initialize(QString key,QVariant defaultValue){
    QSqlQuery query;
    query.prepare("select count(1) as 'count' from options where key=?;");
    query.bindValue(0,key);
    if(!query.exec()){
        qWarning()<<tr("failed to check %1 in options storage:").arg(key)<<query.lastError().text();
        return false;
    }
    query.next();
    if(query.value("count").toInt()<1){
        query.prepare("insert into options (key,value) values (?,?);");
        query.bindValue(0,key);
        query.bindValue(1,defaultValue);
        return query.exec();
    }
    return true;
}

QVariant OptionStorage::read(QString key,QVariant defaultValue){
    QSqlQuery query;
    query.prepare("select value from options where key=?;");
    query.bindValue(0,key);
    query.exec();
    if(query.next()){
        return query.value("value");
    }else{
        return defaultValue;
    }
}
bool OptionStorage::update(QString key,QVariant value){
    QSqlQuery query;
    query.prepare("update options set value=? where key=?;");
    query.bindValue(0,value);
    query.bindValue(1,key);
    return query.exec();
}
