#ifndef SQLITESEARCH_H
#define SQLITESEARCH_H
#include <QSqlDatabase>
#include <QPair>
#include "searchresult.h"

class SqliteSearch
{
    class Token
    {
        public:
            QString key;
            QString value;

            Token(QString key="", QString value="")
            {
                this->key = key;
                this->value = value;
            }


    };

public:


    SqliteSearch(QSqlDatabase &db);
    QVector<SearchResult> search(const QString &query);
    static bool checkParanthesis(QString expression);

private:
    QSqlDatabase *db;
    QVector<Token> tokenize(QString expression);
    QSqlQuery makeSqlQuery(const QVector<Token> &tokens);
    QString fieldToColumn(QString col);
    QPair<QString, QVector<QString> > createSql(const Token &token);
    QString createSortSql(const SqliteSearch::Token &token);
};

#endif // SQLITESEARCH_H
