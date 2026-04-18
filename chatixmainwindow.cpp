#include "chatixmainwindow.h"
#include "./ui_chatixmainwindow.h"
#include "llmconnector.hpp"
#include <QtQuickWidgets/QQuickWidget>
#include <iostream>

ChatixMainWindow::ChatixMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatixMainWindow)
    , model(std::make_unique<lmc::remote_llm>("http://localhost:1234/v1/chat/completions"))
{
    ui->setupUi(this);
    chats.push_back(startMessage);
}

ChatixMainWindow::~ChatixMainWindow()
{
    delete ui;
}

std::string ChatixMainWindow::genHTML() {
    return "";
}

void ChatixMainWindow::on_sendButton_clicked()
{
    try {
        // Добавляем вопрос пользователя в контекст
        chats[curChatID]["messages"].push_back({
            {"role", "user"},
            {"content", ui->questionEdit->toPlainText().toStdString()}
        });
        // Выводим его в чат
        ui->chatBox->append(ui->questionEdit->toPlainText());
        // Получаем ответ от сервера
        nlohmann::json response = model->call_answer(chats[curChatID]);
        if (!response.is_null()) {
            std::string llmContent = response["choices"][0]["message"]["content"];
            chats[curChatID]["messages"].push_back({
                {"role", "assistant"},
                {"content", llmContent}
            });
        }
        // chats[0]["messages"];
        ui->chatBox->append(QString::fromStdString(response["choices"][0]["message"]["content"]));
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
    }
}


