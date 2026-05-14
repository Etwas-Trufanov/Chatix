#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include "json.hpp"
#include "llmconnector.hpp"
#include <QDialog>

namespace settingsData {
    enum lmprovider {
        LMSTER,
        OLLAMA,
        NONE
    };
    struct TSettings {
        // Текущий провайдер
        lmprovider provider;
        // Модель для lms
        QString lmsterModelName;
        // Модель для ollama
        QString ollamaModelName;
        // Адресс для lms
        QString lmsIp;
        // Адресс для ollama
        QString ollamaIp;
        // Имя пользователя
        QString userName;

        TSettings(lmprovider provider,
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
        TSettings() : provider(settingsData::NONE), lmsIp("http://localhost:1234"), lmsterModelName("google/gemma-4-e4b"),
            ollamaIp("http://localhost:11434"), ollamaModelName("granite4.1:3b"), userName("") {}

        TSettings(const QString &userName) : userName(userName) {
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
    };
}

namespace Ui {
class settingsWindow;
}

class settingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit settingsWindow(QWidget *parent = nullptr);

    settingsData::TSettings param;

    ~settingsWindow();

private slots:
    void on_buttonBox_accepted();

    void on_ProviderSelectorBox_currentIndexChanged(int index);

    void on_modelSelector_currentIndexChanged(int index);

private:
    Ui::settingsWindow *ui;
};

#endif // SETTINGSWINDOW_H
