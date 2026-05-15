#include "chatixmainwindow.h"
#include <QtQuickWidgets/QQuickWidget>
#include <QResizeEvent>
#include <QIcon>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <qstandardpaths.h>


#include "./ui_chatixmainwindow.h"
// #include "lmmanager.hpp" // Пока не реализован
#include "llmconnector.hpp"
#include "settingswindow.h"
#include "easyAnswer.h"

// Конструкктор
ChatixMainWindow::ChatixMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatixMainWindow)
{
    ui->setupUi(this);
    ui->questionEdit->installEventFilter(this);
    qDebug() << "Первый пустой конструктор";
    current_settings = settingsData::TSettings();

    bool goodInit = false;

    while (!goodInit) {
        // Инициализация с обработкой ошибок
        try {
            qDebug() << "Второй конструктор settingsData::TSettings(\"\")";
            current_settings = settingsData::TSettings(current_settings.userName);
            // В зависимости от провайдера
            qDebug() << "Raw provider id in chatix constructor" << current_settings.provider;
            switch (current_settings.provider) {
                case settingsData::LMSTER:
                    goodInit = true;
                    provider = std::make_unique<lmc::LMStudioClient>(current_settings.lmsIp.toStdString());
                    // manager = std::make_unique<lmManagers::lmstudioManager>();
                    break;
                case settingsData::OLLAMA:
                    goodInit = true;
                    provider = std::make_unique<lmc::OllamaClient>(current_settings.ollamaIp.toStdString());
                    // manager = std::make_unique<lmManagers::ollamaManager>();
                    break;
                default:
                    QMessageBox::information(parentWidget(), "Ошибка", "Не запущен ни один провайдер:\nOllama\nLMStudio\nЗапустите их и нажмите OK");
                    break;
            }

        } catch (const std::exception &e) {
            qDebug() << "Error:" << e.what();
        }
    }

    if (!loadChatHistoryAndSettings()) {
        qDebug() << "Первичный запуск, вызываем настройки";
        on_settingsButton_clicked();
    } else {
        qDebug() << "Загружено чатов:" << chats.size();
        if (ui->chatList->count() == 0 && !chats.empty()) {
            for (const auto &chat : chats) {
                ui->chatList->addItem(chat.title);
            }
        }
    }
    addChatByDate("Чат");
    switchToChat(chats.size()-1);
}

// Деструктор
ChatixMainWindow::~ChatixMainWindow()
{
    delete ui;
}

// Преобразование текста нейронки в что-то хоть как-то красивое
// chatID - ID чата
QString ChatixMainWindow::genHTML(std::size_t chatID) {
    QString html;

    html += R"(
    <div style="
        font-family: sans-serif;
        padding: 10px;
    ">
    )";

    for (std::size_t i = 1; i < chats[chatID].data["messages"].size(); i++) {

        QString role = QString::fromStdString(
            chats[chatID].data["messages"][i]["role"]
            );

        QString content = QString::fromStdString(
                              chats[chatID].data["messages"][i]["content"]
            );//.toHtmlEscaped();

        content.replace("\n", "<br>");

        bool isUser = (role == "user");

        QString align = isUser ? "left" : "right";
        QString bgColor = "#EFF0F1";

        // Справа - ответ нейросети
        if (align == "right") {
            html += QString(R"(
                <div style="
                    width: 100%;
                    margin-top: 20px;
                    margin-left: 50px;
                    text-align: left;
                ">
                    <div style="
                        display: inline-block;
                        max-width: 70%;
                        padding: 10px;
                        border-radius: 12px;
                        background-color: %2;
                        word-wrap: break-word;
                    ">
                        %3
                    </div>
                </div>
            )")
                        .arg(bgColor)
                        .arg(content);
        } else {
            html += QString(R"(
                <div style="
                    width: 100%;
                    margin-top: 20px;
                    margin-right: 50px;
                    text-align: left;
                ">
                    <div style="
                        display: inline-block;
                        max-width: 70%;
                        padding: 10px;
                        border-radius: 12px;
                        background-color: %2;
                        word-wrap: break-word;
                    ">
                        %3
                    </div>
                </div>
            )")
                        .arg(bgColor)
                        .arg(content);
        }
    }

    html += "</div>";

    return html;
}

