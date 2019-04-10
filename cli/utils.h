#ifndef UTILS_H
#define UTILS_H
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QTextStream>
#include "qssgeneralexception.h"

class Utils
{
public:
    Utils();
    static QByteArray readFile(QString path);
    static QDebug &info();
    static QDebug &error();
};

#endif // UTILS_H
