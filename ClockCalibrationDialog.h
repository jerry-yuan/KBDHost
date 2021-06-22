#ifndef CLOCKCALIBRATIONDIALOG_H
#define CLOCKCALIBRATIONDIALOG_H

#include <QDialog>
#include <AbstractSubApplication.h>
#include <QTimer>
#include <QDateTime>
namespace Ui {
class ClockCalibrationDialog;
}

class ClockCalibrationDialog : public QDialog,public AbstractSubApplication
{
    Q_OBJECT

public:
    explicit ClockCalibrationDialog(QWidget *parent = nullptr);
    ~ClockCalibrationDialog();
    void regActions(QMenu *contextMenu) override;
public slots:
    void refreshClock();
    void keyboardAttached() override;
    void keyboardDettached() override;
    void syncTimeDiff();
    void checkAutoSync();
    void setupTimeImmediately();
    void resetTimeEdit();
    void syncClockTriggered();
    void saveConfiguration();
protected:
    void showEvent(QShowEvent *) override;
    void closeEvent(QCloseEvent *) override;
private:
    bool setupTime(QDateTime dateTime);
    const QString optionSyncEnabled = "rtc_sync_enabled";
    const QString optionSyncCount = "rtc_sync_time_count";
    const QString optionSyncUnit = "rtc_sync_time_unit";
    const QString optionNextSync = "rtc_next_sync";
    Ui::ClockCalibrationDialog *ui;
    QMenu* subMenu;
    QAction* calibrateAction;
    QAction* showAction;

    QTimer* refreshTimer;
    QTimer* autoSyncTimer;

    bool autoSyncEnabled;
    QDateTime nextSyncTime;
    qint32 timeCount;
    qint32 timeUnit;

    qint32 timeDiff = 0;

};

#endif // CLOCKCALIBRATIONDIALOG_H
