#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>

namespace settingsData {
    enum lmprovider {
        LMSTER,
        OLLAMA
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

        TSettings(lmprovider provider,
                                        const QString &lmsIp,
                                        const QString &lmsModelName,

                                        const QString &ollamaIp,
                                        const QString &ollamaModelName) :

                                        provider(provider),

                                        lmsIp(lmsIp),
                                        lmsterModelName(lmsModelName),

                                        ollamaIp(ollamaIp),
                                        ollamaModelName(ollamaModelName) {}
        TSettings() : provider(settingsData::LMSTER), lmsIp("http://localhost:1234/v1/chat/completions"), lmsterModelName("google/gemma-4-e4b"),
            ollamaIp("http://localhost:1234/api/chat"), ollamaModelName("granite4.1:3b") {}
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

private:
    Ui::settingsWindow *ui;
};

#endif // SETTINGSWINDOW_H
