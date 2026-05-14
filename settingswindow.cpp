#include "settingswindow.h"
#include "llmconnector.hpp"
#include "ui_settingswindow.h"
#include "checkUserName.h"
#include "json.hpp"

#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <pwd.h>
#endif

QString settingsData::TSettings::getSystemUsername(){
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

settingsData::TSettings::TSettings(lmprovider provider,
                                   const QString &lmsIp,
                                   const QString &lmsModelName,

                                   const QString &ollamaIp,
                                   const QString &ollamaModelName,
                                   const QString &userName):

                                    provider(provider),

                                    lmsIp(lmsIp),
                                    lmsterModelName(lmsModelName),

                                    ollamaIp(ollamaIp),
                                    ollamaModelName(ollamaModelName),
                                    userName(userName) {}


settingsData::TSettings::TSettings() : provider(settingsData::NONE), lmsIp("http://localhost:1234"), lmsterModelName("google/gemma-4-e4b"),
    ollamaIp("http://localhost:11434"), ollamaModelName("granite4.1:3b"), userName(getSystemUsername()) {
}

settingsData::TSettings::TSettings(const QString &userName) : userName(userName) {
    ollamaIp = "http://localhost:11434";
    lmsIp = "http://localhost:1234";
    provider = lmprovider::NONE;

    nlohmann::json ollamaModels;
    nlohmann::json lmsModels;

    // Ollama
    try {
        auto ollamaClient = lmc::OllamaClient(ollamaIp.toStdString());
        ollamaModels = ollamaClient.get_models();

        qDebug() << "Ollama models:";
        qDebug() << ollamaModels.dump(2);

        if (!ollamaModels.empty()) {
            ollamaModelName =
                QString::fromStdString(ollamaModels[0].get<std::string>());
        }

    } catch (const std::exception& e) {
        qDebug() << "Ollama error:" << e.what();
    }

    // LMStudio
    try {
        auto lmsClient = lmc::LMStudioClient(lmsIp.toStdString());
        lmsModels = lmsClient.get_models();

        qDebug() << "LMStudio models:";
        qDebug() << lmsModels.dump(2);

        if (!lmsModels.empty()) {
            lmsterModelName =
                QString::fromStdString(lmsModels[0].get<std::string>());

            provider = lmprovider::LMSTER;
        }

    } catch (const std::exception& e) {
        qDebug() << "LMStudio error:" << e.what();
    }

    // fallback на Ollama
    if (provider == lmprovider::NONE && !ollamaModels.empty()) {
        provider = lmprovider::OLLAMA;
    }

}

void settingsWindow::UpdateSetModelList() {    // Блокируем сигналы, чтобы авто-выбор первого элемента не вызвал слот
    ui->modelSelector->blockSignals(true);
    ui->modelSelector->clear();

    if (param.provider == settingsData::LMSTER) {
        lmc::LMStudioClient lms(param.lmsIp.toStdString());
        nlohmann::json modelList = lms.get_models();

        for (auto &i : modelList) {
            ui->modelSelector->addItem(QString::fromStdString(i.get<std::string>()));
        }

        // Пытаемся найти модель по названию и подставить её в список
        if (!param.lmsterModelName.isEmpty()) {
            int index = ui->modelSelector->findText(param.lmsterModelName);
            if (index != -1) {
                ui->modelSelector->setCurrentIndex(index);
                qDebug() << "Model in list find in pos:" << index;
            }
        }
    }
    else if (param.provider == settingsData::OLLAMA) {
        lmc::OllamaClient ollama(param.ollamaIp.toStdString());
        nlohmann::json modelList = ollama.get_models();

        for (auto &i : modelList) {
            ui->modelSelector->addItem(QString::fromStdString(i.get<std::string>()));
        }

        if (!param.ollamaModelName.isEmpty()) {
            int index = ui->modelSelector->findText(param.ollamaModelName);
            if (index != -1) {
                ui->modelSelector->setCurrentIndex(index);
                qDebug() << "Model in list find in pos:" << index;
            }
        }
    }
    // Разблокируем сигналы
    ui->modelSelector->blockSignals(false);
}

settingsWindow::settingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settingsWindow)
{
    ui->setupUi(this);
}

settingsWindow::~settingsWindow()
{
    delete ui;
}

void settingsWindow::on_buttonBox_accepted()
{
    if (!checks::isValidUsername(ui->nameEdit->text())) {
        ui->nameEdit->setStyleSheet(
            "QLineEdit {"
            "    color: #d46a6a;"
            "    border: 1px solid #d46a6a;"
            "    border-radius: 6px;"
            "}"
            );
    } else {
        param.userName = ui->nameEdit->text();
        accept();
    }
}

void settingsWindow::on_ProviderSelectorBox_currentIndexChanged(int index)
{
    param.provider = settingsData::lmprovider(index);
    UpdateSetModelList();
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

int settingsWindow::showDialog() {
    ui->nameEdit->setText(param.userName);
    qDebug() << "Raw provider id" << param.provider;
    ui->ProviderSelectorBox->setCurrentIndex(param.provider);
    UpdateSetModelList();
    qDebug() << "lms model" << param.lmsterModelName;
    return exec();
}