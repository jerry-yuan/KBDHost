#include "TrayIconApp.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

#include "KeyboardDevice.h"

#include "ClockCalibrationDialog.h"
#include "logbrowser/LogDialog.h"

QPointer<TrayIconApp> trayIconApp;
TrayIconApp::TrayIconApp(QObject* parent):
    QSystemTrayIcon(parent){
    // 初始化托盘图标
    this->setToolTip(tr("Keyboard Host"));
    this->setIcon(QIcon(":/icon/keyboard.png"));
    // 显示托盘图标
    this->show();

    // 初始化Tools
    this->tools.append(new ClockCalibrationDialog(NULL));
    this->tools.append(new LogDialog(NULL));

    // 初始化菜单动作
    this->exitAction = new QAction(tr("Exit(&Q)"),this);
    connect(this->exitAction,SIGNAL(triggered()),this,SLOT(exit()));

    // 生成菜单
    this->trayIconMenu = new QMenu();

    for(int i=0;i<this->tools.size();i++){
        AbstractSubApplication* application=tools.at(i);
        application->regActions(this->trayIconMenu);
        connect(this,&TrayIconApp::keyboardAttached,[=](){application->keyboardAttached();});
        connect(this,&TrayIconApp::keyboardDettached,[=](){application->keyboardDettached();});
    }

    this->trayIconMenu->addAction(this->exitAction);
    this->setContextMenu(this->trayIconMenu);

    // 初始化USB
    this->usbGlobalThread=new QThread();
    this->usbGlobal = new QUsb();
    this->usbGlobal->moveToThread(this->usbGlobalThread);
    this->usbGlobalThread->start();
    // 遍历USB设备初始化
    QUsb::IdList devices=QUsb::devices();
    foreach(QUsb::Id id,devices){
        this->deviceAttached(id);
    }
    // 绑定热插拔监听事件
    connect(this->usbGlobal,&QUsb::deviceInserted,this,&TrayIconApp::deviceAttached,Qt::QueuedConnection);
    connect(this->usbGlobal,&QUsb::deviceRemoved,this,&TrayIconApp::deviceDettached,Qt::QueuedConnection);

    // 显示启动提示
    //this->showMessage(tr("Keyboard Host"),tr("Keyboard host started!"),icon());
}
TrayIconApp::~TrayIconApp(){

}
void TrayIconApp::exit(){
    int result=QMessageBox::question(NULL,tr("Exit"),tr("Are you sure to exit?"),tr("Yes(&Y)"),tr("No(&N)"));
    if(result==0){
        this->hide();
        qApp->quit();
    };
}

void TrayIconApp::deviceAttached(QUsb::Id deviceId){
    if(deviceId.vid==KeyboardDevice::VendorId && deviceId.pid==KeyboardDevice::ProductId){
        this->deviceFlag++;
        if(this->deviceFlag==3){
            qInfo("Device Attached:%04X %04X",deviceId.vid,deviceId.pid);
            emit this->keyboardAttached();
            this->showMessage(tr("Keyboard connected"),tr("The keyboard has been connected to PC."));
        }
    }
}
void TrayIconApp::deviceDettached(QUsb::Id deviceId){
    if(deviceId.vid==KeyboardDevice::VendorId && deviceId.pid==KeyboardDevice::ProductId){
        this->deviceFlag--;
        if(this->deviceFlag==2){
            qInfo("Device Dettached:%04X %04X",deviceId.vid,deviceId.pid);
            emit this->keyboardDettached();
            this->showMessage(tr("Keyboard removed"),tr("The keyboard has been removed, all features depends on keyboard will be disabled."));
        }
    }
}
