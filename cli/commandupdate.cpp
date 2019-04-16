#include <QCommandLineParser>
#include <QFileInfo>
#include <QDateTime>
#include <QThreadPool>
#include <QtConcurrentRun>
#include "commandupdate.h"
#include "logger.h"


int CommandUpdate::handle(QStringList arguments)
{
    QCommandLineParser parser;
    parser.addOptions({
                          { { "v", "verbose" }, "Print path of the files while updating them" },
                          { { "n", "dry-run"}, "Only print which files would be updated, don't actually update them"},
                          { "pattern", "Only consider to update files in the index matching the pattern, e. g. */.git/*.", "pattern" },
                          { { "d", "delete" }, "If a file does not exist anymore, delete it" },
                          { { "c", "continue" }, "Continue adding files, don't exit on first error"},
                          { { "a", "all" }, "On error, no files should be updated, even already processed ones" },
                          { { "t", "threads" }, "Number of threads to use.", "threads" }

                      });

    parser.addHelpOption();
    parser.addPositionalArgument("update", "Checks files for changes and updates them", "update");

    parser.process(arguments);
    bool keepGoing = parser.isSet("continue");
    bool verbose = parser.isSet("verbose");
    bool deleteMissing = parser.isSet("delete");
    bool dryRun = parser.isSet("dry-run");
    QString pattern = parser.value("pattern");

    if(parser.isSet("all"))
    {
        throw QSSGeneralException("To be implemented");
    }
    if(parser.isSet("threads"))
    {
        QString threadsCount = parser.value("threads");
        QThreadPool::globalInstance()->setMaxThreadCount(threadsCount.toInt());
    }
    FileSaver saver(*this->dbService);
    QVector<FileData> files;
    int offset = 0;
    int limit = 1000;
    int processedRows = dbService->getFiles(files, pattern, offset, limit);

    while(processedRows > 0)
    {
        QVector<QString> filePathsToUpdate;
        for(FileData &fileData : files)
        {
            QFileInfo fileInfo(fileData.absPath);
            if(fileInfo.exists() && fileInfo.isFile())
            {
                if(fileInfo.lastModified().toSecsSinceEpoch() != fileData.mtime)
                {
                    if(!dryRun)
                    {
                        filePathsToUpdate.append(fileData.absPath);
                    }
                    else
                    {
                        Logger::info() << "Would update" << fileData.absPath << endl;
                    }
                }

            }
            else
            {
                if(deleteMissing)
                {
                    if(!dryRun)
                    {
                        if(!this->dbService->deleteFile(fileData.absPath))
                        {
                            Logger::error() << "Error: Failed to delete" << fileData.absPath << "from databas" << endl;
                            if(!keepGoing)
                            {
                                return 1;
                            }
                        }
                        if(verbose)
                        {
                            Logger::info() << "Deleted" << fileData.absPath << endl;
                        }
                    }
                    else
                    {

                        Logger::info() << "Would delete" << fileData.absPath << endl;
                    }

                }
            }
        }


        if(!saver.updateFiles(filePathsToUpdate, keepGoing, verbose))
        {
            if(!keepGoing)
            {
                Logger::error() << "Failed to update all files selected for updating" << endl;
                return 1;
            }
        }
        offset += limit;
        processedRows = this->dbService->getFiles(files, pattern, offset, limit);
    }

    return 0;


}