// Генерация стандартного стартового сообщения
nlohmann::json ChatixMainWindow::genStartMessage(const QString &modelName, const QString &userName) {
    return {
        {"model", modelName.toUtf8().toStdString()},
        {"messages", {
                         {
                             {"role", "system"},
                       {"content", "Ты полезный ассистент, используй HTML форматирование: <b>текст</b>, <i>текст</i>, <mark></mark> и другие. НИ В КОЕМ СЛУЧАЕ НЕ ИСПОЛЬЗУЙ MD ФОРМАТИРОВАНИЕ!" + (!userName.toUtf8().toStdString().empty() ? " Имя пользователя: " + userName.toUtf8().toStdString() : "")}
                         }
                     }},
        {"temperature", 0.7},
        {"stream", false}
    };
}

// Добавление чата, позволяет указать название чата (в будущем для мультиязычности)
void ChatixMainWindow::addChatByDate(const QString &baseTitle) {
    // Создаём системный промпт для нового чата
    nlohmann::json startMsg;
    if (current_settings.provider == settingsData::LMSTER) {
        startMsg = genStartMessage(current_settings.lmsterModelName, current_settings.userName);
    } else {
        startMsg = genStartMessage(current_settings.ollamaModelName, current_settings.userName);
    }

    // Формируем заголовок с датой/временем
    QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    QString title = baseTitle + " " + dateTime;

    // Добавляем чат в вектор
    chats.emplace_back(startMsg, title);

    // Добавляем элемент в список
    ui->chatList->addItem(title);
}

// Обработка отправки сообщения к LLM
void ChatixMainWindow::on_sendButton_clicked() {
    std::size_t tmpID = curChatID;

    // Обновляем название модели
    if (current_settings.provider == settingsData::lmprovider::LMSTER) {
        chats[tmpID].data["model"] = current_settings.lmsterModelName.toStdString();
        qDebug() << chats[tmpID].data.dump(2);
    } else if (current_settings.provider == settingsData::lmprovider::OLLAMA) {
        chats[tmpID].data["model"] = current_settings.ollamaModelName.toStdString();
        qDebug() << chats[tmpID].data.dump(2);
    }

    try {

        // Добавляем вопрос пользователя в контекст
        chats[tmpID].data["messages"].push_back({
            {"role", "user"},
            {"content", ui->questionEdit->toPlainText().toStdString()}
        });

        // Проверяем на простоту вопроса
        bool ok = true;
        auto tmpAnswer = easyAnswer::answerOnEasyQuestion(ui->questionEdit->toPlainText(), ok);
        // Очищаем поле ввода
        ui->questionEdit->clear();
        if (!ok) {
            // Блокируем кнопку и меняем в состояние генерации
            // И обновляем текст
            chats[tmpID].isGenerating = true;
            if (tmpID == curChatID) {
                ui->sendButton->setEnabled(false);
                ui->chatBox->setHtml(genHTML(tmpID));
            }
            qDebug() << "Send message to LLM provider...";
            // Получаем ответ от сервера
            nlohmann::json response = provider->call_answer(chats[tmpID].data);
            qDebug() << "Message was пришло..." << response.dump(2);

            if (!response.is_null()) {
                std::string llmContent = provider->call_and_parse(chats[tmpID].data);

                chats[tmpID].data["messages"].push_back({
                    {"role", "assistant"},
                    {"content", llmContent}
                });
            }
        } else {
            chats[tmpID].data["messages"].push_back({
                {"role", "assistant"},
                {"content", tmpAnswer.toStdString()}
            });
        }

    } catch (std::runtime_error &e) {
        qDebug() << e.what();
    } catch (nlohmann::json_abi_v3_12_0::detail::type_error &e) {
        qDebug() << "Ошибка" << e.what();
    }
    // Обновляем текст
    chats[tmpID].isGenerating = false;
    if (tmpID == curChatID) {
        ui->sendButton->setEnabled(true);
        ui->chatBox->setHtml(genHTML(tmpID));
    }
}


// Переключение видимости списка чатов по нажатию
void ChatixMainWindow::on_hideChatListButton_clicked() {
    bool currentlyVisible = ui->chatList->isVisible();

    // Переключаем видимость
    ui->chatList->setVisible(!currentlyVisible);
    ui->chatListLabel->setVisible(!currentlyVisible);
    QIcon icon = (currentlyVisible) ? QIcon::fromTheme(QIcon::ThemeIcon::GoNext) : QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious);
    ui->hideChatListButton->setIcon(icon);

    // Обновляем флаг
    chatListHidedByUser = currentlyVisible;
}

