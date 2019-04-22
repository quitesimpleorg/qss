#include <QSqlQuery>
#include <QFileInfo>
#include <QDateTime>
#include <QSqlError>
#include "sqlitedbservice.h"
#include "filedata.h"
#include "logger.h"
bool SqliteDbService::fileExistsInDatabase(QString path, qint64 mtime)
{
    auto query = QSqlQuery("SELECT 1 FROM file WHERE path = ? and mtime = ?", dbFactory->forCurrentThread());
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

QVector<SearchResult> SqliteDbService::search(QString searchQuery)
{
    auto connection = dbFactory->forCurrentThread();
    SqliteSearch searcher(connection);
    return searcher.search(searchQuery);
}

bool SqliteDbService::fileExistsInDatabase(QString path)
{
    auto query = QSqlQuery(dbFactory->forCurrentThread());
    query.prepare("SELECT 1 FROM file WHERE path = ?");
    query.addBindValue(path);
    if(!query.exec())
    {        throw QSSGeneralException("Error while trying to query for file existance");
    }
    if(!query.next())
    {
        return false;
    }
    return query.value(0).toBool();
}


SqliteDbService::SqliteDbService(DatabaseFactory &dbFactory)
{
    this->dbFactory = &dbFactory;
}

bool SqliteDbService::deleteFile(QString path)
{
    QSqlQuery query(this->dbFactory->forCurrentThread());
    query.prepare("DELETE FROM file WHERE path = ?");
    query.addBindValue(path);
    bool result =  query.exec();
    if(!result)
    {
        Logger::error() << "Failed to delete file" << path << endl;
    }
    return result;

}
int SqliteDbService::getFiles(QVector<FileData> &results, QString wildCardPattern, int offset , int limit)
{

    int processedRows = 0;
    //TODO: translate/convert wildCardPattern to SQL where instead of regex
    QString sql = "SELECT path, mtime, size, filetype FROM file";

    if(limit != 0)
    {
        sql += " LIMIT " + QString::number(limit);
    }
    if(offset != 0)
    {
        sql += " OFFSET " + QString::number(offset);
    }

    auto query = QSqlQuery(dbFactory->forCurrentThread());
    query.prepare(sql);
    query.setForwardOnly(true);
    if(!query.exec())
    {
        throw QSSGeneralException("Error while trying to retrieve files from database: " + query.lastError().text());
    }

    //TODO: port this to QRegularExpression once >5.12 gets more widespread because of this bug
    //https://bugreports.qt.io/browse/QTBUG-72539?focusedCommentId=439053&page=com.atlassian.jira.plugin.system.issuetabpanels%3Acomment-tabpanel
    bool usePattern = ! wildCardPattern.isEmpty();
    QRegExp regexPattern(wildCardPattern);
    regexPattern.setPatternSyntax(QRegExp::PatternSyntax::WildcardUnix);

    while(query.next())
    {
        QString absPath = query.value(0).toString();
        if(! usePattern || regexPattern.exactMatch(absPath))
            {
                FileData current;
                current.absPath = absPath;
                current.mtime = query.value(1).toInt();
                current.size = query.value(2).toInt();
                current.filetype = query.value(3).toChar();
                results.append(current);
            }
        ++processedRows;

    }
    return processedRows;
}





SaveFileResult SqliteDbService::saveFile(QFileInfo fileInfo, QVector<PageData> &pageData)
{
    QString absPath = fileInfo.absoluteFilePath();
    auto mtime = fileInfo.lastModified().toSecsSinceEpoch();
    QChar fileType = fileInfo.isDir() ? 'd' : 'f';

    QSqlDatabase db = dbFactory->forCurrentThread();
    QSqlQuery delQuery(db);
    delQuery.prepare("DELETE FROM file WHERE path = ?");
    delQuery.addBindValue(absPath);

    QSqlQuery inserterQuery(db);
    inserterQuery.prepare("INSERT INTO file(path, mtime, size, filetype) VALUES(?, ?, ?, ?)");
    inserterQuery.addBindValue(absPath);
    inserterQuery.addBindValue(mtime);
    inserterQuery.addBindValue(fileInfo.size());
    inserterQuery.addBindValue(fileType);


    if(!db.transaction())
    {
        Logger::error() << "Failed to open transaction for " << absPath << " : " << db.lastError() << endl;
        return DBFAIL;
    }

    if(!delQuery.exec())
    {
        Logger::error() << "Failed DELETE query" <<  delQuery.lastError() << endl;
        db.rollback();
        return DBFAIL;
    }


    if(!inserterQuery.exec())
    {
        Logger::error() << "Failed INSERT query" <<  inserterQuery.lastError() << endl;
        db.rollback();
        return DBFAIL;
    }

    int lastid = inserterQuery.lastInsertId().toInt();
    for(const PageData &data : pageData)
    {
        QSqlQuery contentQuery(db);
        contentQuery.prepare("INSERT INTO content(fileid, page, content) VALUES(?, ?, ?)");
        contentQuery.addBindValue(lastid);
        contentQuery.addBindValue(data.pagenumber);
        contentQuery.addBindValue(data.content);
        if(!contentQuery.exec())
        {
            db.rollback();
            Logger::error() << "Failed content insertion " << contentQuery.lastError() << endl;
            return DBFAIL;
        }
    }

    if(!db.commit())
    {
        db.rollback();
        Logger::error() << "Failed to commit transaction for " << absPath <<  " : " << db.lastError() << endl;
        return DBFAIL;
    }
    return OK;
}
