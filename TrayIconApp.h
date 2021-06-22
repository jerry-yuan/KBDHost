#ifndef TRAYICONAPP_H
#define TRAYICONAPP_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QThread>
#include <QPointer>
#include <AbstractSubApplication.h>
#include "qusb.h"
class TrayIconApp : public QSystemTrayIcon
{
    Q_OBJECT
public:
    TrayIconApp(QObject *parent=nullptr);
    ~TrayIconApp();
public slots:
    void exit();
    void deviceAttached(QUsb::Id deviceId);
    void deviceDettached(QUsb::Id deviceId);
signals:
    void keyboardAttached();
    void keyboardDettached();
private:
    QThread* usbGlobalThread=NULL;
    QUsb* usbGlobal = NULL;
    QAction* exitAction = NULL;
    QMenu* trayIconMenu=NULL;

    volatile int deviceFlag = 0;

    // Tools
    QList<AbstractSubApplication*> tools;

};

extern QPointer<TrayIconApp> trayIconApp;

#endif // TRAYICONAPP_H
