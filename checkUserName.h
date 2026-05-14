#ifndef CHECKUSERNAME_H
#define CHECKUSERNAME_H

#include <QString>
#include <qregularexpression.h>

class checks {
    public:
        static bool isValidUsername(const QString& str) {
            if (str.isEmpty())
                return false;

            for (QChar ch : str)
            {
                if (!ch.isLetterOrNumber())
                    return false;
            }

            return true;
        }
};

#endif // CHECKUSERNAME_H
