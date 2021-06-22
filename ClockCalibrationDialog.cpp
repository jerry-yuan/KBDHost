#include "ClockCalibrationDialog.h"
#include "ui_ClockCalibrationDialog.h"
#include "KeyboardDevice.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QMessageBox>
#include "OptionStorage.h"
#include "TrayIconApp.h"
ClockCalibrationDialog::ClockCalibrationDialog(QWidget *parent) : QDialog(parent),
    ui(new Ui::ClockCalibrationDialog){
    OptionStorage os;
    ui->setupUi(this);
    // 初始化时间进率转换
    ui->timeUnit->addItem(tr("Minutes"),60);
    ui->timeUnit->addItem(tr("Hours"),3600);
    ui->timeUnit->addItem(tr("Days"),86400);
    ui->timeUnit->addItem(tr("Weeks"),604800);
    // 初始化选项数据库
    os.initialize(optionSyncEnabled,QVariant::fromValue(true));
    os.initialize(optionSyncCount,QVariant::fromValue(1));
    os.initialize(optionSyncUnit,QVariant::fromValue(2));
    os.initialize(optionNextSync,QVariant::fromValue(QDateTime::currentDateTime()));
    // 初始化触发菜单
    this->subMenu = new QMenu(tr("RTC Clock"));
    this->showAction = new QAction(QIcon(":/icon/settings.png"),tr("Settings(&S)"));
    this->calibrateAction = new QAction(QIcon(":/icon/update.png"),tr("Calibrate with host clock (&C)"));

    this->subMenu->setIcon(QIcon(":/icon/clock.png"));

    this->subMenu->addAction(this->calibrateAction);
    this->subMenu->addAction(this->showAction);

    connect(this->showAction,&QAction::triggered,this,&ClockCalibrationDialog::show);
    connect(this->calibrateAction,&QAction::triggered,this,&ClockCalibrationDialog::syncClockTriggered);
    // 初始化定时器
    this->refreshTimer=new QTimer(this);
    this->refreshTimer->setInterval(500);
    connect(this->refreshTimer,&QTimer::timeout,this,&ClockCalibrationDialog::refreshClock);

    this->autoSyncTimer = new QTimer(this);
    this->autoSyncTimer->setInterval(1000);
    connect(this->autoSyncTimer,&QTimer::timeout,this,&ClockCalibrationDialog::checkAutoSync);
    // 初始化UI逻辑
    connect(ui->syncSwitch,&QCheckBox::toggled,ui->timeEdit,&QTimeEdit::setDisabled);
    connect(ui->syncSwitch,&QCheckBox::toggled,ui->resetToNowBtn,&QPushButton::setDisabled);
    connect(ui->syncSwitch,&QCheckBox::toggled,ui->syncLabel,&QLabel::setEnabled);
    connect(ui->syncSwitch,&QCheckBox::toggled,ui->nextSyncLabel,&QLabel::setVisible);
    connect(ui->syncSwitch,&QCheckBox::toggled,ui->nextSync,&QLabel::setVisible);
    connect(ui->syncSwitch,&QCheckBox::toggled,ui->timeCount,&QSpinBox::setEnabled);
    connect(ui->syncSwitch,&QCheckBox::toggled,ui->timeUnit,&QComboBox::setEnabled);

    connect(ui->resetToNowBtn,&QPushButton::clicked,this,&ClockCalibrationDialog::resetTimeEdit);
    connect(ui->setTimeBtn,&QPushButton::clicked,this,&ClockCalibrationDialog::setupTimeImmediately);
    connect(ui->saveBtn,&QPushButton::clicked,this,&ClockCalibrationDialog::saveConfiguration);
    // 读取数据库设置
    this->autoSyncEnabled=os.read(optionSyncEnabled).toBool();
    this->nextSyncTime = os.read(optionNextSync).toDateTime();
    this->timeCount = os.read(optionSyncCount).toInt();
    this->timeUnit = os.read(optionSyncUnit).toInt();
}

ClockCalibrationDialog::~ClockCalibrationDialog(){
    delete ui;
}
void ClockCalibrationDialog::showEvent(QShowEvent *){
    this->syncTimeDiff();

    ui->syncSwitch->setChecked(this->autoSyncEnabled);
    emit ui->syncSwitch->toggled(this->autoSyncEnabled);
    ui->timeCount->setValue(this->timeCount);
    ui->timeUnit->setCurrentIndex(this->timeUnit);

    refreshClock();
    resetTimeEdit();

    this->refreshTimer->start();
}
void ClockCalibrationDialog::closeEvent(QCloseEvent *){
    this->refreshTimer->stop();
}

void ClockCalibrationDialog::regActions(QMenu *contextMenu){
    contextMenu->addMenu(this->subMenu);
}
void ClockCalibrationDialog::refreshClock(){
    QDateTime currentDateTime=QDateTime::currentDateTime();
    // 显示主机时间
    ui->pcTime->display(currentDateTime.toString("yyyy-MM-dd HH:mm:ss"));
    // 显示键盘时间
    ui->kbdTime->display(currentDateTime.addSecs(timeDiff).toString("yyyy-MM-dd HH:mm:ss"));
    // 显示下次对时时间
    if(QDateTime::currentDateTime().secsTo(this->nextSyncTime)<0){
        ui->nextSync->setText(tr("Immediately when saved config."));
    }else{
        ui->nextSync->setText(this->nextSyncTime.toString(tr("yyyy-MM-dd HH:mm:ss")));
    }

}

