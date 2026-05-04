#ifndef LMMANAGER_HPP
#define LMMANAGER_HPP

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <QStringList>

namespace lmManagers {

    class lmManager : public QObject {
        Q_OBJECT

    protected:
        QProcess process;

        bool isCommandAvailable(const QString& cmd) {
            return !QStandardPaths::findExecutable(cmd).isEmpty();
        }

        bool isRunning() const {
            return process.state() != QProcess::NotRunning;
        }

    public:
        explicit lmManager(QObject* parent = nullptr) : QObject(parent) {}
        virtual ~lmManager() = default;

        virtual bool checkServerInstallation() = 0;
        virtual bool startServer() = 0;
        virtual bool stopServer() = 0;
    };

    //
    // Ollama
    //
    class ollamaManager : public lmManager {
    public:
        using lmManager::lmManager;

        bool checkServerInstallation() override {
            if (!isCommandAvailable("ollama")) {
                qDebug() << "Ollama not found in PATH";
                return false;
            }
            return true;
        }

        bool startServer() override {
            if (isRunning()) {
                qDebug() << "Ollama already running";
                return true;
            }

            process.start("ollama", {"serve"});

            if (!process.waitForStarted()) {
                qDebug() << "Failed to start Ollama";
                return false;
            }

            qDebug() << "Ollama started";
            return true;
        }

        bool stopServer() override {
            if (!isRunning()) {
                qDebug() << "Ollama is not running";
                return true;
            }

            process.terminate();

            if (!process.waitForFinished(3000)) {
                qDebug() << "Force killing Ollama...";
                process.kill();
                process.waitForFinished();
            }

            qDebug() << "Ollama stopped";
            return true;
        }
    };

    //
    // 🔹 LM Studio (LMS)
    //
    class lmstudioManager : public lmManager {
    public:
        using lmManager::lmManager;

        bool checkServerInstallation() override {
    #ifdef _WIN32
            return isCommandAvailable("lmstudio.exe");
    #else
            return isCommandAvailable("lmstudio");
    #endif
        }

        bool startServer() override {
            if (isRunning()) {
                qDebug() << "LM Studio already running";
                return true;
            }

    #ifdef _WIN32
            QString program = "lmstudio.exe";
    #else
            QString program = "lmstudio";
    #endif

            process.start(program);

            if (!process.waitForStarted()) {
                qDebug() << "Failed to start LM Studio";
                return false;
            }

            qDebug() << "LM Studio started";
            return true;
        }

        bool stopServer() override {
            if (!isRunning()) {
                qDebug() << "LM Studio is not running";
                return true;
            }

            process.terminate();

            if (!process.waitForFinished(3000)) {
                qDebug() << "Force killing LM Studio...";
                process.kill();
                process.waitForFinished();
            }

            qDebug() << "LM Studio stopped";
            return true;
        }
    };
}
#endif // LMMANAGER_HPP