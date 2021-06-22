#include "KeyboardDevice.h"

#include <QEventLoop>

KeyboardDevice::KeyboardDevice(QObject* parent):QIODevice(parent){
    this->usbDevice = new QUsbDevice(this);
    // pid,vid
    //this->usbDevice->setId(QUsb::Id(0x4321,0x1234));
    this->usbDevice->setId(QUsb::Id(ProductId,VendorId));
    // config id,interface id,alternative
    this->usbDevice->setConfig(QUsb::Config(1,InterfaceId,0));
    // set the loglevel
    this->usbDevice->setLogLevel(QUsb::logWarning);

    this->inputEndpoint = new QUsbEndpoint(this->usbDevice,QUsbEndpoint::bulkEndpoint,InputEndPoint|EndPointId);
    this->outputEndpoint = new QUsbEndpoint(this->usbDevice,QUsbEndpoint::bulkEndpoint,OutputEndPoint|EndPointId);

    connect(this->inputEndpoint,&QUsbEndpoint::readyRead,this,&KeyboardDevice::readyReadDelegate);
    connect(this->outputEndpoint,&QUsbEndpoint::bytesWritten,this,&KeyboardDevice::byteWrittenDelegate);

    connect(this->inputEndpoint,&QUsbEndpoint::error,this,&KeyboardDevice::errorDelegate);
    connect(this->outputEndpoint,&QUsbEndpoint::error,this,&KeyboardDevice::errorDelegate);
}

KeyboardDevice::~KeyboardDevice(){

}
bool KeyboardDevice::waitForReadyRead(int msecs){
    return this->inputEndpoint->waitForReadyRead(msecs);
}
bool KeyboardDevice::waitForBytesWritten(int msecs){
    return this->outputEndpoint->waitForBytesWritten(msecs);
}
qint64 KeyboardDevice::bytesAvailable() const{
    return this->inputEndpoint->bytesAvailable();
}
qint64 KeyboardDevice::bytesToWrite() const{
    return this->outputEndpoint->bytesToWrite();
}
void KeyboardDevice::close(){
    if(this->inputEndpoint->isOpen()){
        this->inputEndpoint->close();
    }
    if(this->outputEndpoint->isOpen()){
        this->outputEndpoint->close();
    }
    if(this->usbDevice->isConnected()){
        this->usbDevice->close();
    }
}
bool KeyboardDevice::isSequential() const{
    return true;
}
bool KeyboardDevice::open(OpenMode mode){
    int ret;
    // 打开USB设备
    ret=this->usbDevice->open();
    if(ret!=QUsbDevice::statusOK){
        this->setErrorString(tr("failed to open usb device: %1").arg(ret));
        return false;
    }
    // 打开输入接口
    if(mode & ReadOnly){
        if(this->inputEndpoint->open(ReadOnly)){
            this->inputEndpoint->setPolling(true);
        }else{
            this->setErrorString(tr("failed to open input endpoint:%1.").arg(this->inputEndpoint->errorString()));
            this->close();
            return false;
        }
    }
    // 打开输出接口
    if(mode & WriteOnly){
        if(!this->outputEndpoint->open(WriteOnly)){
            this->setErrorString(tr("failed to open output endpoint:%1").arg(this->outputEndpoint->errorString()));
            this->close();
            return false;
        }
    }
    return true;
}
qint64 KeyboardDevice::readData(char *data, qint64 maxlen){
    // I am just a delegate, do nothing in real io
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
}
qint64 KeyboardDevice::writeData(const char *data, qint64 len){
    // I am just a delegate, do nothing in real io
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}
qint64 KeyboardDevice::write(const char *data, qint64 maxSize){
    return this->outputEndpoint->write(data,maxSize);
}

qint64 KeyboardDevice::write(const char *data){
    return this->outputEndpoint->write(data);
}
qint64 KeyboardDevice::write(const QByteArray &byteArray){
    return this->outputEndpoint->write(byteArray);
}

qint64 KeyboardDevice::read(char* data, qint64 maxSize){
    return this->inputEndpoint->read(data,maxSize);
}
QByteArray KeyboardDevice::read(qint64 maxSize){
    return this->inputEndpoint->read(maxSize);
}
QByteArray KeyboardDevice::readAll(){
    return this->inputEndpoint->readAll();
}
qint64 KeyboardDevice::readLine(char *data, qint64 maxSize){
    return this->inputEndpoint->readLine(data,maxSize);
}
QByteArray KeyboardDevice::readLine(qint64 maxSize){
    return this->inputEndpoint->readLine(maxSize);
}

void KeyboardDevice::readyReadDelegate(){
    emit readyRead();
}
void KeyboardDevice::byteWrittenDelegate(qint64 bytes){
    emit bytesWritten(bytes);
}
void KeyboardDevice::errorDelegate(QUsbEndpoint::Status status){
    qDebug()<<"error from "<<sender()<<":"<<status;
    emit error((QUsbEndpoint*)sender(),status);
}

void KeyboardDevice::setLogLevel(QUsb::LogLevel logLevel){
    this->usbDevice->setLogLevel(logLevel);
}
QUsb::LogLevel KeyboardDevice::logLevel(){
    return this->usbDevice->logLevel();
}
QByteArray KeyboardDevice::request(quint8 command,QByteArray req,quint8* statusCode){
    QByteArray requestPackage = QByteArray();
    QByteArray responsePackage = QByteArray();
    quint8 status;
    quint16 packageLength = req.size();
    QEventLoop loop;

    if(statusCode==nullptr){
        statusCode = &status;
    }

    // 命令
    requestPackage.append(command);
    // 参数区长度
    requestPackage.append((char*)&packageLength,sizeof(packageLength));
    // 参数区
    requestPackage.append(req);
    // 发送请求
    this->write(requestPackage);
    // 等待数据到来
    connect(this->inputEndpoint,&QUsbEndpoint::readyRead,&loop,&QEventLoop::quit);
    loop.exec();
    disconnect(this->inputEndpoint,&QUsbEndpoint::readyRead,&loop,&QEventLoop::quit);
    // 读取状态码
    this->read((char*)statusCode,1);
    // 读取包长度
    this->read((char*)&packageLength,2);
    // 读取响应主体
    QByteArray responseBody;
    while(responseBody.length()<packageLength){
        this->waitForReadyRead(10);
        responseBody.append(this->read(qMin(1000,packageLength-responseBody.length())));
    }
    return responseBody;
}
