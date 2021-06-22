#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractTableModel>

class LogModel : public QAbstractTableModel{
    Q_OBJECT

public:
    explicit LogModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int count(QtMsgType logType);
    int debugCount();
    int infoCount();
    int warningCount();
    int criticalCount();
    int fatalCount();
    static QString mapToString(const QtMsgType msgType);
public slots:
    void clear();
    bool pushMessage(QtMsgType type,const QMessageLogContext& context,const QString& msg);
signals:
    void logUpdated();
private:
    QStringList dbColumns;
};
extern QPointer<LogModel> logStorage;
#endif // LOGMODEL_H
