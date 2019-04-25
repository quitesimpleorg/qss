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
            wasbool = true;
            result.append(Token(boolean));
        }

        if(bracket != "")
        {
           if(!wasbool)
           {
               if(bracket == "(")
               {
                   result.append(Token("AND"));
               }
           }
           result.append(Token(bracket));
        }


        if(loneword != "")
        {
            if(!wasbool)
            {
                result.append(Token("AND"));
            }
            wasbool = false;
            result.append(Token("path.contains", loneword));
        }

        if(filtername != "")
        {
            if(!wasbool)
            {
                result.append(Token("AND"));
            }
            wasbool = false;
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
        QString sortsql = " ORDER BY ";
        QStringList splitted_inner = token.value.split(",");
        for(int i = 0; i < splitted_inner.length(); i++)
        {
            QStringList splitted = splitted_inner[i].split(" ");
            if(splitted.length() == 2)
            {
                QString field = splitted[0];
                QString order = splitted[1];
                if(order.compare("asc", Qt::CaseInsensitive))
                {
                    order = "ASC";
                }
                else if(order.compare("desc", Qt::CaseInsensitive))
                {
                    order = "DESC";
                }
                else
                {
                    throw QSSGeneralException("Unknown order specifier: " + order);
                }

                field = fieldToColumn(field);
                if(field == "")
                {
                    throw QSSGeneralException("Unknown field:" + field);
                }

                sortsql += field + " " + order;
                if(splitted_inner.length() - i > 1)
                {
                    sortsql += ", ";
                }
            }
            else if(splitted.length() == 1)
            {
                sortsql +=  splitted[0] + " ASC ";
            }
            else
            {
                throw QSSGeneralException("sort specifier must have format [field] (asc|desc)");
            }


        }
        return sortsql;
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
    QString sortSql;
    QString limitSql;
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
    if(isContentSearch)
    {
        prepSql = "SELECT file.path AS path, content.page AS page, file.mtime AS mtime, file.size AS size, file.filetype AS filetype FROM file INNER JOIN content ON file.id = content.fileid WHERE 1=1 AND " + whereSql + " " + sortSql;
    }
    else
    {
        prepSql = "SELECT file.path AS path, 0 as page,  file.mtime AS mtime, file.size AS size, file.filetype AS filetype FROM file WHERE  1=1 AND " + whereSql + " " + sortSql;
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
        throw QSSGeneralException("SQL Error: " + dbQuery.lastError().text());
    }

    while(dbQuery.next())
    {
        SearchResult result;
        result.fileData.absPath = dbQuery.value("path").toString();
        result.fileData.mtime = dbQuery.value("mtime").toUInt();
        result.fileData.size = dbQuery.value("size").toUInt();
        result.fileData.filetype = dbQuery.value("filetype").toChar();
        result.page = dbQuery.value("page").toUInt();
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
