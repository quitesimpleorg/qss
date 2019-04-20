#include <QStack>
#include <QRegularExpression>

#include "sqlitesearch.h"

SqliteSearch::SqliteSearch()
{

}

QVector<SqliteSearch::Token> SqliteSearch::tokenize(QString expression)
{
    if(!checkParanthesis(expression))
    {
        throw std::invalid_argument("Invalid paranthesis");
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



QString SqliteSearch::createSql(const SqliteSearch::Token &token)
{
    QString key = token.key;
    QString value = token.value;
    value = value.replace("'", "\\'");
    if(key == "AND" || key == "OR" || key == "(" || key == ")")
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

QString SqliteSearch::makeSql(const QVector<SqliteSearch::Token> &tokens)
{
    QString result;
    for(const Token &c : tokens)
    {
        result += createSql(c);
    }
    return result;
}

void SqliteSearch::search(const QString &query)
{

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