void ClockCalibrationDialog::keyboardAttached(){
    this->subMenu->setDisabled(false);
    // 启动自动同步定时器
    if(ui->syncSwitch->isChecked()){
        this->autoSyncTimer->start();
    }
}
void ClockCalibrationDialog::keyboardDettached(){
    this->hide();
    this->subMenu->setDisabled(true);
    if(ui->syncSwitch->isChecked()){
        this->autoSyncTimer->stop();
    }
}
void ClockCalibrationDialog::syncClockTriggered(){
    OptionStorage os;
    if(this->setupTime(QDateTime::currentDateTime())){
        this->timeDiff=0;
        this->nextSyncTime = QDateTime::currentDateTime().addSecs(this->timeCount*ui->timeUnit->itemData(this->timeUnit).toInt());
        trayIconApp->showMessage(
                    tr("Auto RTC Calibration"),
                    tr("Successfully sync the keyboard RTC clock to host clock.\nNext Sync:%1")
                        .arg(this->nextSyncTime.toString("yyyy-MM-dd HH:mm:ss")),
                    QSystemTrayIcon::Information);
    }else{
        this->nextSyncTime = QDateTime::currentDateTime().addSecs(600);
        trayIconApp->showMessage(tr("Auto RTC Calibration"),tr("Failed to sync the keyboard RTC clock with host clock, retry will be carried in 10 minutes."),QSystemTrayIcon::Warning);
    }
    os.update(optionNextSync,this->nextSyncTime);
}

void ClockCalibrationDialog::resetTimeEdit(){
    ui->timeEdit->setDateTime(QDateTime::currentDateTime());
}
void ClockCalibrationDialog::checkAutoSync(){
    QDateTime currentDateTime=QDateTime::currentDateTime();
    if(currentDateTime.secsTo(this->nextSyncTime)<0){
        this->syncClockTriggered();
    }
}
void ClockCalibrationDialog::setupTimeImmediately(){
    QDateTime calibrationTime;
    QDateTime currentTime=QDateTime::currentDateTime();
    if(ui->syncSwitch->isChecked()){
        calibrationTime = currentTime;
    }else{
        calibrationTime = ui->timeEdit->dateTime();
    }
    this->timeDiff=currentTime.secsTo(calibrationTime);

    if(this->setupTime(calibrationTime)){
        QMessageBox::information(this,tr("Success"),tr("Keyboard time set successfully."),tr("Ok"));
    }else{
        QMessageBox::warning(this,tr("Fail"),tr("Failed to set keyboard time. Please refer to the log for details."),tr("Ok"));
    }

}
void ClockCalibrationDialog::saveConfiguration(){
    OptionStorage os;
    qint64 secsToNextSync;
    this->autoSyncEnabled=ui->syncSwitch->isChecked();
    this->timeCount=ui->timeCount->value();
    this->timeUnit=ui->timeUnit->currentIndex();
    secsToNextSync = this->timeCount*ui->timeUnit->itemData(this->timeUnit).toInt();
    if(QDateTime::currentDateTime().secsTo(this->nextSyncTime)>secsToNextSync){
        this->nextSyncTime = QDateTime::currentDateTime().addSecs(secsToNextSync);
    }
    if(this->autoSyncEnabled^this->autoSyncTimer->isActive()){
        if(this->autoSyncEnabled){
            this->autoSyncTimer->start();
        }else{
            this->autoSyncTimer->stop();
        }
    }
    // 写回OptionStorage;
    os.update(optionSyncEnabled,this->autoSyncEnabled);
    os.update(optionSyncCount,this->timeCount);
    os.update(optionSyncUnit,this->timeUnit);
    os.update(optionNextSync,this->nextSyncTime);
    // 隐藏窗口
    this->hide();
}
void ClockCalibrationDialog::syncTimeDiff(){
    KeyboardDevice keyboardDevice;
    quint8 statusCode;
    if(!keyboardDevice.open(QIODevice::ReadWrite)){
        qWarning()<<keyboardDevice.errorString();
        return;
    }
    QByteArray rtcTimestampData=keyboardDevice.request(KeyboardDevice::RTCGetCounterCommand,QByteArray(),&statusCode);
    keyboardDevice.close();
    // 小端模式转换为数字
    uint32_t rtcTimestamp=0;
    memcpy(&rtcTimestamp,rtcTimestampData.data(),sizeof(rtcTimestamp));
    this->timeDiff = rtcTimestamp - QDateTime::currentSecsSinceEpoch();
}
bool ClockCalibrationDialog::setupTime(QDateTime calibrationTime){
    KeyboardDevice keyboardDevice;
    quint32 timestamp;
    QByteArray buffer;
    quint8 statusCode;
    if(!keyboardDevice.open(QIODevice::ReadWrite)){
        qWarning()<<keyboardDevice.errorString();
        return false;
    }
    timestamp = calibrationTime.toTime_t();
    keyboardDevice.request(KeyboardDevice::RTCSetCounterCommand,QByteArray((char*)&timestamp,4),&statusCode);
    keyboardDevice.close();
    return statusCode==KeyboardDevice::R_ACK;
}
