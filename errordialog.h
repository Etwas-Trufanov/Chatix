#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(const QString &errName,
                         const QString &okButtonText,
                         const QString &cancelButtonText,
                         const QString &commandText,
                         QWidget *parent = nullptr);
    ~ErrorDialog();

private:
    Ui::ErrorDialog *ui;
};

#endif // ERRORDIALOG_H
