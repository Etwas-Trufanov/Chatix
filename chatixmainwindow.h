#ifndef CHATIXMAINWINDOW_H
#define CHATIXMAINWINDOW_H
#include <QMainWindow>
#include <qlistwidget.h>
#include <vector>

#include "llmconnector.hpp"
#include "lmmanager.hpp"
#include "settingswindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatixMainWindow;
}
QT_END_NAMESPACE

// Структура чата
struct chatElement {
    bool isGenerating = false;
    nlohmann::json data;
    chatElement(nlohmann::json systemPromt) : data(systemPromt) {}
    ~chatElement() = default;
};

class ChatixMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatixMainWindow(QWidget *parent = nullptr);
    ~ChatixMainWindow() override;

private slots:

    void on_sendButton_clicked();

    void on_hideChatListButton_clicked();

    void on_newChatButton_clicked();

    void on_chatList_itemClicked(QListWidgetItem *item);

    void on_settingsButton_clicked();

private:

    Ui::ChatixMainWindow *ui;

    settingsData::TSettings current_settings;

    nlohmann::json genStartMessage(const QString &modelName, const QString &userName);

    void switchToChat(std::size_t index);

    std::vector<chatElement> chats;

    bool chatListHidedByUser = false;

    std::size_t curChatID = 0;

    std::unique_ptr<lmc::LLMClient> provider;
    std::unique_ptr<lmManagers::lmManager> manager;

    std::string context;

    QString genMD(std::size_t chatID);

    protected:
    void resizeEvent(QResizeEvent *event) override;
};
#endif // CHATIXMAINWINDOW_H
