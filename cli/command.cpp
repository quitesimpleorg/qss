#include <QFile>
#include <QThread>
#include <QDebug>
#include "command.h"
#include "qssgeneralexception.h"

bool Command::fileExistsInDatabase(QSqlDatabase &db, QString path, qint64 mtime)
{
    auto query = QSqlQuery("SELECT 1 FROM file WHERE path = ? and mtime = ?", db);
    query.addBindValue(path);
    query.addBindValue(mtime);
    if(!query.exec())
    {        throw QSSGeneralException("Error while trying to query for file existance");
    }
    if(!query.next())
    {
        return false;
    }
    return query.value(0).toBool();
}

QSqlDatabase Command::dbConnection()
{
    if(dbStore.hasLocalData())
    {
        return dbStore.localData();
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "QSS" + QString::number((quint64)QThread::currentThread(), 16));
    db.setDatabaseName(this->dbConnectionString);
    if(!db.open())
    {
        qDebug() << "Failed to open the database: " << this->dbConnectionString;
    }
    dbStore.setLocalData(db);
    return db;
}

