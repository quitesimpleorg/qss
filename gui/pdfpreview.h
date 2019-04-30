#ifndef PDFPREVIEW_H
#define PDFPREVIEW_H
#include <QImage>

class PdfPreview
{
public:
    PdfPreview();
    QImage previewImage;
    QString documentPath;
    unsigned int page;

    bool hasPreviewImage()
    {
        return ! previewImage.isNull();
    }
};

#endif // PDFPREVIEW_H
