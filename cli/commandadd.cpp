#include <QFileInfo>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QMap>
#include <QTextStream>
#include <QException>
#include <QCommandLineParser>
#include <QMutex>
#include <QMutexLocker>
#include <QtConcurrent/QtConcurrentMap>
#include "commandadd.h"
#include "logger.h"





int CommandAdd::handle(QStringList arguments)
{
    QCommandLineParser parser;
    parser.addOptions({
                          { { "c", "continue" }, "Continue adding files, don't exit on first error"},
                          { { "a", "all" }, "On error, no files should be added, even already processed ones" },
                          { { "v", "verbose" }, "Print skipped and added files" },
                          { { "t", "threads" }, "Number of threads to use.", "threads" }
                      });

    parser.addHelpOption();
    parser.addPositionalArgument("add", "Add paths to the index", "add [paths...]");

    parser.process(arguments);
    bool keepGoing = parser.isSet("continue");
    bool verbose = parser.isSet("verbose");
    if(parser.isSet("all"))
    {
        throw QSSGeneralException("To be implemented");
    }
    if(parser.isSet("threads"))
    {
        QString threadsCount = parser.value("threads");
        QThreadPool::globalInstance()->setMaxThreadCount(threadsCount.toInt());
    }

    QStringList files = parser.positionalArguments();

    if(files.length() == 0)
    {
        QTextStream stream(stdin);

        while(!stream.atEnd())
        {
            QString path = stream.readLine();
            files.append(path);
         }
    }

    FileSaver saver(*this->dbService);
    if(!saver.addFiles(files.toVector(), keepGoing, verbose))
    {
        Logger::error() << "Errors occured while trying to add files to the database" << endl;
        return 1;
    }

    return 0;
}

