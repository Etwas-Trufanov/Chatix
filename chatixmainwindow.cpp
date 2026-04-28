#include "chatixmainwindow.h"
#include "./ui_chatixmainwindow.h"
#include "lmmanager.hpp"
#include "llmconnector.hpp"
#include "settingswindow.h"
#include <QtQuickWidgets/QQuickWidget>
#include <iostream>
#include <QResizeEvent>
#include <QIcon>

ChatixMainWindow::ChatixMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatixMainWindow),
    model(std::make_unique<lmc::lmster>("http://localhost:8080/v1/chat/completions"))
{
    ui->setupUi(this);
    chats.push_back(startMessage);
    ui->chatList->addItem("Chat 0");
    int isLmsInstalled = lmmanager::checkLmsInstallation();
    qDebug() << "LMS state:" << isLmsInstalled;
    if (isLmsInstalled != 0) {
        #ifdef _WIN32
            QString cmd = QString::fromStdString("powershell -Command \"irm https://lmstudio.ai/install.ps1 | iex\"");
        #else
            QString termCommand = QString::fromStdString("curl -fsSL https://lmstudio.ai/install.sh | bash");
        #endif
            qDebug() << "lms not installed!";
    }

    // В планах:
    // Загружаем конфиг из памяти

    // Сейчас
    // Опрашиваем все провайдеры по очереди пока не найдём первый доступный


}

ChatixMainWindow::~ChatixMainWindow()
{
    delete ui;
}

QString ChatixMainWindow::genMD(std::size_t chatID) {
    QString chatString;
    for (std::size_t i = 1; i < chats[chatID].data["messages"].size(); i++) {
        chatString += QString::fromStdString(chats[chatID].data["messages"][i]["content"]);
        chatString += "\n\n";
    }
    std::cout << chatString.toStdString() << std::endl;
    return chatString;
}

void ChatixMainWindow::on_sendButton_clicked()
{
    std::size_t tmpID = curChatID;
    try {
        // Добавляем вопрос пользователя в контекст
        chats[tmpID].data["messages"].push_back({
            {"role", "user"},
            {"content", ui->questionEdit->toPlainText().toStdString()}
        });
        // Очищаем поле ввода
        ui->questionEdit->clear();
        // Блокируем кнопку и меняем в состояние генерации
        // И обновляем текст
        chats[tmpID].isGenerating = true;
        if (tmpID == curChatID) {
            ui->sendButton->setEnabled(false);
            ui->chatBox->setMarkdown(genMD(tmpID));
        }
        // Получаем ответ от сервера
        qDebug() << "Send message to LLM provider";
        nlohmann::json response = model->call_answer(chats[tmpID].data);
        if (!response.is_null()) {
            std::string llmContent = response["choices"][0]["message"]["content"];
            chats[tmpID].data["messages"].push_back({
                {"role", "assistant"},
                {"content", llmContent}
            });
        }
        // Обновляем текст
        chats[tmpID].isGenerating = true;
        if (tmpID == curChatID) {
            ui->sendButton->setEnabled(true);
            ui->chatBox->setMarkdown(genMD(tmpID));
        }
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
        ui->sendButton->setEnabled((chats[index].isGenerating) ? false : true);
        curChatID = index;
        ui->chatBox->setMarkdown(genMD(index));
    }
}

void ChatixMainWindow::on_settingsButton_clicked()
{
    settingsWindow *settings = new settingsWindow();
    settings->exec();
}

