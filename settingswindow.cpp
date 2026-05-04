#include "settingswindow.h"
#include "ui_settingswindow.h"

settingsWindow::settingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settingsWindow)
{
    ui->setupUi(this);
    qDebug() << param.provider;
    ui->ProviderSelectorBox->setCurrentIndex(param.provider);
}

settingsWindow::~settingsWindow()
{
    delete ui;
}

void settingsWindow::on_buttonBox_accepted()
{
    accept();
}

void settingsWindow::on_ProviderSelectorBox_currentIndexChanged(int index)
{
    param.provider = settingsData::lmprovider(index);
    qDebug() << "index provider in list " << index;
}

