#pragma once
#include <curl/curl.h>
#include <string>
#include <stdexcept>
#include "json.hpp"

// Функция для обработки ответа сервера
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

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
            CURL* curl;
            CURLcode res;
            std::string readBuffer;
            std::string server_url;
        public:
            remote_llm(const std::string &ip) : server_url(ip) {
                curl_global_init(CURL_GLOBAL_ALL);
                curl = curl_easy_init();
                if (!curl) {
                    throw std::runtime_error("Init CURL error");
                }
            }
            nlohmann::json call_answer(nlohmann::json &context) override {
                readBuffer.clear();
                std::string json_str = context.dump();

                struct curl_slist* headers = nullptr;
                headers = curl_slist_append(headers, "Content-Type: application/json");

                curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

                curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
                curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
                curl_easy_setopt(curl, CURLOPT_STDERR, stderr);
                curl_easy_setopt(curl, CURLOPT_PROXY, "");

                CURLcode res = curl_easy_perform(curl);
                curl_slist_free_all(headers);

                if (res != CURLE_OK) {
                    throw std::runtime_error(std::string("CURL Error: ") + curl_easy_strerror(res));
                }

                return nlohmann::json::parse(readBuffer);
            }
            ~remote_llm() override {
                if (curl) curl_easy_cleanup(curl);
                curl_global_cleanup();
            }
    };
}

// LLMCONNECTOR_HPP
