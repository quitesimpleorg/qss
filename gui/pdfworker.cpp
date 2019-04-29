
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QScopedPointer>
#include <QMutexLocker>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentMap>
#include "pdfworker.h"


static QMutex cacheMutex;
struct Renderer
{

    typedef PdfPreview result_type;
    double scaleX;
    double scaleY;
    QHash<QString, Poppler::Document *> documentcache;
    Renderer(double scaleX, double scaleY)
    {
        this->scaleX = scaleX;
        this->scaleY = scaleY;
    }
    Poppler::Document *document(QString path)
    {
        if(documentcache.contains(path))
            return documentcache.value(path);

        Poppler::Document *result = Poppler::Document::load(path);
        if(result == nullptr)
        {
            return nullptr;
        }
        result->setRenderHint(Poppler::Document::TextAntialiasing);
        QMutexLocker locker(&cacheMutex);
        documentcache.insert(path, result);
        return result;
    }



    PdfPreview operator()(const PdfPreview &preview)
    {
        Poppler::Document *doc = document(preview.documentPath);
        if(doc == nullptr)
        {
            return preview;
        }
        if(doc->isLocked())
        {
            return preview;
        }
        int p = (int)preview.page - 1;
        if(p < 0)
        {
            p = 0;
        }
        Poppler::Page *pdfPage = doc->page(p);
        PdfPreview result = preview;
        result.previewImage  = pdfPage->renderToImage(scaleX, scaleY);
        return result;
    }
};



QFuture<PdfPreview> PdfWorker::generatePreviews(QVector<SearchResult> paths, double scalefactor)
{
    QVector<PdfPreview> previews;

    for(SearchResult &sr : paths)
    {
        for(int page : sr.pages)
        {
            PdfPreview p;
            p.documentPath = sr.fileData.absPath;
            p.page = page;
            previews.append(p);
        }
    }



    double scaleX = QGuiApplication::primaryScreen()->physicalDotsPerInchX() * scalefactor;
    double scaleY = QGuiApplication::primaryScreen()->physicalDotsPerInchY() * scalefactor;


    return QtConcurrent::mapped(previews, Renderer(scaleX, scaleY));
}

