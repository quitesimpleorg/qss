
#include <QRegularExpression>
#include <QDebug>
#include <QSqlError>
#include <QStack>
#include "searchworker.h"
#include "../shared/sqlitesearch.h"
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
    if(!checkParanthesis(expression))
    {
        throw std::invalid_argument("Invalid paranthesis");
    }
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
    value = value.replace("'", "\\'");
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
        return " content.id IN (SELECT content_fts.ROWID FROM content_fts WHERE content_fts.content MATCH '" + value + "' )";
    }

    throw std::invalid_argument("Unknown filter: " + key.toStdString());
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

bool SearchWorker::checkParanthesis(QString expression)
{
    QStack<QChar> open;
    QStack<QChar> close;

    for(QChar &c : expression)
    {
        if(c == '(')
        {
            open.push(c);
        }
        if(c == ')')
        {
            close.push(c);
        }
    }
    if(open.size() != close.size())
    {
        return false;
    }
    while(!open.empty() && !close.empty())
    {
        QChar o = open.pop();
        QChar c = close.pop();
        if(o != '(' && c != ')' )
        {
            return false;
        }
    }
    return true;
}

void SearchWorker::search(const QString &query)
{
    SqliteSearch searcher(db);
    emit searchResultsReady(searcher.search(query));

}

