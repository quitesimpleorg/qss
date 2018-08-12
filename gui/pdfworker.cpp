
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include "pdfworker.h"

PdfWorker::PdfWorker()
{

}

Poppler::Document * PdfWorker::document(QString path)
{
    if(this->documentcache.contains(path))
        return this->documentcache.value(path);

    Poppler::Document *result = Poppler::Document::load(path);
    result->setRenderHint(Poppler::Document::TextAntialiasing);
    this->documentcache.insert(path, result);
    return result;
}
void PdfWorker::generatePreviews(QVector<SearchResult> paths, double scalefactor)
{
    for(SearchResult &sr : paths)
    {
        Poppler::Document *doc = document(sr.path);
        qDebug() << sr.path;
        if(doc->isLocked())
        {
            continue;
        }
        int p = (int)sr.page - 1;
        if(p < 0)
            p = 0;
        Poppler::Page *pdfpage = doc->page(p);
        QImage image = pdfpage->renderToImage(QGuiApplication::primaryScreen()->physicalDotsPerInchX() * scalefactor, QGuiApplication::primaryScreen()->physicalDotsPerInchY() *scalefactor);

        PdfPreview preview;
        preview.previewImage = image;
        preview.documentPath = sr.path;
        emit previewReady(preview);
    }
    emit previewsFinished();
}
