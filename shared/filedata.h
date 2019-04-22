#ifndef FILEDATA_H
#define FILEDATA_H
#include <QString>

class FileData
{
public:
    QString absPath;
    uint mtime;
    uint size;
    QChar filetype;
};

#endif // FILEDATA_H
