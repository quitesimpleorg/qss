
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QScopedPointer>
#include <QMutexLocker>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentMap>
#include <atomic>
#include "pdfworker.h"


static QMutex cacheMutex;
struct Renderer
{

    typedef PdfPreview result_type;
    double scaleX;
    double scaleY;
    QHash<QString, Poppler::Document *> documentcache;
    qsizetype maxTotalPreviewImageMemUsage;
    std::atomic<qsizetype> currentTotalPreviewImageMemUsage { 0 };

    Renderer(double scaleX, double scaleY, qsizetype maxPreviewImageMemUsage)
    {
        this->scaleX = scaleX;
        this->scaleY = scaleY;
        this->maxTotalPreviewImageMemUsage = maxPreviewImageMemUsage;

    }

    /*we need this one because std::atomic has none, but this is only a functor for
    concurrentmap, thus, it's ok for it to be half-broken*/
    Renderer(const Renderer &o)
    {
        this->scaleX = o.scaleX;
        this->scaleY = o.scaleY;
    }

    ~Renderer()
    {
        qDeleteAll(documentcache);
    }


    Poppler::Document *document(QString path)
    {
        if(documentcache.contains(path))
            return documentcache.value(path);

        Poppler::Document *result = Poppler::Document::load(path);
        if(result == nullptr)
        {
            //TODO: some kind of user feedback would be nice
            return nullptr;
        }
        result->setRenderHint(Poppler::Document::TextAntialiasing);
        QMutexLocker locker(&cacheMutex);
        documentcache.insert(path, result);
        return result;
    }



    PdfPreview operator()(const PdfPreview &preview)
    {
        PdfPreview result = preview;
        if(currentTotalPreviewImageMemUsage.load() < maxTotalPreviewImageMemUsage)
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
            QImage img = pdfPage->renderToImage(scaleX, scaleY);
            result.previewImage = img;
            currentTotalPreviewImageMemUsage += img.sizeInBytes();
        }
        return result;
    }
};



QFuture<PdfPreview> PdfWorker::generatePreviews(const QVector<SearchResult> paths, double scalefactor)
{
    QVector<PdfPreview> previews;

    for(const SearchResult &sr : paths)
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

    QSettings setting;
    qsizetype maxPreviewImageMemUsage = setting.value("maxpreviewimagesmemory", 1024 * 1024 * 1024).toLongLong();

    return QtConcurrent::mapped(previews, Renderer(scaleX, scaleY, maxPreviewImageMemUsage));
}

