#ifndef QSSQUERY_H
#define QSSQUERY_H
#include <QString>
#include <QVector>
#include "qssgeneralexception.h"
#include "token.h"
/* Fields that can be queried or sorted */
enum QueryField
{
    FILE_PATH,
    FILE_MTIME,
    FILE_SIZE,
    CONTENT_TEXT,
    CONTENT_TEXT_PAGE
};

enum SortOrder
{
    ASC,
    DESC
};

struct SortCondition
{
    QueryField field;
    SortOrder order;
};

enum QueryType
{
    NOTHING = 0,
    PATH_ONLY = FILTER_PATH,
    CONTENT_ONLY = FILTER_CONTENT,
    COMBINED = PATH_ONLY | CONTENT_ONLY
};

class QSSQuery
{
private:
    /* Helper field to determine quertype as well as to quickly check what kind of filters etc.
     * are being used in this query*/
    int tokensMask;
    QVector<Token> tokens;
    QVector<SortCondition> sortConditions;
    void addToken(Token t);
public:
    const QVector<Token> & getTokens() const;
    const QVector<SortCondition> & getSortConditions() const;
    QueryType getQueryType();
    int getTokensMask() const { return tokensMask; }
    void addSortCondition(SortCondition sc);
    static bool checkParanthesis(QString query);
    static QSSQuery build(QString query);


};




#endif // QSSQUERY_H
