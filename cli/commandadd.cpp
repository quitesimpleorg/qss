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
#include "processor.h"
#include "pdfprocessor.h"
#include "commandadd.h"
#include "defaulttextprocessor.h"
#include "tagstripperprocessor.h"
#include "nothingprocessor.h"
#include "odtprocessor.h"
#include "odsprocessor.h"
#include "utils.h"
static DefaultTextProcessor *defaultTextProcessor = new DefaultTextProcessor();
static TagStripperProcessor *tagStripperProcessor = new TagStripperProcessor();
static NothingProcessor *nothingProcessor = new NothingProcessor();
static OdtProcessor *odtProcessor = new OdtProcessor();
static OdsProcessor *odsProcessor = new OdsProcessor();

static QMap<QString, Processor*> processors{
    {"pdf", new PdfProcessor() },
    {"txt", defaultTextProcessor},
    {"md", defaultTextProcessor},
    {"py", defaultTextProcessor},
    {"xml", nothingProcessor},
    {"html", tagStripperProcessor},
    {"java", defaultTextProcessor},
    {"js", defaultTextProcessor},
    {"cpp", defaultTextProcessor},
    {"c", defaultTextProcessor},
    {"sql", defaultTextProcessor},
    {"odt", odtProcessor},
    {"ods", odsProcessor}
};



AddFileResult CommandAdd::addFile(QString path)
{
    QFileInfo info(path);
    QString absPath = info.absoluteFilePath();
    auto mtime = info.lastModified().toSecsSinceEpoch();
    QChar fileType = info.isDir() ? 'd' : 'f';

    QSqlDatabase db = dbConnection();
    if(fileExistsInDatabase(db, absPath, mtime))
    {
        return SKIPPED;
    }



    Processor *processor = processors.value(info.suffix(), nothingProcessor);
    QVector<PageData> pageData;
    if(processor->PREFERED_DATA_SOURCE == FILEPATH)
    {
        pageData = processor->process(absPath);
    }
    else
    {
        pageData = processor->process(Utils::readFile(absPath));
    }

    if(pageData.isEmpty())
    {
        qDebug() << "Could not get any content for " << absPath;
    }


    //Workaround to "database is locked" error. Perhaps try WAL mode etc.
    //QMutexLocker locker(&writeMutex);
    if(!db.transaction())
    {
        qDebug() << "Failed to open transaction for " << absPath << " : " << db.lastError();
        return DBFAIL;
    }

    QSqlQuery delQuery("DELETE FROM file WHERE path = ?", db);
    delQuery.addBindValue(absPath);
    if(!delQuery.exec())
    {
        qDebug() << "Failed DELETE query" <<  delQuery.lastError();
        db.rollback();
        return DBFAIL;
    }


    QSqlQuery inserterQuery("INSERT INTO file(path, mtime, size, filetype) VALUES(?, ?, ?, ?)", db);
    inserterQuery.addBindValue(absPath);
    inserterQuery.addBindValue(mtime);
    inserterQuery.addBindValue(info.size());
    inserterQuery.addBindValue(fileType);
    if(!inserterQuery.exec())
    {
        qDebug() << "Failed INSERT query" <<  inserterQuery.lastError();
        db.rollback();
        return DBFAIL;
    }
    int lastid = inserterQuery.lastInsertId().toInt();
    for(PageData &data : pageData)
    {
        QSqlQuery contentQuery("INSERT INTO content(fileid, page, content) VALUES(?, ?, ?)", db);
        contentQuery.addBindValue(lastid);
        contentQuery.addBindValue(data.pagenumber);
        contentQuery.addBindValue(data.content);
        if(!contentQuery.exec())
        {
            db.rollback();
            qDebug() << "Failed content insertion " << contentQuery.lastError();
            return DBFAIL;
        }
    }

    if(!db.commit())
    {
        db.rollback();
        qDebug() << "Failed to commit transaction for " << absPath <<  " : " << db.lastError();
        return DBFAIL;
    }
    return OK;
}


int CommandAdd::handle(QStringList arguments)
{
    QCommandLineParser parser;
    parser.addOptions({
                          { { "c", "continue" }, "Continue adding files, don't exit on first error"},
                          { { "a", "all" }, "On error, no files should be added, even already processed ones" },
                          { { "v", "verbose" }, "Print skipped and added files" },
                          { { "n", "threads" }, "Number of threads to use.", "threads" }
                      });

    parser.addHelpOption();
    parser.addPositionalArgument("paths", "List of paths to process/add to the index", "[paths...]");

    parser.process(arguments);
    bool keepGoing = false;
    bool verbose = false;
    if(parser.isSet("continue"))
    {
        keepGoing = true;
    }
    if(parser.isSet("verbose"))
    {
        verbose = true;
    }
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

    bool terminate = false;
    QtConcurrent::blockingMap(files, [&](QString &path) {
        if(terminate)
        {
            return;
        }
        if(verbose)
        {
            qDebug() << "Processing " << path;
        }
        auto result = addFile(path);
        if(result == DBFAIL)
        {
            qDebug() << "Failed to add " << path;
            if(!keepGoing)
            {
                terminate = true;
            }
        }
        if(verbose)
        {
            if(result == SKIPPED)
            {
                qDebug() << "SKIPPED" << path << "as it already exists in the database";
            }
            else
            {
                qDebug() << "Added" << path;
            }
        }


    });

    return 0;
}
