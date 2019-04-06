#ifndef PAGEDATA_H
#define PAGEDATA_H
#include <QString>
class PageData
{
public:

    unsigned int pagenumber = 0;
    QString content;

    PageData()
    {

    }
    PageData(unsigned int pagenumber, QString content)
    {
        this->pagenumber = pagenumber;
        this->content = content;
    }
};
#endif // PAGEDATA_H
