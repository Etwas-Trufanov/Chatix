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
    nlohmann::json q = nlohmann::json::parse(R"({
    "messages": [
      {"role": "system", "content": "Ты полезный ассистент."},
      {"role": "user", "content": "Привет! Как дела?"}
    ],
    "temperature": 0.7,
    "max_tokens": -1
})");
    try {
        nlohmann::json r = model->call_answer(q);
        ui->chatBox->setText(QString::fromStdString(r.dump(2)));
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
    }
}


