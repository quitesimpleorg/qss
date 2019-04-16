#ifndef DATABASEFACTORY_H
#define DATABASEFACTORY_H
#include <QSqlDatabase>
#include <QThreadStorage>
#include "utils.h"
class DatabaseFactory
{
private:
    QString connectionString;
public:
    DatabaseFactory(QString connectionString);
    QSqlDatabase createNew();
    QSqlDatabase forCurrentThread();
};

#endif // DATABASEFACTORY_H
