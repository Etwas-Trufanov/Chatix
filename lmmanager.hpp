#ifndef LMMANAGER_HPP
#define LMMANAGER_HPP

#include <cstdlib>
#include <string>
#include <QDebug>

namespace lmmanager {

    int runCommand(const std::string& command) {
        qDebug() << "> " << command;
        return system(command.c_str());
    }

    // Функция для проверки доступности команды в системе
    bool isCommandAvailable(const std::string& cmd) {
        std::string checkCmd;
    #ifdef _WIN32
        checkCmd = "where " + cmd + " > nul 2>&1";
    #else
        checkCmd = "command -v " + cmd + " > /dev/null 2>&1";
    #endif
        return runCommand(checkCmd) == 0;
    }

    int checkLmsInstallation() {
        qDebug() << "Проверка установки 'lms' (CLI LM Studio)...";

        // 1. Проверяем, установлен ли lms
        if (isCommandAvailable("lms")) {
            qDebug() << "✓ 'lms' уже установлен.";
            return 0;
        }
        return 1;
    }
}
#endif // LMMANAGER_HPP
