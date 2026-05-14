#ifndef CHECKUSERNAME_H
#define CHECKUSERNAME_H

#include <QString>
#include <qregularexpression.h>

namespace nameChecker {
    bool nameIsGood(const QString &name) {
        static const QRegularExpression regex("^[A-Za-z0-9]+$");
        return regex.match(name).hasMatch();
    }
};

#endif // CHECKUSERNAME_H
