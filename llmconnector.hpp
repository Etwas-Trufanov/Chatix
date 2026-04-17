#pragma once

#include <nlohmann/json.hpp>

namespace {
    class llm {
        public:
        virtual std::string call_answer(context) = 0;
        ~llm();
    };

    class llama_cpp : llm {
        std::string call_answer()
    };
    class remote_llm : llm{

    };
}

// LLMCONNECTOR_HPP
