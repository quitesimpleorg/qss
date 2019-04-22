#ifndef SQLITEDBSERVICE_H
#define SQLITEDBSERVICE_H
#include <QFileInfo>
#include "databasefactory.h"
#include "utils.h"
#include "pagedata.h"
#include "filedata.h"
#include "../shared/sqlitesearch.h"
enum SaveFileResult
{
    OK,
    SKIPPED,
    DBFAIL,
    PROCESSFAIL
};

class SqliteDbService
{
private:
    DatabaseFactory *dbFactory = nullptr;
public:
    SqliteDbService(DatabaseFactory &dbFactory);
    SaveFileResult saveFile(QFileInfo fileInfo, QVector<PageData> &pageData);
    int getFiles(QVector<FileData> &results, QString wildCardPattern, int offset , int limit);
    bool deleteFile(QString path);
    bool fileExistsInDatabase(QString path);
    bool fileExistsInDatabase(QString path, qint64 mtime);
    QVector<SearchResult> search(QString searchQuery);

};

#endif // SQLITEDBSERVICE_H
