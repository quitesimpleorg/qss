
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
    if(result == nullptr)
    {
        return nullptr;
    }
    result->setRenderHint(Poppler::Document::TextAntialiasing);
    this->documentcache.insert(path, result);
    return result;
}
void PdfWorker::generatePreviews(QVector<SearchResult> paths, double scalefactor)
{
    this->cancelCurrent = false;
    this->generating = true;
    for(SearchResult &sr : paths)
    {
        if(this->cancelCurrent.load())
        {
            break;
        }
        Poppler::Document *doc = document(sr.fileData.absPath);
        if(doc == nullptr)
        {
            continue;
        }
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
        preview.documentPath = sr.fileData.absPath;
        preview.page = sr.page;
        emit previewReady(preview);
    }
    isFreeMutex.lock();
    isFree.wakeOne();
    isFreeMutex.unlock();
    generating = false;
    emit previewsFinished();
}

void PdfWorker::cancelAndWait()
{
    if(this->generating.load())
    {
        this->cancelCurrent = true;

        isFreeMutex.lock();
        isFree.wait(&isFreeMutex);
        isFreeMutex.unlock();
    }
}
