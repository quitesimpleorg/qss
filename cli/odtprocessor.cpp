#include <quazip5/quazip.h>
#include <quazip5/quazipfile.h>
#include "odtprocessor.h"
#include "tagstripperprocessor.h"


QVector<PageData> OdtProcessor::process(const QByteArray &data) const
{
    throw QSSGeneralException("Not implemented yet");
}

QVector<PageData> OdtProcessor::process(QString path) const
{
    QuaZipFile zipFile(path);
    zipFile.setFileName("content.xml");
    if(!zipFile.open(QIODevice::ReadOnly))
    {
       throw QSSGeneralException("Error while opening file " + path);
    }
    QByteArray entireContent = zipFile.readAll();
    if(entireContent.isEmpty())
    {
        throw QSSGeneralException("Error while reading content.xml of " + path);
    }
    TagStripperProcessor tsp;
    return tsp.process(entireContent);
}
