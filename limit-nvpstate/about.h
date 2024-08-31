#pragma once

#include <QDialog>
#include <ui_about.h>

class about : public QDialog {
    Q_OBJECT

public:
    about(QWidget* parent = nullptr);

private:
    Ui::About ui;
};
