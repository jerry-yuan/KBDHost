#ifndef KEYBOARDDEVICE_H
#define KEYBOARDDEVICE_H

#include <QObject>
#include <QIODevice>
#include "qusbdevice.h"
#include "qusbendpoint.h"

class KeyboardDevice : public QIODevice{
    Q_OBJECT
    enum{
        InputEndPoint=0x80,
        OutputEndPoint=0x00
    };

public:
    enum{
        R_ACK = 0x55,
        R_NAK = 0xAA
    };
    enum{
        SyncCommand=0x00,
        RTCGetCounterCommand=0x10,
        RTCSetCounterCommand
    };
    const static int VendorId = 0x1234;
    const static int ProductId = 0x4321;
    const static int InterfaceId = 0x02;
    const static int EndPointId = 0x03;

    KeyboardDevice(QObject *parent=nullptr);
    ~KeyboardDevice();
    qint64 bytesAvailable() const override;
    qint64 bytesToWrite() const override;
    void close() override;
    bool isSequential() const override;
    bool open(OpenMode mode) override;
    bool waitForReadyRead(int msecs) override;
    bool waitForBytesWritten(int msecs) override;

    void setLogLevel(QUsb::LogLevel logLevel);
    QUsb::LogLevel logLevel();

    qint64 write(const char *data, qint64 maxSize);
    qint64 write(const char *data);
    qint64 write(const QByteArray &byteArray);

    qint64 read(char* data, qint64 maxSize);
    QByteArray read(qint64 maxSize);
    QByteArray readAll();
    qint64 readLine(char *data,qint64 maxSize);
    QByteArray readLine(qint64 maxSize=0);

    QByteArray request(quint8 command,QByteArray req,quint8* statusCode=nullptr);
public slots:
    void readyReadDelegate();
    void byteWrittenDelegate(qint64 bytes);
    void errorDelegate(QUsbEndpoint::Status status);
signals:
    void error(QUsbEndpoint* endpoint,QUsbEndpoint::Status status);
protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
private:
    QUsbDevice* usbDevice;
    QUsbEndpoint* inputEndpoint;
    QUsbEndpoint* outputEndpoint;
};

#endif // KEYBOARDDEVICE_H
