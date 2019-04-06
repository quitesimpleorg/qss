#include "tagstripperprocessor.h"

TagStripperProcessor::TagStripperProcessor()
{

}

QVector<PageData> TagStripperProcessor::process(const QByteArray &data) const
{
    auto result = DefaultTextProcessor::process(data);
    //TODO: does not work properly with <br> and does not deal with entities...

    result[0].content.remove(QRegExp("<[^>]*>"));
    return result;
}
