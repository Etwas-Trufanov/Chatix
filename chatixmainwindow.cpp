#include "chatixmainwindow.h"
#include "./ui_chatixmainwindow.h"
#include "llmconnector.hpp"
#include <QtQuickWidgets/QQuickWidget>
#include <iostream>
#include <QResizeEvent>

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

QString ChatixMainWindow::genMD() {
    QString chatString;
    for (std::size_t i = 1; i < chats[curChatID]["messages"].size(); i++) {
        chatString += QString::fromStdString(chats[curChatID]["messages"][i]["content"]);
        chatString += "\n\n";
    }
    std::cout << chatString.toStdString() << std::endl;
    return chatString;
}

void ChatixMainWindow::on_sendButton_clicked()
{
    try {
        // Добавляем вопрос пользователя в контекст
        chats[curChatID]["messages"].push_back({
            {"role", "user"},
            {"content", ui->questionEdit->toPlainText().toStdString()}
        });
        // Получаем ответ от сервера
        ui->chatBox->setMarkdown(genMD());
        nlohmann::json response = model->call_answer(chats[curChatID]);
        if (!response.is_null()) {
            std::string llmContent = response["choices"][0]["message"]["content"];
            chats[curChatID]["messages"].push_back({
                {"role", "assistant"},
                {"content", llmContent}
            });
        }
        // Выводим новый вопрос-ответ
        ui->chatBox->setMarkdown(genMD());
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
    }
}



void ChatixMainWindow::on_hideChatListButton_clicked()
{
    bool currentlyVisible = ui->chatList->isVisible();

    // Переключаем видимость
    ui->chatList->setVisible(!currentlyVisible);
    ui->chatListLabel->setVisible(!currentlyVisible);

    // Обновляем флаг
    chatListHidedByUser = currentlyVisible;
}

void ChatixMainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    qDebug() << event->size().width();

    if (event->size().width() <= 450) {
        ui->chatList->setVisible(false);
        ui->chatListLabel->setVisible(false);
    } else if (event->size().width() > 450) {
        if (!chatListHidedByUser) {
            qDebug() << chatListHidedByUser << " " << "Теперь видно";
            ui->chatList->setVisible(true);
            ui->chatListLabel->setVisible(true);
        }
    }
}
