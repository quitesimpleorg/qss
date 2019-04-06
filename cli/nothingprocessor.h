#ifndef NOTHINGPROCESSOR_H
#define NOTHINGPROCESSOR_H
#include <QVector>
#include "processor.h"
#include "pagedata.h"

class NothingProcessor : public Processor
{
public:
    NothingProcessor();

public:
    QVector<PageData> process(const QByteArray &data) const override {
        return { };
    }
};


#endif // NOTHINGPROCESSOR_H
