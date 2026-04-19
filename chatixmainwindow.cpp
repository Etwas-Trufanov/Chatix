#include "chatixmainwindow.h"
#include "./ui_chatixmainwindow.h"
#include "llmconnector.hpp"
#include <QtQuickWidgets/QQuickWidget>
#include <iostream>
#include <QResizeEvent>
#include <QIcon>

ChatixMainWindow::ChatixMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatixMainWindow)
    , model(std::make_unique<lmc::remote_llm>("http://localhost:1234/v1/chat/completions"))
{
    ui->setupUi(this);
    chats.push_back(startMessage);
    ui->chatList->addItem("Chat 0");
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
        // Обновляем текст
        ui->chatBox->setMarkdown(genMD());
        // Очищаем поле ввода
        ui->questionEdit->clear();
        // Получаем ответ от сервера
        ui->sendButton->setEnabled(false);
        nlohmann::json response = model->call_answer(chats[curChatID]);
        if (!response.is_null()) {
            std::string llmContent = response["choices"][0]["message"]["content"];
            chats[curChatID]["messages"].push_back({
                {"role", "assistant"},
                {"content", llmContent}
            });
        }
        // Обновляем текст
        ui->chatBox->setMarkdown(genMD());
        ui->sendButton->setEnabled(true);
    } catch (std::runtime_error &e) {
        qDebug() << e.what();
    }/* catch (...) {
        qDebug() << "Ошибка";
    }*/
}



void ChatixMainWindow::on_hideChatListButton_clicked()
{
    bool currentlyVisible = ui->chatList->isVisible();

    // Переключаем видимость
    ui->chatList->setVisible(!currentlyVisible);
    ui->chatListLabel->setVisible(!currentlyVisible);
    QIcon icon = (currentlyVisible) ? QIcon::fromTheme(QIcon::ThemeIcon::GoNext) : QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious);
    ui->hideChatListButton->setIcon(icon);

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
        QIcon icon = QIcon::fromTheme(QIcon::ThemeIcon::GoNext);
        ui->hideChatListButton->setIcon(icon);
    } else if (event->size().width() > 450) {
        if (!chatListHidedByUser) {
            qDebug() << chatListHidedByUser << " " << "Теперь видно";
            ui->chatList->setVisible(true);
            ui->chatListLabel->setVisible(true);
            QIcon icon = QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious);
            ui->hideChatListButton->setIcon(icon);
        }
    }
}

void ChatixMainWindow::on_newChatButton_clicked() {
    chats.push_back(startMessage);
    ui->chatList->addItem("Chat " + QString::number(chats.size() - 1));
    switchToChat(chats.size() - 1);
}


void ChatixMainWindow::on_chatList_itemClicked(QListWidgetItem *item) {
    int index = ui->chatList->row(item);
    switchToChat(index);
}

void ChatixMainWindow::switchToChat(std::size_t index) {
    if (index >= 0 && index < static_cast<int>(chats.size())) {
        curChatID = index;
        ui->chatBox->setMarkdown(genMD());
    }
}

