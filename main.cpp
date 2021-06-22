#include "TrayIconApp.h"

#include <QApplication>
#include <QDebug>
#include <QList>
#include <QTranslator>
#include <QDebug>
#include <QPointer>
#include <QSqlDatabase>
#include <QDateTime>
#include "logbrowser/LogModel.h"
#include "KeyboardDevice.h"
void logMessageHandler(QtMsgType type,const QMessageLogContext &context,const QString &msg){
    bool ret;
    QMetaObject::invokeMethod(logStorage,"pushMessage",
                                       Q_RETURN_ARG(bool,ret),
                                       Q_ARG(QtMsgType,type),
                                       Q_ARG(const QMessageLogContext&,context),
                                       Q_ARG(const QString&,msg));
    if(!ret||true){
        // 写入日志失败,写标准输入输出
        QByteArray localMsg = msg.toLocal8Bit();
        QByteArray msgTime=QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz").toLocal8Bit();
        const char *file = context.file ? context.file : "";
        switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "[%s][%s]:%s@%u: %s\n", logStorage->tr("DEBUG").toLocal8Bit().constData(),msgTime.constData(),file,context.line,localMsg.constData());
            break;
        case QtInfoMsg:
            fprintf(stderr, "[%s][%s]:%s@%u: %s\n", logStorage->tr("INFO").toLocal8Bit().constData(),msgTime.constData(),file,context.line,localMsg.constData());
            break;
        case QtWarningMsg:
            fprintf(stderr, "[%s][%s]:%s@%u: %s\n", logStorage->tr("WARNING").toLocal8Bit().constData(),msgTime.constData(),file,context.line,localMsg.constData());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "[%s][%s]:%s@%u: %s\n", logStorage->tr("CRITICAL").toLocal8Bit().constData(),msgTime.constData(),file,context.line,localMsg.constData());
            break;
        case QtFatalMsg:
            fprintf(stderr, "[%s][%s]:%s@%u: %s\n", logStorage->tr("FATAL").toLocal8Bit().constData(),msgTime.constData(),file,context.line,localMsg.constData());
            break;
        }
        fflush(stderr);
    }
}

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false);
    // 初始化内存数据库
    QSqlDatabase defaultDatabase=QSqlDatabase::addDatabase("QSQLITE");
    defaultDatabase.setDatabaseName("test.db");
    defaultDatabase.open();
    // 初始化日志系统
    logStorage=QPointer<LogModel>(new LogModel());
    qInstallMessageHandler(logMessageHandler);
    // 根据系统语言加载翻译包
    QLocale locale=QLocale::system();
    QTranslator* translator=new QTranslator(&a);
    QString translationFile=QString(":/i18n/KBDHost_%1.qm").arg(locale.name());
    if(QFile::exists(translationFile)){
        translator->load(translationFile);
        a.installTranslator(translator);
    }
    // 初始化并启动主程序
    trayIconApp=new TrayIconApp(&a);
    return a.exec();
}