// Обработка изменения размера окна, автоматически скрывает список чатов
void ChatixMainWindow::resizeEvent(QResizeEvent *event) {
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

// Создание чата и автоматическое переключение
void ChatixMainWindow::on_newChatButton_clicked() {
    addChatByDate("Чат");
    switchToChat(chats.size() - 1);
    saveChatHistoryAndSettings();
}

// Переключение на чат, item - элемент списка
void ChatixMainWindow::on_chatList_itemClicked(QListWidgetItem *item) {
    int index = ui->chatList->row(item);
    switchToChat(index);
}

// Переключение на чат по его индексу
// index - индекс этого чата в chats[]
void ChatixMainWindow::switchToChat(std::size_t index) {
    if (index >= 0 && index < static_cast<int>(chats.size())) {
        qDebug() << "Состояние чата " << (chats[index].isGenerating ? "генерирует" : "простаивает");
        ui->sendButton->setEnabled((!chats[index].isGenerating));
        ui->chatNameLabel->setText(ui->chatList->item(index)->text());
        curChatID = index;
        ui->chatBox->setHtml(genHTML(index));
    }
}

// Открытие настроек
void ChatixMainWindow::on_settingsButton_clicked() {
    // Создаём экземпляр окна и передаём данные
    settingsWindow *settings = new settingsWindow();
    settings->param = current_settings;

    // Вызываем его и если внутри было подтверждено -> обрабатываем
    if (settings->showDialog() == QDialog::Accepted) {
        qDebug() << "before:\nconfig: lms model" << current_settings.lmsterModelName;
        qDebug() << "config: ollama model" << current_settings.ollamaModelName;
        qDebug() << "config: useraname:" << current_settings.userName << "\nold values end";

        // Применяем данные
        current_settings.provider = settings->param.provider;
        current_settings.lmsterModelName = settings->param.lmsterModelName;
        current_settings.ollamaModelName = settings->param.ollamaModelName;
        if (settings->param.userName != "" && current_settings.userName != settings->param.userName) {
            current_settings.userName = settings->param.userName;
        }

        // Обрабатываем изменение провайдера
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
        case settingsData::lmprovider::NONE:
            qDebug() << "Now NONE provider(";
            break;
        }

        // Костыль: обновляем в json текущего чата имя пользователя
        if (!chats.empty()) chats[curChatID].data["messages"][0]["content"] = "Ты полезный ассистент" + (!current_settings.userName.toUtf8().toStdString().empty() ? ", а пользователя зовут: " + current_settings.userName.toUtf8().toStdString() : "");

        qDebug() << "config: lms model" << current_settings.lmsterModelName;
        qDebug() << "config: ollama model" << current_settings.ollamaModelName;
        qDebug() << "config: useraname:" << current_settings.userName;
    }
}

// Сохранение пользовательских данных в папку данных приложений пользователя
// Автоматически создаёт путь
// Возвращает результат выполнения
bool ChatixMainWindow::saveChatHistoryAndSettings() {
    // Определяем путь к файлу
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(configDir);
    if (!dir.exists()) dir.mkpath(".");
    QString filePath = configDir + "/chatix_data.json";

    nlohmann::json output;
    // Сохраняем настройки
    output["settings"]["provider"] = current_settings.provider;
    output["settings"]["lmsIp"] = current_settings.lmsIp.toStdString();
    output["settings"]["lmsterModelName"] = current_settings.lmsterModelName.toStdString();
    output["settings"]["ollamaIp"] = current_settings.ollamaIp.toStdString();
    output["settings"]["ollamaModelName"] = current_settings.ollamaModelName.toStdString();
    output["settings"]["userName"] = current_settings.userName.toStdString();

    // Сохраняем чаты
    nlohmann::json chatsArray = nlohmann::json::array();
    for (const auto &chat : chats) {
        // Отсеиваем пустые чаты
        if (chat.data["messages"].size() > 1) {
            nlohmann::json chatObj;
            chatObj["title"] = chat.title.toStdString();
            chatObj["data"] = chat.data;   // data уже nlohmann::json
            chatsArray.push_back(chatObj);
        }
    }
    output["chats"] = chatsArray;

    // Записываем в файл
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Не удалось открыть файл для записи:" << filePath;
        return false;
    }
    std::string jsonStr = output.dump(2);
    file.write(jsonStr.c_str());
    file.close();
    qDebug() << "Сохранено в" << filePath;
    return true;
}

