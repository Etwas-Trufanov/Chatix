#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>

namespace Ui {
class settingsWindow;
}

class settingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit settingsWindow(QWidget *parent = nullptr);
    ~settingsWindow();

private:
    Ui::settingsWindow *ui;
};

#endif // SETTINGSWINDOW_H
