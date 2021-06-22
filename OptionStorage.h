#ifndef OPTIONSTORAGE_H
#define OPTIONSTORAGE_H

#include <QObject>
#include <QSqlQuery>
#include <QVariant>
class OptionStorage : public QObject
{
    Q_OBJECT
public:
    explicit OptionStorage(QObject *parent = nullptr);
    bool initialize(QString key,QVariant defaultValue=QVariant());
    QVariant read(QString key,QVariant defaultValue=QVariant());
    bool update(QString key, QVariant value);
signals:
private:
};

#endif // OPTIONSTORAGE_H
