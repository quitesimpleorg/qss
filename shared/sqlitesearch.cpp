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

QString SqliteSearch::fieldToColumn(QueryField field)
{
    if(field == FILE_MTIME)
    {
        return "file.mtime";
    }
    else if(field == FILE_PATH)
    {
        return "file.path";
    }
    else if(field == FILE_SIZE)
    {
        return "file.size";
    }
    else if(field == CONTENT_TEXT_PAGE)
    {
        return "content.page";
    }
    else if(field == CONTENT_TEXT)
    {
        return "content.text";
    }
    return "";
}


QString SqliteSearch::createSortSql(const QVector<SortCondition> sortConditions)
{
        QString sortsql;
        for(const SortCondition &sc : sortConditions)
        {
            QString order;
            QString field = fieldToColumn(sc.field);
            if(field == "")
            {
                throw QSSGeneralException("Unknown sort field supplied");
            }
            if(sc.order == DESC)
            {
                order = "DESC";
            }
            else
            {
                order = "ASC";
            }
            sortsql += field + " " + order + ", ";
        }
        sortsql.chop(2);
        if(!sortsql.isEmpty())
        {
            return " ORDER BY " + sortsql;
        }
        return "";
}

QPair<QString, QVector<QString>> createNonArgPair(QString key)
{
    return { " " + key + " ", QVector<QString>() };
}

QPair<QString, QVector<QString>> SqliteSearch::createSql(const Token &token)
{
    QPair<QString, QVector<QString>> result;
    QString value = token.value;
    value = value.replace("'", "\\'");

    if(token.type == BOOL_AND)
    {
        return createNonArgPair("AND");
    }
    if(token.type == BOOL_OR)
    {
        return createNonArgPair("OR");
    }
    if(token.type == NEGATION)
    {
        return createNonArgPair("NOT");
    }
    if(token.type == BRACKET_OPEN)
    {
        return createNonArgPair("(");
    }
    if(token.type == BRACKET_CLOSE)
    {
        return createNonArgPair(")");
    }
    if(token.type == FILTER_PATH_STARTS)
    {
        return { " file.path LIKE ? || '%' ", { value } };
    }
    if(token.type == FILTER_PATH_ENDS)
    {
        return { " file.path LIKE '%' || ? ", { value } };
    }
    if(token.type == FILTER_PATH_CONTAINS)
    {
        return { " file.path LIKE '%' || ? || '%' ", { value } } ;
    }
    if(token.type == FILTER_CONTENT_PAGE)
    {
        return { " content.page = ?", { value } };
    }
    if(token.type == FILTER_CONTENT_CONTAINS)
    {
        return { " content.id IN (SELECT content_fts.ROWID FROM content_fts WHERE content_fts.content MATCH ?) " , { value } };
    }
    throw QSSGeneralException("Unknown token passed (should not happen)");
}

QSqlQuery SqliteSearch::makeSqlQuery(const QSSQuery &query)
{
    QString whereSql;
    QVector<QString> bindValues;
    bool isContentSearch = query.getTokensMask() & FILTER_CONTENT == FILTER_CONTENT;
    if(query.getTokens().isEmpty())
    {
        throw QSSGeneralException("Nothing to search for supplied");
    }

    for(const Token &token : query.getTokens())
    {
           auto sql = createSql(token);
           whereSql += sql.first;
           bindValues.append(sql.second);
    }

    QString prepSql;
    QString sortSql = createSortSql(query.getSortConditions());
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



QVector<SearchResult> SqliteSearch::search(const QSSQuery &query)
{
    QVector<SearchResult> results;
    QSqlQuery dbQuery = makeSqlQuery(query);
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
