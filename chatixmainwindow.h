#ifndef CHATIXMAINWINDOW_H
#define CHATIXMAINWINDOW_H
#include <QMainWindow>
#include <qlistwidget.h>
#include <vector>
#include <QKeyEvent>

#include "llmconnector.hpp"
//#include "lmmanager.hpp"
#include "settingswindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatixMainWindow;
}
QT_END_NAMESPACE

// Структура чата
struct chatElement {
    bool isGenerating = false;           // состояние ожидания ответа
    nlohmann::json data;                 // хранимая переписка
    QString title;                       // заголовок чата

    // Конструктор
    // systemPrompt - системный промпт
    // title - название чата
    chatElement(nlohmann::json systemPromt, const QString &title = "Новый чат")
        : data(systemPromt), title(title) {}
    ~chatElement() = default;           // Деструктор
};

class ChatixMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatixMainWindow(QWidget *parent = nullptr);
    ~ChatixMainWindow() override;

private slots:

    // Обработчик отправки сообщения
    void on_sendButton_clicked();

    // Обработчик скрытия списка чатов
    void on_hideChatListButton_clicked();

    // Обработчик создания нового чата
    void on_newChatButton_clicked();

    // Обработчик переключения чата
    void on_chatList_itemClicked(QListWidgetItem *item);

    // Обработчик открытия настроек
    void on_settingsButton_clicked();

    void on_saveAction_triggered();

    void on_clearHistoryAction_triggered();

    void on_closeAction_triggered();

private:

    Ui::ChatixMainWindow *ui;

    settingsData::TSettings current_settings;

    nlohmann::json genStartMessage(const QString &modelName, const QString &userName);

    // Загрузка истории чата с диска
    // Возвращает false при ошибке
    bool loadChatHistoryAndSettings();

    // Сохранение истории чата на диск
    // В windows \Users\<Имя_пользователя>\AppData\Local\Chatix\chatix_data.json
    // В linux ~/.local/share/Chatix/chatix_data.json
    // Возвразает false при ошибке
    bool saveChatHistoryAndSettings();

    // Добавление чата с припиской даты
    // string - название чата, к нему добавляется дата
    void addChatByDate(const QString &string);

    // Переключение на чат
    // Index - индекс чата
    void switchToChat(std::size_t index);

    // Вектор чатов
    std::vector<chatElement> chats;

    // Скрыт ли список чатов пользователем
    bool chatListHidedByUser = false;

    // Индекс текущего чата
    std::size_t curChatID = 0;

    // Провайдер и менеджер
    std::unique_ptr<lmc::LLMClient> provider;
    // std::unique_ptr<lmManagers::lmManager> manager;          // (пока не реализован)

    // Функция генерации красивого
    QString genHTML(std::size_t chatID);

    protected:

    // Переопределённый эвент закрытия
    void closeEvent(QCloseEvent *event) override;
    // Переопределённый эвент изменения размера
    void resizeEvent(QResizeEvent *event) override;
    // Переопределение эвента нажатия клавиши
    bool eventFilter(QObject *obj, QEvent *event) override;
};
#endif // CHATIXMAINWINDOW_H
