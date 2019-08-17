#include <QStack>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QDebug>
#include <optional>
#include <algorithm>
#include "qssquery.h"


const QVector<Token> &QSSQuery::getTokens() const
{
    return tokens;
}

const QVector<SortCondition> &QSSQuery::getSortConditions() const
{
    return sortConditions;
}

QueryType QSSQuery::getQueryType()
{
   return static_cast<QueryType>(tokensMask & COMBINED);
}

bool QSSQuery::checkParanthesis(QString expression)
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

std::optional<QueryField> fromString(QString fieldString)
{
    if(fieldString == "path" || fieldString == "file.path")
    {
        return FILE_PATH;
    }
    else if(fieldString == "mtime" || fieldString == "file.mtime")
    {
        return FILE_MTIME;
    }
    else if(fieldString == "size" || fieldString == "file.size")
    {
        return FILE_SIZE;
    }
    else if(fieldString == "content.text")
    {
        return CONTENT_TEXT;
    }
    else if(fieldString == "content.page" || fieldString == "page")
    {
        return CONTENT_TEXT_PAGE;
    }
    return { };
}

//sort:(mtime desc, page asc)
QVector<SortCondition> createSortConditions(QString sortExpression)
{
        QVector<SortCondition> result;
        QStringList splitted_inner = sortExpression.split(",");
        for(int i = 0; i < splitted_inner.length(); i++)
        {
            QStringList splitted = splitted_inner[i].split(" ");
            if(splitted.length() < 1 || splitted.length() > 2)
            {
                throw QSSGeneralException("sort specifier must have format [field] (asc|desc)");
            }


            QString field = splitted[0];
            auto queryField = fromString(field);
            if(!queryField)
            {
                throw QSSGeneralException("Unknown sort field supplied");
            }

            SortOrder order;
            if(splitted.length() == 2)
            {
                QString orderstr = splitted[1];
                if(orderstr.compare("asc", Qt::CaseInsensitive) == 0)
                {
                    order = ASC;
                }
                else if(orderstr.compare("desc", Qt::CaseInsensitive) == 0)
                {
                    order = DESC;
                }
                else
                {
                    throw QSSGeneralException("Unknown order specifier: " + order);
                }

            }
            else
            {
                order = ASC;
            }

            SortCondition condition;
            condition.field = queryField.value();
            condition.order = order;
            result.append(condition);


        }


    return result;
}


void QSSQuery::addToken(Token t)
{
    tokens.append(t);
    tokensMask |= t.type;
}

/* Builds the query from the supplied expression
 *
 * AND is the default boolean operator, when the user does not provide any
 * thus, "Downloads zip" becomes essentailly "path.contains:(Downloads) AND path.contains:(zip)"
 *
 * TODO: It's a bit ugly still*/
QSSQuery QSSQuery::build(QString expression)
{
    if(!checkParanthesis(expression))
    {
        throw QSSGeneralException("Invalid paranthesis");
    }

    QSSQuery result;
    //TODO: merge lonewords
    QRegularExpression rx("((?<filtername>(\\.|\\w)+):(?<args>\\((?<innerargs>[^\\)]+)\\)|([\\w,])+)|(?<boolean>AND|OR)|(?<negation>!)|(?<bracket>\\(|\\))|(?<loneword>\\w+))");
    QRegularExpressionMatchIterator i = rx.globalMatch(expression);
    auto previousWasBool = [&result]{ return ! result.tokens.empty() && ( (result.tokens.last().type & BOOL) == BOOL); };
    auto previousWas = [&result](TokenType t) { return ! result.tokens.empty() && ( result.tokens.last().type == t); };

    while(i.hasNext())
    {
        QRegularExpressionMatch m = i.next();
        QString boolean = m.captured("boolean");
        QString negation = m.captured("negation");
        QString filtername = m.captured("filtername");
        QString bracket = m.captured("bracket");
        QString loneword = m.captured("loneword");

        if(boolean != "")
        {
            if(previousWasBool())
            {
                throw QSSGeneralException("Can't have two booleans following each other");
            }
            if(previousWas(NEGATION))
            {
                throw QSSGeneralException("Can't have a negation preceeding a boolean");
            }
            if(boolean == "AND")
            {
                result.addToken(Token(BOOL_AND));
            }
            else if(boolean == "OR")
            {
                result.addToken(Token(BOOL_OR));
            }
        }
        if(negation != "")
        {
            if(previousWas(NEGATION))
            {
                throw QSSGeneralException("Can't have two negations following each other");
            }
            if(!previousWasBool())
            {
                result.addToken(Token(BOOL_AND)); //Implicit and, our default operation
            }
            result.addToken(Token(NEGATION));
        }
        if(!result.tokens.isEmpty() && !previousWasBool() && !previousWas(NEGATION) && !previousWas(BRACKET_OPEN) && bracket != ")")
        {
            //the current token isn't a negation, isn't a boolean. Thus, implicit AND is required
            result.addToken(Token(BOOL_AND));
        }

        if(bracket != "")
        {
           if(bracket == "(")
           {
               result.addToken(Token(BRACKET_OPEN));
           }
           else
           {
               result.addToken(Token(BRACKET_CLOSE));
           }
        }


        if(loneword != "")
        {
            result.addToken(Token(FILTER_PATH_CONTAINS, loneword));
        }

        if(filtername != "")
        {
            TokenType tokenType;
            QString value = m.captured("innerargs");
            if(value == "")
            {
                value = m.captured("args");
            }

            if(filtername == "path.contains")
            {
                tokenType = FILTER_PATH_CONTAINS;
            }
            else if(filtername == "path.starts")
            {
                tokenType = FILTER_PATH_STARTS;
            }
            else if(filtername == "path.ends")
            {
                tokenType = FILTER_PATH_ENDS;
            }
            else if(filtername == "file.size" || filtername == "size")
            {
                tokenType = FILTER_PATH_SIZE;
            }
            else if(filtername == "c" || filtername == "contains")
            {
                tokenType = FILTER_CONTENT_CONTAINS;
            }
            else if(filtername == "page" || filtername == "content.page")
            {
                tokenType = FILTER_CONTENT_PAGE;
            }
            else if(filtername == "sort") //TODO: given this is not really a "filter", this feels slightly misplaced here
            {
                if(!result.sortConditions.empty())
                {
                    throw QSSGeneralException("Two sort statements are illegal");
                }
                result.sortConditions = createSortConditions(value);
                continue;
            }
            else
            {
                throw QSSGeneralException("Unknown filter provided!");
            }
            result.addToken(Token(tokenType, value));
        }

    }

    bool contentsearch = result.getTokensMask() & FILTER_CONTENT == FILTER_CONTENT;
    bool sortsForContent = std::any_of(result.sortConditions.begin(), result.sortConditions.end(), [](SortCondition c){ return c.field == CONTENT_TEXT; });

    if(!contentsearch && sortsForContent)
    {
        throw QSSGeneralException("We cannot sort by text if we don't search for it");
    }


    return result;
}
