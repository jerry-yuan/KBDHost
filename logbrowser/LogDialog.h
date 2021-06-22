#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>
#include "AbstractSubApplication.h"
#include "logbrowser/LogModel.h"

namespace Ui {
class LogDialog;
}

class LogDialog : public QDialog,public AbstractSubApplication
{
    Q_OBJECT

public:
    explicit LogDialog(QWidget *parent = nullptr);
    ~LogDialog();
    void regActions(QMenu* contextMenu) override;
public slots:
    void updateStatistic();
    void handleError(QString title,QString message);
    void doExport();
protected:
    void showEvent(QShowEvent *) override;
    void closeEvent(QCloseEvent *) override;

private:
    Ui::LogDialog *ui;
    QAction* triggerAction=NULL;
};

#endif // LOGDIALOG_H
