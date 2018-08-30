#ifndef PDFWORKER_H
#define PDFWORKER_H
#include <QObject>
#include <QImage>
#include <QHash>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <poppler-qt5.h>
#include "pdfpreview.h"
#include "searchresult.h"
class PdfWorker : public QObject
{
    Q_OBJECT

private:
    QHash<QString, Poppler::Document *> documentcache;
    Poppler::Document *document(QString path);
    std::atomic<bool> cancelCurrent { false } ;
    std::atomic<bool> generating { false };
    QMutex isFreeMutex;
    QWaitCondition isFree;
public:
    PdfWorker();
    void cancelAndWait();
public slots:
    void generatePreviews(QVector<SearchResult> paths, double scalefactor);
signals:
    void previewReady(PdfPreview p);
    void previewsFinished();
};

#endif // PDFWORKER_H
