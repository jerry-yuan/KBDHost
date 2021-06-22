#include "logbrowser/LogDialog.h"
#include "ui_LogDialog.h"
#include <QPointer>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
QPointer<LogModel> logStorage;
LogDialog::LogDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogDialog){
    ui->setupUi(this);
    ui->logView->setModel(logStorage);
    this->triggerAction = new QAction(QIcon(":/icon/log.png"),tr("Logs"),this);

    ui->exportProgress->hide();

    connect(this->triggerAction,&QAction::triggered,this,&LogDialog::show);
    connect(ui->clearBtn,&QPushButton::clicked,logStorage,&LogModel::clear);
    connect(ui->clearBtn,&QPushButton::clicked,this,&LogDialog::updateStatistic);
    connect(ui->saveBtn,&QPushButton::clicked,this,&LogDialog::doExport);
}

LogDialog::~LogDialog(){
    delete ui;
}
void LogDialog::regActions(QMenu* contextMenu){
    contextMenu->addAction(this->triggerAction);
}
void LogDialog::updateStatistic(){
    ui->debugCount->display(logStorage->debugCount());
    ui->infoCount->display(logStorage->infoCount());
    ui->warningCount->display(logStorage->warningCount());
    ui->criticalCount->display(logStorage->criticalCount());
    ui->fatalCount->display(logStorage->fatalCount());
}
void LogDialog::showEvent(QShowEvent *){
    connect(logStorage,SIGNAL(logUpdated()),ui->logView,SLOT(resizeColumnsToContents()));
    connect(logStorage,SIGNAL(logUpdated()),this,SLOT(updateStatistic()));
    updateStatistic();
    ui->logView->resizeColumnsToContents();
}
void LogDialog::closeEvent(QCloseEvent *){
    disconnect(logStorage,SIGNAL(logUpdated()),ui->logView,SLOT(resizeColumnsToContents()));
    disconnect(logStorage,SIGNAL(logUpdated()),this,SLOT(updateStatistic()));
}
void LogDialog::handleError(QString title,QString message){
    QMessageBox::critical(this,title,message,tr("Ok"));
}
void LogDialog::doExport(){
    QString outPath = QFileDialog::getSaveFileName(NULL,tr("Save As..."),"",tr("Log File (*.log)"));
    if(outPath.isEmpty()){
        return;
    }
    QSqlQuery query;

    //读取有多少行
    if(!query.exec("select count(1) from log")){
        QMessageBox::critical(this,tr("Failed to read data"),tr("Unable to read database:%1").arg(query.lastError().text()),tr("Ok"));
        return;
    }
    query.next();
    int total=query.value(0).toInt();
    int progress=0;
    if(total<1){
        QMessageBox::critical(this,tr("No available logs"),tr("There is no logs to export!"),tr("Ok"));
        return;
    }
    query.exec("select timestamp,filename,lineno,level,message from log");
    QFile file(outPath);

    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(this,tr("Failed to open file"),tr("Unable to open file:%1").arg(outPath),tr("Ok"));
        return;
    }
    QString logTemplate=QString("[%1][%2]:%3@%4:%5\n");
    ui->exportProgress->setRange(0,total);
    ui->exportProgress->setVisible(true);
    ui->saveBtn->setEnabled(false);
    ui->clearBtn->setEnabled(false);
    while(query.next()){
        file.write(logTemplate
                   .arg(LogModel::mapToString((QtMsgType)query.value("level").toInt()))
                   .arg(query.value("timestamp").toDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                   .arg(query.value("filename").toString())
                   .arg(query.value("lineno").toInt())
                   .arg(query.value("message").toString())
                   .toLocal8Bit());
        progress++;
        ui->exportProgress->setValue(progress);
    }
    file.close();
    ui->exportProgress->setVisible(false);
    ui->saveBtn->setEnabled(true);
    ui->clearBtn->setEnabled(true);
}

