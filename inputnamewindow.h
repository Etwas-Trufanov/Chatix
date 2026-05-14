#ifndef INPUTNAMEWINDOW_H
#define INPUTNAMEWINDOW_H

#include <QDialog>

namespace Ui {
class inputnamewindow;
}

class inputnamewindow : public QDialog
{
    Q_OBJECT

public:
    explicit inputnamewindow(QWidget *parent = nullptr);

    QString userName;

    ~inputnamewindow();

private slots:
    void on_doneButton_clicked();

private:
    Ui::inputnamewindow *ui;

    QString getSystemUsername();
};

#endif // INPUTNAMEWINDOW_H
