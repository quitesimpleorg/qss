#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QSqlError>
#include "commanddelete.h"

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
    bool removeNonExistant = parser.isSet("deleted");
    bool verbose = parser.isSet("verbose");
    bool dryRun = parser.isSet("dry-run");
    QString pattern = parser.value("pattern");



    //TODO: port this to QRegularExpression once >5.12 gets more widespread because of this bug
    //https://bugreports.qt.io/browse/QTBUG-72539?focusedCommentId=439053&page=com.atlassian.jira.plugin.system.issuetabpanels%3Acomment-tabpanel

   // QRegularExpression regexPattern = QRegularExpression::wildcardToRegularExpression(pattern);

    bool usePattern = ! pattern.isEmpty();
    QRegExp regexPattern(pattern);
    regexPattern.setPatternSyntax(QRegExp::PatternSyntax::WildcardUnix);



    QSqlDatabase db = dbConnection();


    if(removeNonExistant)
    {
        //TODO: try to translate pattern to SQL WHERE statement
        QSqlQuery pathsQuery("SELECT path FROM file", db);
        if(!pathsQuery.exec())
        {
            qDebug() << "Failed to query current paths";
            return 1;
        }

        while(pathsQuery.next())
        {
            QString path = pathsQuery.value(0).toString();
            bool removeFile = true;
            if(usePattern)
            {
                removeFile = regexPattern.exactMatch(path);
            }
            if(removeFile)
            {
                QFile file(path);
                if(!file.exists())
                {
                    if(!dryRun)
                    {
                        QSqlQuery query("DELETE FROM file WHERE path = ?", db);
                        query.addBindValue(path);
                        if(!query.exec())
                        {
                            qDebug() << "Failed to delete " << path << query.lastError();
                            return 1;
                        }
                        if(verbose)
                        {
                            qInfo() << "Deleted " << path;
                        }
                    }
                    else
                    {
                        qInfo() << "Would delete " << path;
                    }

                }
            }
        }
    }

    QStringList files = parser.positionalArguments();
    for(QString &file : files)
    {
        QFileInfo fileInfo(file);
        QString absPath = fileInfo.absoluteFilePath();
        if(fileExistsInDatabase(db, absPath))
        {
            if(!dryRun)
            {
                QSqlQuery deletionQuery("DELETE FROM file WHERE path = ?", db);
                deletionQuery.addBindValue(absPath);
                if(deletionQuery.exec())
                {
                    if(verbose)
                    {
                        qInfo() << "Deleted" << absPath;
                    }
                }
                else
                {
                    qDebug() << "Failed to delete:" << absPath << deletionQuery.lastError();
                }
            }

        }
        else
        {
            qInfo() << "No such file in database:" << absPath;
        }

    }
    return 0;

}


