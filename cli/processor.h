#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <QVector>
#include <QFile>
#include "pagedata.h"
#include "utils.h"
enum DataSource
{
    FILEPATH,
    ARRAY
};

class Processor
{
public:
    /* Indicates the data source the processor performs best with. For example,
     * you do not want to read the entire of a compressed archive just to get the content of
     * a single file */
    DataSource PREFERED_DATA_SOURCE = ARRAY;
    Processor();
    virtual QVector<PageData> process(const QByteArray &data) const = 0;
    virtual QVector<PageData> process(QString path) const
    {
        return process(Utils::readFile(path));

    }


    virtual ~Processor() { }
};

#endif // PROCESSOR_H
