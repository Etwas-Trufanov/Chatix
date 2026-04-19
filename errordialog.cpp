#include "errordialog.h"
#include "ui_errordialog.h"

ErrorDialog::ErrorDialog(const QString &errName, const QString &okButtonText, const QString &cancelButtonText, const QString &commandText, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
    ui->errorText->appendPlainText(errName);
    ui->okButton->setText(okButtonText);
    ui->cancelButton->setText(cancelButtonText);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}
