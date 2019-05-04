#include <QStack>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QDebug>
#include "sqlitesearch.h"
#include "qssgeneralexception.h"

SqliteSearch::SqliteSearch(QSqlDatabase &db)
{
    this->db = &db;
}

QVector<SqliteSearch::Token> SqliteSearch::tokenize(QString expression)
{
    if(!checkParanthesis(expression))
    {
        throw QSSGeneralException("Invalid paranthesis");
    }
    //TODO: merge lonewords
    QVector<SqliteSearch::Token> result;
    QRegularExpression rx("((?<filtername>(\\.|\\w)+):(?<args>\\((?<innerargs>[^\\)]+)\\)|([\\w,])+)|(?<boolean>AND|OR|!)|(?<bracket>\\(|\\))|(?<loneword>\\w+))");
    QRegularExpressionMatchIterator i = rx.globalMatch(expression);
    auto isSort = [](QString &key) { return key == "sort"; };
    auto isBool = [](QString &key) { return key == "AND" || key == "OR" || key == "!"; };
    while(i.hasNext())
    {
        QRegularExpressionMatch m = i.next();
        QString boolean = m.captured("boolean");
        QString filtername = m.captured("filtername");
        QString bracket = m.captured("bracket");
        QString loneword = m.captured("loneword");

        if(boolean != "")
        {
            result.append(Token(boolean));
        }

        if(!result.empty())
        {
            QString &lastKey = result.last().key;
            if(!isBool(lastKey) && !isSort(lastKey) && !isSort(filtername))
            {
                result.append(Token("AND"));
            }
        }

        if(bracket != "")
        {
           if(bracket == "(")
           {
               result.append(Token("AND"));
           }
          result.append(Token(bracket));
        }


        if(loneword != "")
        {
            result.append(Token("path.contains", loneword));
        }

        if(filtername != "")
        {
            QString value = m.captured("innerargs");
            if(value == "")
            {
                value = m.captured("args");
            }
            result.append(Token(filtername, value));
        }

    }
    return result;
}


QString SqliteSearch::fieldToColumn(QString field)
{
    if(field == "mtime" || field == "file.mtime")
    {
        return "file.mtime";
    }
    else if(field == "page" || field == "content.page")
    {
        return "content.page";
    }
    else if(field == "path" || field == "file.path")
    {
        return "file.path";
    }
    else if(field == "size" || field == "file.size")
    {
        return "file.size";
    }
    return "";
}


QString SqliteSearch::createSortSql(const SqliteSearch::Token &token)
{
    //sort:(mtime desc, page asc)
    if(token.key == "sort")
    {
        QString sortsql;
        QStringList splitted_inner = token.value.split(",");
        for(int i = 0; i < splitted_inner.length(); i++)
        {
            QStringList splitted = splitted_inner[i].split(" ");
            if(splitted.length() < 1 || splitted.length() > 2)
            {
                throw QSSGeneralException("sort specifier must have format [field] (asc|desc)");
            }

            QString field = splitted[0];
            field = fieldToColumn(field);
            if(field == "")
            {
                throw QSSGeneralException("Unknown sort field supplied");
            }

            QString order;
            if(splitted.length() == 2)
            {
                order = splitted[1];
                if(order.compare("asc", Qt::CaseInsensitive) == 0)
                {
                    order = "ASC";
                }
                else if(order.compare("desc", Qt::CaseInsensitive) == 0)
                {
                    order = "DESC";
                }
                else
                {
                    throw QSSGeneralException("Unknown order specifier: " + order);
                }

            }
            else
            {
                order = "ASC";
            }

            sortsql += field + " " + order;
            if(splitted_inner.length() - i > 1)
            {
                sortsql += ", ";
            }
        }
        return " ORDER BY " + sortsql;

    }
    return "";
}

