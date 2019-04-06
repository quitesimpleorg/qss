#ifndef COMMAND_H
#define COMMAND_H
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThreadStorage>
#include <QVariant>
class Command
{
protected:
    bool fileExistsInDatabase(QSqlDatabase &db, QString path, qint64 mtime);
    QByteArray readFile(QString path) const;
    QString dbConnectionString;
    QThreadStorage<QSqlDatabase> dbStore;
public:
    Command(QString dbConnectionString)
    {
        this->dbConnectionString = dbConnectionString;
    }

    QSqlDatabase dbConnection();
    virtual int handle(QStringList arguments) = 0;
    virtual ~Command() { };
};

#endif // COMMAND_H
