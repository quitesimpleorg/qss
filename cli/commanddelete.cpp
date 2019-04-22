#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QSqlError>
#include "commanddelete.h"
#include "logger.h"

int CommandDelete::removeNonExistent(bool verbose, bool dryRun, QString pattern)
{
    int offset = 0;
    int limit = 1000;
    QVector<FileData> files;
    int processedRows = this->dbService->getFiles(files, pattern, 0, limit);
    while(processedRows > 0 )
    {
        for(FileData &file : files)
        {
            if(!dryRun)
            {
                QFileInfo fileInfo(file.absPath);
                if(!fileInfo.exists())
                {
                    if(this->dbService->deleteFile(file.absPath))
                    {
                        if(verbose)
                        {
                            Logger::info() << "Deleted" << file.absPath << endl;
                        }
                    }
                    else
                    {
                        Logger::error()<< "Failed to delete:" << file.absPath << ", exiting." << endl
                                         ;
                        return 1;
                    }
                }
            }
            else
            {
                Logger::info() << "Would delete " << file.absPath << endl;
            }

        }
        offset += limit;
        processedRows = this->dbService->getFiles(files, pattern, 0, limit);
    }
    return 0;
 }

int CommandDelete::removePaths(const QStringList &paths, bool verbose, bool dryRun)
{
    int result = 0;
    for(const QString &file : paths)
    {
        QFileInfo fileInfo(file);
        QString absPath = fileInfo.absoluteFilePath();
        if(this->dbService->fileExistsInDatabase(absPath))
        {
            if(!dryRun)
            {
                if(this->dbService->deleteFile(absPath))
                {
                    if(verbose)
                    {
                        Logger::info() << "Deleted" << absPath << endl;
                    }
                }
                else
                {
                    Logger::error()<< "Failed to delete:" << absPath << endl;
                    result = 1;
                }
            }

        }
        else
        {
            Logger::error() << "No such file in database:" << absPath << endl;
            result = 1;
        }
    }
    return result;
}

int CommandDelete::handle(QStringList arguments)
{
    QCommandLineParser parser;
    parser.addOptions({
                          { { "v", "verbose" }, "Print path of the files while deleting them" },
                          { { "n", "dry-run"}, "Only print which files would be deleted from the database, don't delete them"},
                          { "pattern", "Only delete files from index matching the pattern, e. g. */.git/*. Only applies to --deleted or standalone.", "pattern" },
                          { "deleted", "Delete all files from the index that don't exist anymore" }
                      });

    parser.addHelpOption();
    parser.addPositionalArgument("delete", "Delete paths from the index", "delete [paths...]");

    parser.process(arguments);
    bool verbose = parser.isSet("verbose");
    bool dryRun = parser.isSet("dry-run");
    QString pattern = parser.value("pattern");
    if(parser.isSet("deleted"))
    {

        int result = removeNonExistent(verbose, dryRun, pattern);
        if(result != 0)
        {
            return result;
        }
    }

    QStringList files = parser.positionalArguments();
    if(files.length() > 0)
    {
        int result = removePaths(files, verbose, dryRun);
        if(result != 0)
        {
            return result;
        }
    }



    return 0;

}


