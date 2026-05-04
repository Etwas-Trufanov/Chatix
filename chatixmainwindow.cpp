#include "chatixmainwindow.h"
#include <QtQuickWidgets/QQuickWidget>
#include <QResizeEvent>
#include <QIcon>

#include "./ui_chatixmainwindow.h"
#include "lmmanager.hpp"
#include "llmconnector.hpp"
#include "settingswindow.h"

ChatixMainWindow::ChatixMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatixMainWindow)
{
    ui->setupUi(this);

    // Ищем конфиг
    // тут типа код



    settingsData::TSettings current_settings = settingsData::TSettings(
        settingsData::LMSTER,
        "http://localhost:1234/v1/chat/completions",
        "google/gemma-4-e4b",
        "http://localhost:11434",
        "granite4.1:3b"
        );

    try {
        switch (current_settings.provider) {
        case settingsData::LMSTER:
            provider = std::make_unique<lmc::LMStudioClient>(current_settings.lmsIp.toStdString());
            manager = std::make_unique<lmManagers::lmstudioManager>();
            chats.push_back(genStartMessage(current_settings.lmsterModelName, "Petya"));
            break;
        case settingsData::OLLAMA:
            // ✅ Было LMStudioClient, меняем на OllamaClient
            provider = std::make_unique<lmc::OllamaClient>(current_settings.ollamaIp.toStdString());
            manager = std::make_unique<lmManagers::ollamaManager>();
            chats.push_back(genStartMessage(current_settings.ollamaModelName, "Petya"));
            break;
        }
        manager->startServer();
    } catch (const std::exception& e) {
        qDebug() << "Error:" << e.what();
    }

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
    return chatString;
}

nlohmann::json ChatixMainWindow::genStartMessage(const QString &modelName, const QString &userName) {
    return {
        {"model", modelName.toUtf8().toStdString()},
        {"messages", {
                         {
                             {"role", "system"},
                             {"content", "Ты полезный ассистент, а пользователя зовут: " + userName.toUtf8().toStdString()}
                         }
                     }},
        {"temperature", 0.7},
        {"stream", false}
    };
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
        qDebug() << "Send message to LLM provider...";
        // Получаем ответ от сервера
        nlohmann::json response = provider->call_answer(chats[tmpID].data);
        qDebug() << "Message was пришло..." << response.dump(2);

        if (!response.is_null()) {
            std::string llmContent = response["choices"][0]["message"]["content"];
            chats[tmpID].data["messages"].push_back({
                {"role", "assistant"},
                {"content", llmContent}
            });
        }
        // Обновляем текст
        chats[tmpID].isGenerating = false;
        if (tmpID == curChatID) {
            ui->sendButton->setEnabled(true);
            ui->chatBox->setMarkdown(genMD(tmpID));
        }
    } catch (std::runtime_error &e) {
        qDebug() << e.what();
    } catch (nlohmann::json_abi_v3_12_0::detail::type_error &e) {
        qDebug() << "Ошибка" << e.what();
    }
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
    switch (current_settings.provider) {
    case settingsData::lmprovider::LMSTER:
        chats.push_back(genStartMessage(current_settings.lmsterModelName, "Petya"));
        break;
    case settingsData::lmprovider::OLLAMA:
        chats.push_back(genStartMessage(current_settings.ollamaModelName, "Petya"));
        break;
    }

    ui->chatList->addItem("Chat " + QString::number(chats.size() - 1));
    switchToChat(chats.size() - 1);
}


void ChatixMainWindow::on_chatList_itemClicked(QListWidgetItem *item) {
    int index = ui->chatList->row(item);
    switchToChat(index);
}

void ChatixMainWindow::switchToChat(std::size_t index) {
    if (index >= 0 && index < static_cast<int>(chats.size())) {
        qDebug() << "Состояние чата " << (chats[index].isGenerating ? "генерирует" : "простаивает");
        ui->sendButton->setEnabled((!chats[index].isGenerating));
        curChatID = index;
        ui->chatBox->setMarkdown(genMD(index));
    }
}

void ChatixMainWindow::on_settingsButton_clicked()
{
    settingsWindow *settings = new settingsWindow();

    settings->param = current_settings;


    if (settings->exec() == QDialog::Accepted) {
        current_settings.provider = settings->param.provider;
        switch (current_settings.provider) {
        case settingsData::lmprovider::OLLAMA:
            provider.reset();
            provider = std::make_unique<lmc::OllamaClient>(current_settings.ollamaIp.toStdString());
            qDebug() << "Now ollama";
            break;
        case settingsData::lmprovider::LMSTER:
            provider.reset();
            provider = std::make_unique<lmc::LMStudioClient>(current_settings.lmsIp.toStdString());
            qDebug() << "Now lms";
            break;
        }
    }
}

