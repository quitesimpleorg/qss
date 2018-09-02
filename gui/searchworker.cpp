#include "searchworker.h"

#include <QRegularExpression>
#include <QDebug>
#include <QSqlError>
SearchWorker::SearchWorker()
{

}
SearchWorker::SearchWorker(const QString &dbpath)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbpath);
    if(!db.open())
    {
        qDebug() << "failed to open database";
    }
}

QVector<SearchWorker::Command> SearchWorker::tokenize(QString expression)
{
    //TODO: merge lonewords
    QVector<Command> result;
    QRegularExpression rx("((?<filtername>(\\.|\\w)+):(?<args>\\((?<innerargs>[^\\)]+)\\)|(\\w)+)|(?<boolean>AND|OR|!)|(?<bracket>\\(|\\))|(?<loneword>\\w+))");
    QRegularExpressionMatchIterator i = rx.globalMatch(expression);
    bool wasbool = true;
    while(i.hasNext())
    {
        QRegularExpressionMatch m = i.next();
        QString boolean = m.captured("boolean");
        QString filtername = m.captured("filtername");
        QString bracket = m.captured("bracket");
        QString loneword = m.captured("loneword");
        if(boolean != "")
        {
           /* if(wasbool)
            {
                throw new std::runtime_error("Bool after Bool is invalid");
            }*/
            wasbool = true;
            result.append(Command(boolean));
        }

        if(bracket != "")
        {
           if(!wasbool)
           {
               if(bracket == "(")
               {
                   result.append(Command("AND"));
               }
           }
           result.append(Command(bracket));
        }


        if(loneword != "")
        {
            if(!wasbool)
            {
                result.append(Command("AND"));
            }
            wasbool = false;
            result.append(Command("path.contains", loneword));
        }
        if(filtername != "")
        {
            if(!wasbool)
            {
                result.append(Command("AND"));
            }
            wasbool = false;
            QString value = m.captured("innerargs");
            if(value == "")
                value = m.captured("args");
            result.append(Command(filtername, value));

        }

    }
    return result;
}

QString SearchWorker::createSql(const SearchWorker::Command &cmd)
{
    QString key = cmd.key;
    QString value = cmd.value;
    if(key == "AND" || key == "OR" || key=="(" || key==")")
    {
        return " " + key + " ";
    }
    if(key == "!")
    {
        return " NOT ";
    }
    if(key == "path.starts")
    {
        return " file.path LIKE '" + value + "%' ";
    }
    if(key == "path.ends")
    {
        return " file.path LIKE '%" + value + "' ";
    }
    if(key == "path.contains" || key == "inpath")
    {
        return " file.path LIKE '%" + value + "%' ";
    }
    if(key == "page")
    {
        return " content.page = " + value;
    }
    if(key == "contains" || key == "c")
    {
        return " ( COALESCE( (SELECT 1 FROM content_fts WHERE content_fts.content MATCH '" + value + "' AND content_fts.ROWID= content.id), 0 ) )";
    }
    qDebug() << "NOHIT" << key;
    //TODO: exception?
    return "NOTHING";
}

QString SearchWorker::makeSql(const QVector<SearchWorker::Command> &tokens)
{
    QString result;
    for(const Command &c : tokens)
    {
        result += createSql(c);
    }
    return result;
}

void SearchWorker::search(const QString &query)
{
    QSqlQuery dbquery(db);
    QVector<SearchResult> results;
    QString whereSql = makeSql(tokenize(query));

    QString prep;
    //TODO: hack, as we don't wanna look into content and get redundant results, when we don't even care about content
    if(whereSql.contains("content."))
    {
        prep = "SELECT file.path AS path, content.page AS page, file.mtime AS mtime FROM file INNER JOIN content ON file.id = content.fileid INNER JOIN content_fts ON content.id = content_fts.ROWID WHERE 1=1 AND " + whereSql + " ORDER By file.mtime DESC, content.page ASC";
    }
    else
    {
        prep = "SELECT file.path AS path, 0 as page, file.mtime AS mtime FROM file WHERE " + whereSql + " ORDER by file.mtime DESC";
    }
    dbquery.prepare(prep);
    dbquery.exec();
    qDebug() << "prepped: " << prep;
    qDebug() << dbquery.lastError();
    while(dbquery.next())
    {
        SearchResult result;

        result.path = dbquery.value("path").toString();
        result.page = dbquery.value("page").toUInt();
        result.mtime = dbquery.value("mtime").toUInt();
        results.append(result);
    }
    emit searchResultsReady(results);
}

