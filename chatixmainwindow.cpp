#include "chatixmainwindow.h"
#include "./ui_chatixmainwindow.h"
#include <QtQuickWidgets/QQuickWidget>

ChatixMainWindow::ChatixMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatixMainWindow)
{
    ui->setupUi(this);
}

ChatixMainWindow::~ChatixMainWindow()
{
    delete ui;
}
