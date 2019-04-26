#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H
#include "filedata.h"

class SearchResult
{
public:
    FileData fileData;
    QVector<unsigned int> pages;

};

#endif // SEARCHRESULT_H
