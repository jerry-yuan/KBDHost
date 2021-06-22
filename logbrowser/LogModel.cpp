#include "logbrowser/LogModel.h"
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QDateTime>
#include <QColor>
LogModel::LogModel(QObject *parent) : QAbstractTableModel(parent){
    QSqlQuery query;
    query.exec("select count(1) as 'count' from sqlite_master where type='table' and name='log';");
    query.next();
    if(query.value("count").toInt()<1){
        qWarning()<<tr("Table %1 is disappeared, try to recreate").arg("log");
        query.exec("CREATE TABLE log ( "
                   "id        INTEGER        PRIMARY KEY AUTOINCREMENT,"
                   "timestamp DATETIME,"
                   "filename  VARCHAR( 50 ),"
                   "lineno    INTEGER,"
                   "level     INTEGER,"
                   "message   TEXT "
                   ");");
        query.exec("CREATE INDEX level_index ON log (level);");
    }
    dbColumns<<"timestamp"<<"level"<<"message"<<"filename"<<"lineno";
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal){
        if(role == Qt::DisplayRole){
            switch(section){
            case 0:
                return QVariant(tr("Timestamp"));
            case 1:
                return QVariant(tr("Level"));
            case 2:
                return QVariant(tr("Message"));
            case 3:
                return QVariant(tr("File name"));
            case 4:
                return QVariant(tr("Lineno"));
            default:
                break;
            }
        }
    }
    return QAbstractTableModel::headerData(section,orientation,role);
}

int LogModel::rowCount(const QModelIndex &parent) const{
    QSqlQuery query("select count(1) from log");
    if (parent.isValid())
        return 0;
    query.next();
    return query.value(0).toInt();
}

int LogModel::columnCount(const QModelIndex &parent) const{
    if (parent.isValid())
        return 0;
    return 5;
}

QVariant LogModel::data(const QModelIndex &index, int role) const{
    QSqlQuery query;
    QVariant result=QVariant();
    if (!index.isValid())
        return QVariant();
    if(role==Qt::DisplayRole){
        if(index.column()>=dbColumns.length()){
            return QVariant();
        }
        QString column=dbColumns.at(index.column());
        query.prepare(QString("select %1 from log where id=?;").arg(column));
        query.bindValue(0,index.row()+1);
        query.exec();
        if(query.next()){
            result=query.value(column);
            switch(index.column()){
            // dbColumns<<"timestamp"<<"level"<<"message"<<"filename"<<"lineno";
            case 0: //timestamp
                result = QVariant::fromValue(result.toDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
                break;
            case 1: // level
                result = mapToString((QtMsgType)result.toInt());
                break;
            default:
                break;
            }
        }else{
            result=query.lastError().text();
        }
    }else if(role==Qt::ForegroundRole){
        query.prepare("select level from log where id=?;");
        query.bindValue(0,index.row()+1);
        query.exec();
        if(query.next()){
            switch((QtMsgType)query.value("level").toInt()){
            case QtMsgType::QtDebugMsg:
                result = QVariant(QColor(Qt::black));
                break;
            case QtMsgType::QtInfoMsg:
                result = QVariant(QColor(Qt::blue));
                break;
            case QtMsgType::QtWarningMsg:
                result = QVariant(QColor(153,102,0));
                break;
            case QtMsgType::QtCriticalMsg:
                result = QVariant(QColor(Qt::red));
                break;
            case QtMsgType::QtFatalMsg:
                result = QVariant(QColor(Qt::red));
                break;
            default:
                result = QVariant(tr("UNKNOW"));
                break;
            }
        }
    }
    return result;

}
bool LogModel::pushMessage(QtMsgType type,const QMessageLogContext& context,const QString& msg){
    QSqlDatabase db;
    QSqlQuery query(db);
    query.prepare("Insert into log (timestamp,filename,lineno,level,message) values (?,?,?,?,?);");
    query.bindValue(0,QDateTime::currentDateTime());
    query.bindValue(1,QString(context.file));
    query.bindValue(2,context.line);
    query.bindValue(3,type);
    query.bindValue(4,msg);
    beginInsertRows(QModelIndex(),rowCount(),rowCount());
    int  ret=query.exec();
    endInsertRows();
    emit logUpdated();
    return ret;
}
int LogModel::count(QtMsgType logType){
    QSqlQuery query;
    query.prepare("select count(1) from log where level=?");
    query.bindValue(0,logType);
    query.exec();
    query.next();
    return query.value(0).toInt();
}
int LogModel::debugCount(){
    return count(QtDebugMsg);
}
int LogModel::infoCount(){
    return count(QtInfoMsg);
}
int LogModel::warningCount(){
    return count(QtWarningMsg);
}
int LogModel::criticalCount(){
    return count(QtCriticalMsg);
}
int LogModel::fatalCount(){
    return count(QtFatalMsg);
}
void LogModel::clear(){
    QSqlQuery query;
    beginResetModel();
    query.exec("delete from log;");
    query.exec("update sqlite_sequence SET seq = 0 where name ='log'");
    endResetModel();
}
QString LogModel::mapToString(const QtMsgType msgType){
    QString result;
    switch(msgType){
    case QtMsgType::QtDebugMsg:
        result=tr("DEBUG");
        break;
    case QtMsgType::QtInfoMsg:
        result = tr("INFO");
        break;
    case QtMsgType::QtWarningMsg:
        result = tr("WARNING");
        break;
    case QtMsgType::QtCriticalMsg:
        result = tr("CRITICAL");
        break;
    case QtMsgType::QtFatalMsg:
        result = tr("FATAL");
        break;
    default:
        result = tr("UNKNOW");
        break;
    }
    return result;
}
