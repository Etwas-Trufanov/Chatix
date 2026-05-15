#pragma once
#include <string>
#include <stdexcept>
#include "json.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <QTimer>

namespace lmc {

/**
     * @brief Базовый абстрактный клиент для LLM серверов
     *
     * Требует реализации:
     *  - call_answer()  -> отправка запроса генерации
     *  - get_models()   -> получение списка моделей
     */
class LLMClient : public QObject {
    protected:
        QNetworkAccessManager* manager; // HTTP клиент Qt
        std::string server_url;         // базовый URL сервера (например http://localhost:11434)

        /**
             * @brief POST запрос с JSON телом
             * @param url полный URL (endpoint)
             * @param body JSON тело запроса
             * @return JSON ответ сервера
             */
        nlohmann::json post_json(const std::string& url, const nlohmann::json& body) {
            QNetworkRequest request(QString::fromStdString(url));
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

            QByteArray data = QByteArray::fromStdString(body.dump());
            QNetworkReply* reply = manager->post(request, data);

            // блокируемся до получения ответа (синхронно)
            QEventLoop loop;
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();

            if (reply->error() != QNetworkReply::NoError) {
                std::string errorMsg = reply->errorString().toStdString();
                reply->deleteLater();
                throw std::runtime_error("Network Error: " + errorMsg);
            }

            QByteArray response = reply->readAll();
            reply->deleteLater();

            return nlohmann::json::parse(response.toStdString());
        }

        /**
             * @brief GET запрос
             * @param url полный URL (endpoint), timeout таймаут
             * @return JSON ответ сервера
             */
        nlohmann::json get_json(const std::string& url, uint timeout) {
            QNetworkRequest request(QString::fromStdString(url));
            QNetworkReply* reply = manager->get(request);

            QEventLoop loop;

            QTimer timer;
            bool timedOut = false;

            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

            // Лямбда фунция, она прерывает запрос
            QObject::connect(&timer, &QTimer::timeout, [&]() {
                timedOut = true;
                reply->abort();
                loop.quit();
            });

            if (timeout > 0) {
                timer.setSingleShot(true);
                timer.start(timeout * 1000);
            }

            loop.exec();

            if (timedOut) {
                reply->deleteLater();
                throw std::runtime_error("Request timeout");
            }

            if (reply->error() != QNetworkReply::NoError) {
                std::string errorMsg = reply->errorString().toStdString();
                reply->deleteLater();
                throw std::runtime_error("Network Error: " + errorMsg);
            }

            QByteArray response = reply->readAll();
            reply->deleteLater();

            return nlohmann::json::parse(response.toStdString());
        }

    public:
        /**
             * @param url базовый URL сервера (без endpoint'ов)
             */
        LLMClient(const std::string& url)
            : server_url(url), manager(new QNetworkAccessManager()) {}

        virtual ~LLMClient() {
            delete manager;
        }

        /**
             * @brief Отправить запрос генерации
             * @param context JSON в формате конкретного API (зависит от сервера)
             * @return JSON ответ сервера
             */
        virtual nlohmann::json call_answer(nlohmann::json& context) = 0;

        /**
             * @brief Получить список доступных моделей
             * @return JSON массив строк с именами моделей
             */
        virtual nlohmann::json get_models() = 0;

        /**
         * @brief Парсит ответ сервера и извлекает текст ответа ассистента
         * @param response Сырой JSON от сервера
         * @return Строка с контентом
         */
        virtual std::string parse_response_content(const nlohmann::json& response) = 0;

        /**
         * @brief Удобный метод: отправить запрос и сразу получить распарсенный текст
         */
        std::string call_and_parse(nlohmann::json& context) {
            auto response = call_answer(context);
            return parse_response_content(response);
        }
};

// ================= LM Studio / OpenAI-like =================

class LMStudioClient : public LLMClient {
    public:
        using LLMClient::LLMClient; // наследуем конструктор

        /**
             * @brief Отправка запроса (обычно OpenAI-compatible endpoint)
             * @param context JSON (messages, model, и т.д.)
             */
        nlohmann::json call_answer(nlohmann::json& context) override {
            return post_json(server_url + "/v1/chat/completions", context);
        }

        /**
             * @brief Получение моделей
             * endpoint: /v1/models
             * ожидаемый ответ:
             * { "data": [ { "id": "model_name" } ] }
             */
        nlohmann::json get_models() override {
            auto result = get_json(server_url + "/v1/models", 2);

            nlohmann::json models = nlohmann::json::array();

            for (auto& m : result["data"]) {
                models.push_back(m["id"]); // вытаскиваем имя модели
            }

            return models;
        }

        std::string parse_response_content(const nlohmann::json& response) override {
            // OpenAI-style: {"choices": [{"message": {"content": "..."}}]}
            if (response.contains("choices") && !response["choices"].empty()) {
                const auto& choice = response["choices"][0];
                if (choice.contains("message") && choice["message"].contains("content")) {
                    return choice["message"]["content"].get<std::string>();
                }
            }
            throw std::runtime_error("LMStudio: Failed to parse response - missing choices[0].message.content");
        }
};

// ================= Ollama =================

// llmconnector.hpp (фрагмент OllamaClient)
class OllamaClient : public LLMClient {
public:
    using LLMClient::LLMClient;

    nlohmann::json call_answer(nlohmann::json& context) override {
        qDebug() << server_url + "/api/chat" << "ollama url";
        return post_json(server_url + "/api/chat", context);
    }

    nlohmann::json get_models() override {
        auto result = get_json(server_url + "/api/tags", 2);

        nlohmann::json models = nlohmann::json::array();
        for (auto& m : result["models"]) {
            models.push_back(m["name"]);
        }
        return models;
    }

    std::string parse_response_content(const nlohmann::json& response) override {
        // Ollama-style: {"message": {"role": "assistant", "content": "..."}}
        if (response.contains("message") && response["message"].contains("content")) {
            return response["message"]["content"].get<std::string>();
        }
        // Fallback для старых версий / stream-режима
        if (response.contains("response")) {
            return response["response"].get<std::string>();
        }
        throw std::runtime_error("Ollama: Failed to parse response - missing message.content or response field");
    }
};

}

