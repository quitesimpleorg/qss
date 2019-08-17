#ifndef SQLITESEARCH_H
#define SQLITESEARCH_H
#include <QSqlDatabase>
#include <QPair>
#include "searchresult.h"
#include "token.h"
#include "../shared/qssquery.h"
class SqliteSearch
{

public:
    SqliteSearch(QSqlDatabase &db);
    QVector<SearchResult> search(const QSSQuery &query);

private:
    QSqlDatabase *db;
    QSqlQuery makeSqlQuery(const QSSQuery &query);
    QString fieldToColumn(QueryField field);
    QPair<QString, QVector<QString> > createSql(const Token &token);
    QString createSortSql(const QVector<SortCondition> sortConditions);
};

#endif // SQLITESEARCH_H
