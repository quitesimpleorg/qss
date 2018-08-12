#include "searchworker.h"

#include <QDebug>
SearchWorker::SearchWorker()
{

}
SearchWorker::SearchWorker(const QString &dbpath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbpath);
    if(!db.open())
    {
        qDebug() << "failed to open database";
    }
    queryContent = new QSqlQuery(db);
    queryFile = new QSqlQuery(db);
    queryFile->prepare("SELECT path, mtime FROM file WHERE path LIKE ? ORDER BY mtime DESC");

    queryContent->prepare("SELECT file.path, content.page, file.mtime FROM file INNER JOIN content ON file.id = content.fileid INNER JOIN content_fts ON content.id = content_fts.ROWID WHERE content_fts.content MATCH ? ORDER By file.mtime DESC, content.page ASC");
}

void SearchWorker::searchForFile(const QString &query)
{
    QVector<SearchResult> results;
    queryFile->addBindValue("%" + query + "%");
    queryFile->exec();
    while(queryFile->next())
    {
        SearchResult result;
        result.page = 0;
        result.path = queryFile->value(0).toString();
        result.mtime = queryFile->value(1).toUInt();

        results.append(result);
    }
    emit searchResultsReady(results);
}
void SearchWorker::searchForContent(const QString &query)
{
    QVector<SearchResult> results;
    queryContent->addBindValue(query);
    queryContent->exec();
    while(queryContent->next())
    {
        SearchResult result;

        result.path = queryContent->value(0).toString();
        result.page = queryContent->value(1).toUInt();
        result.mtime = queryContent->value(2).toUInt();
        results.append(result);
    }
    emit searchResultsReady(results);
}