// Загрузка параметров и чатов
// Возвращает результат загрузки
bool ChatixMainWindow::loadChatHistoryAndSettings() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString filePath = configDir + "/chatix_data.json";
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qDebug() << "Файл сохранений не найден, создаём новый";
        return false;
    }

    QByteArray data = file.readAll();
    file.close();
    nlohmann::json loaded;
    try {
        loaded = nlohmann::json::parse(data.toStdString());
    } catch (const std::exception &e) {
        qDebug() << "Ошибка парсинга JSON:" << e.what();
        return false;
    }

    // Загружаем настройки
    if (loaded.contains("settings")) {
        auto &s = loaded["settings"];
        current_settings.provider = static_cast<settingsData::lmprovider>(s.value("provider", 2));
        current_settings.lmsIp = QString::fromStdString(s.value("lmsIp", "http://localhost:1234"));
        current_settings.lmsterModelName = QString::fromStdString(s.value("lmsterModelName", ""));
        current_settings.ollamaIp = QString::fromStdString(s.value("ollamaIp", "http://localhost:11434"));
        current_settings.ollamaModelName = QString::fromStdString(s.value("ollamaModelName", ""));
        current_settings.userName = QString::fromStdString(s.value("userName", ""));
    }

    // Очищаем текущие чаты и список
    chats.clear();
    ui->chatList->clear();

    // Загружаем чаты
    if (loaded.contains("chats") && loaded["chats"].is_array()) {
        for (auto &chatJson : loaded["chats"]) {
            QString title = QString::fromStdString(chatJson.value("title", "Безымянный"));
            nlohmann::json data = chatJson.value("data", nlohmann::json::object());
            chatElement newChat(data, title);
            chats.push_back(newChat);
            ui->chatList->addItem(title);
        }
    }

    // Обновляем провайдера в соответствии с загруженными настройками
    try {
        switch (current_settings.provider) {
        case settingsData::OLLAMA:
            provider = std::make_unique<lmc::OllamaClient>(current_settings.ollamaIp.toStdString());
            break;
        case settingsData::LMSTER:
            provider = std::make_unique<lmc::LMStudioClient>(current_settings.lmsIp.toStdString());
            break;
        default:
            qDebug() << "Провайдер не установлен";
            break;
        }
    } catch (const std::exception &e) {
        qDebug() << "Ошибка инициализации провайдера:" << e.what();
        return false;
    }

    return true;
}

// Закрытие окна с сохранением
void ChatixMainWindow::closeEvent(QCloseEvent *event) {
    if (saveChatHistoryAndSettings()) {
        qDebug() << "Chats saved!";
    } else {
        qDebug() << "Chats not saved(!";
    }
}

// Отправка сообщения к LMS при нажатии на Enter
bool ChatixMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->questionEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // Enter без Ctrl -> отправка
        if ((keyEvent->key() == Qt::Key_Return ||
             keyEvent->key() == Qt::Key_Enter) &&
            !(keyEvent->modifiers() & Qt::ControlModifier))
        {
            on_sendButton_clicked();
            return true;
        }

        // Ctrl+Enter -> перенос строки
        if ((keyEvent->key() == Qt::Key_Return ||
             keyEvent->key() == Qt::Key_Enter) &&
            (keyEvent->modifiers() & Qt::ControlModifier))
        {
            ui->questionEdit->insertPlainText("\n");
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// Сохраняем всё по нажатию в меню
void ChatixMainWindow::on_saveAction_triggered() {
    bool saveState = saveChatHistoryAndSettings();
    while (!saveState) {
        auto result = QMessageBox::warning(this, "Ошибка", "Ошибка сохранения!\nПопробовать ещё раз?",
                              QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes) {
            saveState = saveChatHistoryAndSettings();
        } else {
            break;
        }
    }
}

// Очищаем историю и удаляем настройки
void ChatixMainWindow::on_clearHistoryAction_triggered() {
    // Получаем путь к файлу
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString filePath = configDir + "/chatix_data.json";

    QFile file(filePath);

    // Проверяем существование файла
    if (!file.exists()) {
        qDebug() << "Файл не существует:" << filePath;
        return;
    }

    // Удаляем файл
    if (!file.remove()) {
        qDebug() << "Не удалось удалить файл:" << filePath;
        return;
    }

    qDebug() << "Файл удалён:" << filePath;
    // Очищаем чаты и добавляем пустой чат
    chats.clear();
    ui->chatList->clear();
    addChatByDate("Чат");
    switchToChat(0);
}


// По нажатию кнопки закрыть в меню - закрываем
void ChatixMainWindow::on_closeAction_triggered() {
    if (saveChatHistoryAndSettings()) {
        qDebug() << "Chats saved!";
    } else {
        qDebug() << "Chats not saved(!";
    }
    close();
}

