#pragma once
#include <string>
#include <stdexcept>
#include "json.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>

namespace lmc {

/**
     * @brief Базовый абстрактный клиент для LLM серверов
     *
     * Требует реализации:
     *  - call_answer()  → отправка запроса генерации
     *  - get_models()   → получение списка моделей
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
         * @param url полный URL (endpoint)
         * @return JSON ответ сервера
         */
    nlohmann::json get_json(const std::string& url) {
        QNetworkRequest request(QString::fromStdString(url));
        QNetworkReply* reply = manager->get(request);

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
        return post_json(server_url, context);
    }

    /**
         * @brief Получение моделей
         * endpoint: /models
         * ожидаемый ответ:
         * { "data": [ { "id": "model_name" } ] }
         */
    nlohmann::json get_models() override {
        auto result = get_json(server_url + "/models");

        nlohmann::json models = nlohmann::json::array();

        for (auto& m : result["data"]) {
            models.push_back(m["id"]); // вытаскиваем имя модели
        }

        return models;
    }
};

// ================= Ollama =================

// llmconnector.hpp (фрагмент OllamaClient)
class OllamaClient : public LLMClient {
public:
    using LLMClient::LLMClient;

    nlohmann::json call_answer(nlohmann::json& context) override {
        return post_json(server_url + "/api/chat", context);
    }

    nlohmann::json get_models() override {
        auto result = get_json(server_url + "/api/tags");

        nlohmann::json models = nlohmann::json::array();
        for (auto& m : result["models"]) {
            models.push_back(m["name"]);
        }
        return models;
    }
};

}