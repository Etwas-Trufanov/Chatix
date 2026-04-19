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
    class llm {
        public:
        llm() = default;
        virtual nlohmann::json call_answer(nlohmann::json &context) = 0;
        virtual ~llm() = default;
    };

    class llama_cpp : public llm {
        public:
            nlohmann::json call_answer(nlohmann::json &context) override {
                return nlohmann::json::parse(R"("pukpuk")");
            }
    };
    class remote_llm : public llm {
        private:
            QNetworkAccessManager* manager;
            std::string server_url;
        public:
            remote_llm(const std::string &ip) : server_url(ip), manager(new QNetworkAccessManager()) {}

            nlohmann::json call_answer(nlohmann::json &context) override {
                QNetworkRequest request(QString::fromStdString(server_url));
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

                QByteArray body = QByteArray::fromStdString(context.dump());

                QNetworkReply* reply = manager->post(request, body);

                QEventLoop loop;
                QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
                loop.exec();

                if (reply->error() != QNetworkReply::NoError) {
                    std::string errorMsg = reply->errorString().toStdString();
                    reply->deleteLater();
                    throw std::runtime_error("Network Error: " + errorMsg);
                }

                QByteArray responseData = reply->readAll();
                nlohmann::json result = nlohmann::json::parse(responseData.toStdString());
                reply->deleteLater();

                return result;
            }

            ~remote_llm() override {
                delete manager;
            }
    };
}
