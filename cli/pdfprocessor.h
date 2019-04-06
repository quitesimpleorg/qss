#ifndef PDFPROCESSOR_H
#define PDFPROCESSOR_H
#include "processor.h"
class PdfProcessor : public Processor
{
public:
    PdfProcessor();

public:
    QVector<PageData> process(const QByteArray &data) const override;
};

#endif // PDFPROCESSOR_H
