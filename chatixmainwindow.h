#ifndef CHATIXMAINWINDOW_H
#define CHATIXMAINWINDOW_H
#include "llmconnector.hpp"
#include <QMainWindow>
#include <qlistwidget.h>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatixMainWindow;
}
QT_END_NAMESPACE

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

private:

    Ui::ChatixMainWindow *ui;

    const nlohmann::json startMessage = nlohmann::json::parse(R"({
"messages": [
  {"role": "system", "content": "Ты полезный ассистент."}
],
"temperature": 0.7,
"max_tokens": -1
})");

    void switchToChat(std::size_t index);

    std::vector<nlohmann::json> chats;

    bool chatListHidedByUser = false;

    std::size_t curChatID = 0;

    std::unique_ptr<lmc::llm> model;

    std::string context;

    QString genMD();

    protected:
    void resizeEvent(QResizeEvent *event) override;
};
#endif // CHATIXMAINWINDOW_H
