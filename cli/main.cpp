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
void printUsage(QString argv0)
{
    qInfo() << "Usage: " << argv0 << "command";
}

Command *commandFromName(QString name, QString connectionstring)
{
    if(name == "add")
    {
        return new CommandAdd(connectionstring);
    }
    if(name == "delete")
    {
        return new CommandDelete(connectionstring);
    }
    if(name == "update")
    {

    }
    if(name == "search")
    {

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
    Command *cmd = commandFromName(commandName, QProcessEnvironment::systemEnvironment().value("QSS_PATH"));
    if(cmd != nullptr)
    {
        try
        {
            return cmd->handle(args);
        }
        catch(const QSSGeneralException &e)
        {
            Utils::error() << "Exception caught, message: " << e.message << endl;
        }
    }
    else
    {
        Utils::error() << "Unknown command " << commandName << endl;
    }
    return 1;
}
