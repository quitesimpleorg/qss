#include <QSqlError>
#include <QDateTime>
#include <QtConcurrentMap>
#include <functional>
#include "filesaver.h"
#include "processor.h"
#include "pdfprocessor.h"
#include "commandadd.h"
#include "defaulttextprocessor.h"
#include "tagstripperprocessor.h"
#include "nothingprocessor.h"
#include "odtprocessor.h"
#include "odsprocessor.h"
#include "utils.h"
#include "logger.h"
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


FileSaver::FileSaver(SqliteDbService &dbService)
{
    this->dbService = &dbService;
}

SaveFileResult FileSaver::addFile(QString path)
{
    QFileInfo info(path);
    QString absPath = info.absoluteFilePath();
    auto mtime = info.lastModified().toSecsSinceEpoch();
    if(this->dbService->fileExistsInDatabase(absPath, mtime))
    {
        return SKIPPED;
    }
    return saveFile(info);
}

SaveFileResult FileSaver::updateFile(QString path)
{
    QFileInfo info(path);
    return saveFile(info);
}

bool FileSaver::addFiles(const QVector<QString> paths, bool keepGoing, bool verbose)
{
    return processFiles(paths, std::bind(&FileSaver::addFile, this, std::placeholders::_1), keepGoing, verbose);
}

bool FileSaver::updateFiles(const QVector<QString> paths, bool keepGoing, bool verbose)
{
    return processFiles(paths, std::bind(&FileSaver::updateFile, this, std::placeholders::_1), keepGoing, verbose);
}

bool FileSaver::processFiles(const QVector<QString> paths, std::function<SaveFileResult(QString path)> saverFunc, bool keepGoing, bool verbose)
{
    std::atomic<bool> terminate { false };
    std::atomic<bool> hasError { false };
    QtConcurrent::blockingMap(paths, [&](const QString &path) {
        if(terminate.load())
        {
            return;
        }
        if(verbose)
        {
            Logger::info() << "Processing " << path << endl;
        }
        SaveFileResult result = saverFunc(path);
        if(result == DBFAIL || result == PROCESSFAIL)
        {
            hasError = true;
            Logger::error() << "Failed to process " << path << endl;
            if(!keepGoing)
            {
                terminate = true;

            }
        }
        if(verbose)
        {
            if(result == SKIPPED)
            {
                Logger::info() << "Skipped" << path << "as it already exists in the database" << endl;
            }
            else if(result == OK)
            {
                Logger::info() << "Added" << path << endl;
            }
        }
     });
    return ! hasError.load();
}


SaveFileResult FileSaver::saveFile(const QFileInfo &fileInfo)
{
    Processor *processor = processors.value(fileInfo.suffix(), nothingProcessor);
    QVector<PageData> pageData;
    QString absPath = fileInfo.absoluteFilePath();

    if(fileInfo.isFile())
    {
        try
        {
            if(processor->PREFERED_DATA_SOURCE == FILEPATH)
            {
                pageData = processor->process(absPath);
            }
            else
            {
                pageData = processor->process(Utils::readFile(absPath));
            }
        }
        catch(QSSGeneralException &e)
        {
            Logger::error() << "Error while processing" << absPath << ":" << e.message;
            return PROCESSFAIL;
        }
    }


    //Could happen if a file corrupted for example
    if(pageData.isEmpty() && processor != nothingProcessor)
    {
        Logger::error() << "Could not get any content for " << absPath << endl;
    }

    return this->dbService->saveFile(fileInfo, pageData);

}
