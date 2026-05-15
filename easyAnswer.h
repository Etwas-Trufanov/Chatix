#ifndef EASYANSWER_H
#define EASYANSWER_H

#include <QString>
#include <QDateTime>

class easyAnswer {
public:
    // Возвращает ответ на вопрос, принимает ok для обработки сложного запроса
    static QString answerOnEasyQuestion(const QString &question, bool &ok) {
        if (question == "Скажи время" || question == "Сколько времени" || question == "Время") {
            ok = true;
            return QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
        }
        ok = false;
        return question;
    }
};

#endif // EASYANSWER_H
