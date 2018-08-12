#ifndef PDFPREVIEW_H
#define PDFPREVIEW_H
#include <QImage>

class PdfPreview
{
public:
    PdfPreview();
    QImage previewImage;
    QString documentPath;
};

#endif // PDFPREVIEW_H
