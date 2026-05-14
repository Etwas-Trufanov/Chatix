#include "inputnamewindow.h"
#include "ui_inputnamewindow.h"
#include "checkUserName.h"
#include <qregularexpression.h>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <pwd.h>
#endif

QString inputnamewindow::getSystemUsername() {
#ifdef _WIN32
    char username[256];
    DWORD size = sizeof(username);

    if (GetUserNameA(username, &size))
        return QString::fromStdString(std::string(username));

    return "Unknown";
#else
    struct passwd *pw = getpwuid(getuid());

    if (pw)
        return QString::fromStdString(std::string(pw->pw_name));

    return "Unknown";
#endif
}

inputnamewindow::inputnamewindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::inputnamewindow)
{
    ui->setupUi(this);
    userName = getSystemUsername();
    ui->nameEdit->setText(userName);
}

inputnamewindow::~inputnamewindow()
{
    delete ui;
}

void inputnamewindow::on_doneButton_clicked()
{
    if (ui->nameEdit->text() != userName)  {
        if (nameChecker::nameIsGood(ui->nameEdit->text())) {
            userName = ui->nameEdit->text();
            accept();
        } else {
            ui->nameEdit->setStyleSheet("");
        }
    }
}

