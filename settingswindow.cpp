#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "json.hpp"
#include "checkUserName.h"

settingsWindow::settingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settingsWindow)
{
    ui->setupUi(this);
    qDebug() << param.provider;
    ui->ProviderSelectorBox->setCurrentIndex(param.provider);
    if (param.provider == settingsData::LMSTER) {
        lmc::LMStudioClient lms(param.lmsIp.toStdString());
        nlohmann::json modelList = lms.get_models();

        for (auto &i : modelList) {
            ui->modelSelector->addItem(QString::fromStdString(i.get<std::string>()));
        }

    } else {
        lmc::OllamaClient ollama(param.ollamaIp.toStdString());
        nlohmann::json modelList = ollama.get_models();

        for (auto &i : modelList) {
            ui->modelSelector->addItem(QString::fromStdString(i.get<std::string>()));
        }

    }
    lmc::OllamaClient ollama(param.ollamaIp.toStdString());

    nlohmann::json ollamaModels = ollama.get_models();

}

settingsWindow::~settingsWindow()
{
    delete ui;
}

void settingsWindow::on_buttonBox_accepted()
{
    if (!nameChecker::nameIsGood(ui->nameEdit->text())) {
        ui->nameEdit->setStyleSheet("{color: red}");
    } else {
        accept();
    }
}

void settingsWindow::on_ProviderSelectorBox_currentIndexChanged(int index)
{
    param.provider = settingsData::lmprovider(index);
    qDebug() << "index provider in list " << index;
}


void settingsWindow::on_modelSelector_currentIndexChanged(int index)
{
    if (param.provider == settingsData::LMSTER) {
        param.lmsterModelName = ui->modelSelector->itemText(index);
        qDebug() << "if settings set lms model" << param.lmsterModelName;
    } else {
        param.ollamaModelName = ui->modelSelector->itemText(index);
        qDebug() << "if settings set ollama model" << param.ollamaModelName;
    }
}

