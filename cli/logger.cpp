#include <QMutexLocker>
#include "logger.h"

Logger::Logger(QFile *file)
{
    out = new QDebug(file);
    out->noquote();
    this->file = file;
}

Logger::Logger(Logger &&o)
{
    this->out = o.out;
    o.out = nullptr;
}

Logger &Logger::error()
{
    static Logger result  = []{
        QFile *file = new QFile();
        file->open(stderr, QIODevice::WriteOnly);
        return Logger(file);
    }();
    return result;
}

Logger &Logger::info()
{
    static Logger result  = []{
        QFile *file = new QFile();
        file->open(stdout, QIODevice::WriteOnly);
        return Logger(file);
    }();
    return result;
}




