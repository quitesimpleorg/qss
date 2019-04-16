#ifndef LOGGER_H
#define LOGGER_H
#include <QFile>
#include <QDebug>
#include <QMutex>
class Logger
{
private:
    QDebug *out;
    QMutex mutex;
    QFile *file;
public:
    Logger(QFile *file);
    Logger(Logger &&o);

    template<class T>
    Logger &operator <<(const T &val)
    {
        QMutexLocker locker(&this->mutex);
        *out << val;
        file->flush();
        return *this;
    }

    template<class T>
    Logger &operator <<(const T *val)
    {
        QMutexLocker locker(&this->mutex);
        *out << val;
        file->flush();
        return *this;
    }

    ~Logger()
    {
        delete out;
    }



    static Logger& error();

    static Logger& info();
};

#endif // LOGGER_H
