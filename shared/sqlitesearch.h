#ifndef SQLITESEARCH_H
#define SQLITESEARCH_H
#include <QSqlDatabase>
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
private:
    QSqlDatabase *db;
    QVector<SqliteSearch::Token> tokenize(QString expression);
    QString createSql(const Token &token);
    QString makeSql(const QVector<Token> &tokens);
    bool checkParanthesis(QString expression);


public:
    SqliteSearch(QSqlDatabase &db);
    QVector<SearchResult> search(const QString &query);

};

#endif // SQLITESEARCH_H
