#ifndef XMLSTRIPPERPROCESSOR_H
#define XMLSTRIPPERPROCESSOR_H
#include "defaulttextprocessor.h"

class TagStripperProcessor : public DefaultTextProcessor
{
public:
    TagStripperProcessor();

public:
    QVector<PageData> process(const QByteArray &data) const override;
};

#endif // XMLSTRIPPERPROCESSOR_H
