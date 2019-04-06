#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H
#include <QString>

class SearchResult
{
public:
    unsigned int page;
    QString path;
    uint64_t mtime;
    uint64_t size;
    QChar filetype;
    SearchResult();
};

#endif // SEARCHRESULT_H
