#ifndef UTILS_H
#define UTILS_H
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QTextStream>
#include <QDebug>
#include "qssgeneralexception.h"

class Utils
{
public:
    Utils();
    static QByteArray readFile(QString path);

};

#endif // UTILS_H
