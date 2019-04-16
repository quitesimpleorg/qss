#ifndef COMMAND_H
#define COMMAND_H
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThreadStorage>
#include <QVariant>
#include "utils.h"
#include "sqlitedbservice.h"
class Command
{
protected:

    SqliteDbService *dbService;
    QString dbConnectionString;
public:
    Command(SqliteDbService &dbService)
    {
        this->dbService = &dbService;
    }

    QSqlDatabase dbConnection();
    virtual int handle(QStringList arguments) = 0;
    virtual ~Command() { };
};

#endif // COMMAND_H
