#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDataStream>
#include <QDebug>
#include <QProcessEnvironment>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QDebug>
#include <functional>
#include <exception>
#include "encodingdetector.h"
#include "pdfprocessor.h"
#include "defaulttextprocessor.h"
#include "utils.h"
#include "command.h"
#include "commandadd.h"
#include "commanddelete.h"
#include "commandupdate.h"
#include "commandsearch.h"
#include "databasefactory.h"
#include "logger.h"
void printUsage(QString argv0)
{
    qInfo() << "Usage: " << argv0 << "command";
}

Command *commandFromName(QString name, SqliteDbService &dbService)
{
    if(name == "add")
    {
        return new CommandAdd(dbService);
    }
    if(name == "delete")
    {
        return new CommandDelete(dbService);
    }
    if(name == "update")
    {
        return new CommandUpdate(dbService);
    }
    if(name == "search")
    {
        return new CommandSearch(dbService);
    }

    return nullptr;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QStringList args = app.arguments();
    QString argv0 = args.takeFirst();
    if(args.length() < 1)
    {
        printUsage(argv0);
        exit(1);
    }

    QString commandName = args.first();
    QString connectionString = QProcessEnvironment::systemEnvironment().value("QSS_PATH");
    DatabaseFactory dbFactory(connectionString);
    SqliteDbService dbService(dbFactory);
    Command *cmd = commandFromName(commandName, dbService);
    if(cmd != nullptr)
    {
        try
        {
            return cmd->handle(args);
        }
        catch(const QSSGeneralException &e)
        {
            Logger::error() << "Exception caught, message: " << e.message << endl;
        }
    }
    else
    {
        Logger::error() << "Unknown command " << commandName << endl;
    }
    return 1;
}