QPair<QString, QVector<QString>> SqliteSearch::createSql(const SqliteSearch::Token &token)
{
    QPair<QString, QVector<QString>> result;

    QString key = token.key;
    QString value = token.value;
    value = value.replace("'", "\\'");
    if(key == "AND" || key == "OR" || key == "(" || key == ")")
    {
        return { " " + key + " ", QVector<QString>() };
    }
    if(key == "!")
    {
        return { " NOT ", QVector<QString>() };
    }
    if(key == "path.starts")
    {
        return { " file.path LIKE ? || '%' ", { value } };
    }
    if(key == "path.ends")
    {
        return { " file.path LIKE '%' || ? ", { value } };
    }
    if(key == "path.contains" || key == "inpath")
    {
        return { " file.path LIKE '%' || ? || '%' ", { value } } ;
    }
    if(key == "page")
    {
        return { " content.page = ?", { value } };
    }
    if(key == "contains" || key == "c")
    {
        return { " content.id IN (SELECT content_fts.ROWID FROM content_fts WHERE content_fts.content MATCH ?) " , { value } };
    }
    throw QSSGeneralException("Unknown token: " + key);
}

QSqlQuery SqliteSearch::makeSqlQuery(const QVector<SqliteSearch::Token> &tokens)
{
    QString whereSql;
    QString limitSql;
    QString sortSql;
    QVector<QString> bindValues;
    bool isContentSearch = false;
    for(const Token &c : tokens)
    {
        if(c.key == "sort")
        {
            if(sortSql != "")
            {
                throw QSSGeneralException("Invalid input: Two seperate sort statements are invalid");
            }
            sortSql = createSortSql(c);
        }
        else
        {
            if(c.key == "c" || c.key == "contains")
            {
                isContentSearch = true;
            }
            auto sql = createSql(c);
            whereSql += sql.first;
            bindValues.append(sql.second);
        }
    }

    QString prepSql;
    if(whereSql.isEmpty())
    {
        throw QSSGeneralException("Nothing to search for supplied");
    }
    if(isContentSearch)
    {
        if(sortSql.isEmpty())
        {
            sortSql =  "ORDER BY file.mtime DESC, content.page ASC";
        }
        prepSql = "SELECT file.path AS path, group_concat(content.page) AS pages, file.mtime AS mtime, file.size AS size, file.filetype AS filetype FROM file INNER JOIN content ON file.id = content.fileid WHERE 1=1 AND " + whereSql + " GROUP BY file.path  " + sortSql;
    }
    else
    {
        if(sortSql.isEmpty())
        {
            sortSql =  "ORDER BY file.mtime DESC";
        }
        if(sortSql.contains("content."))
        {
            throw QSSGeneralException("Cannot sort for content fields when not doing a content search");
        }

        prepSql = "SELECT file.path AS path, '0' as pages,  file.mtime AS mtime, file.size AS size, file.filetype AS filetype FROM file WHERE  1=1 AND " + whereSql + " " + sortSql;
    }

    QSqlQuery dbquery(*db);
    dbquery.prepare(prepSql);

    for(const QString &value : bindValues)
    {
        if(value != "")
        {
            dbquery.addBindValue(value);
        }
    }
   return dbquery;
}



QVector<SearchResult> SqliteSearch::search(const QString &query)
{
    QVector<SearchResult> results;
    QSqlQuery dbQuery = makeSqlQuery(tokenize(query));
    bool success = dbQuery.exec();
    if(!success)
    {

        qDebug() << dbQuery.lastError();
        qDebug() << dbQuery.executedQuery();
        throw QSSGeneralException("SQL Error: " + dbQuery.lastError().text());
    }

    while(dbQuery.next())
    {
        SearchResult result;
        result.fileData.absPath = dbQuery.value("path").toString();
        result.fileData.mtime = dbQuery.value("mtime").toUInt();
        result.fileData.size = dbQuery.value("size").toUInt();
        result.fileData.filetype = dbQuery.value("filetype").toChar();
        QString pages = dbQuery.value("pages").toString();
        QStringList pagesList = pages.split(",");
        for(QString &page : pagesList)
        {
            if(page != "")
            {
                result.pages.append(page.toUInt());
            }
        }
        results.append(result);
    }
    return results;
}

bool SqliteSearch::checkParanthesis(QString expression)
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
