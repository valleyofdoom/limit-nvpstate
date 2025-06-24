#pragma once

#include <ui_limitnvpstate.h>
#include <unordered_set>

class limitnvpstate : public QMainWindow {
    Q_OBJECT

public:
    limitnvpstate(QWidget* parent = nullptr);

private:
    Ui::limitnvpstateClass ui;

    void setupProcessRunningTrigger();
    void stopProcessRunningTrigger();
    void setupProcessForegroundTrigger();
    void stopProcessForegroundTrigger();
    void createTrayIcon();
    void saveProcessExceptions();
    void getAvailablePStates();

private slots:
    void unlimitTriggerChanged(int index);
    void selectedGPUChanged(int index);
    void selectedPStateChanged(int index);
    void addProcess();
    void exitApp(int exitCode);

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* e) override;
};

