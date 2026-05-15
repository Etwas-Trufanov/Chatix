#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

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


        TSettings();
        TSettings(const QString &userName);
        TSettings(lmprovider provider,
                  const QString &lmsIp,
                  const QString &lmsModelName,

                  const QString &ollamaIp,
                  const QString &ollamaModelName,
                  const QString &userName);

        QString getSystemUsername();
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

    int showDialog();

    ~settingsWindow();

private slots:
    void on_buttonBox_accepted();

    void on_ProviderSelectorBox_currentIndexChanged(int index);

    void on_modelSelector_currentIndexChanged(int index);

    void on_buttonBox_rejected();

private:
    Ui::settingsWindow *ui;

    void UpdateSetModelList();
};

#endif // SETTINGSWINDOW_H
