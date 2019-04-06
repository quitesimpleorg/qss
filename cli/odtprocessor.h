#ifndef ODTPROCESSOR_H
#define ODTPROCESSOR_H
#include "processor.h"
class OdtProcessor : public Processor
{
public:
    OdtProcessor()
    {
        this->PREFERED_DATA_SOURCE = FILEPATH;
    }
    QVector<PageData> process(const QByteArray &data) const override;

    QVector<PageData> process(QString path) const override;


};

#endif // ODTPROCESSOR_H
