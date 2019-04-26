#ifndef FILESAVER_H
#define FILESAVER_H
#include <QSqlDatabase>
#include <QFileInfo>
#include "command.h"
#include "pagedata.h"
#include "filedata.h"
#include "sqlitedbservice.h"


class FileSaver
{
private:
    SqliteDbService *dbService;
protected:
    SaveFileResult addFile(QString path);
    SaveFileResult updateFile(QString path);
public:
    FileSaver(SqliteDbService &dbService);
    SaveFileResult saveFile(const QFileInfo &fileInfo);
    int processFiles(const QVector<QString> paths, std::function<SaveFileResult(QString path)> saverFunc, bool keepGoing, bool verbose);
    int addFiles(const QVector<QString> paths, bool keepGoing, bool verbose);
    int updateFiles(const QVector<QString> paths, bool keepGoing, bool verbose);

;
};

#endif // FILESAVER_H
