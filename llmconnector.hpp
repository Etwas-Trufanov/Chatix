#pragma once
#include <string>
#include <stdexcept>
#include "json.hpp"
#include "ollama.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>

namespace lmc {
    class lmster : public QObject {
        private:
            QNetworkAccessManager* manager;
            std::string server_url;
        public:
            lmster(const std::string &ip) : server_url(ip), manager(new QNetworkAccessManager()) {}

            nlohmann::json call_answer(nlohmann::json &context) {
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

            ~lmster() override {
                delete manager;
            }
    };
}
