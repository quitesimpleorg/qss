#ifndef TOKEN_H
#define TOKEN_H
#include <QString>


enum TokenType
{
    WORD,
    NEGATION = 2,
    BOOL = 4,
    BOOL_AND,
    BOOL_OR,
    GROUP = 8,
    BRACKET_OPEN,
    BRACKET_CLOSE,
    SORT = 16,
    FILTER_PATH = 32,
    FILTER_PATH_MTIME,
    FILTER_PATH_CONTAINS,
    FILTER_PATH_SIZE,
    FILTER_PATH_ENDS,
    FILTER_PATH_STARTS,
    FILTER_CONTENT = 64,
    FILTER_CONTENT_CONTAINS,
    FILTER_CONTENT_PAGE,

};

class Token
{
public:
    Token()
    {

    }
    Token(TokenType type)
    {
        this->type = type;
    }

    Token(TokenType type, QString value)
    {
        this->type = type;
        this->value = value;
    }

    TokenType type;
    QString value;
};

#endif // TOKEN_H
