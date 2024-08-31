#pragma once

#include <ui_limitnvpstate.h>
#include <unordered_set>

class limitnvpstate : public QMainWindow {
    Q_OBJECT

public:
    limitnvpstate(QWidget* parent = nullptr);

private:
    Ui::limitnvpstateClass ui;

    void createTrayIcon();
    void saveProcessExceptions();
    void getAvailablePStates();

private slots:
    void selectedGPUChanged(int index);
    void selectedPStateChanged(int index);
    void addProcess();

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* e) override;
};

