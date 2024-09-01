#include <about.h>

about::about(QWidget *parent) : QDialog(parent) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui.setupUi(this);
}
