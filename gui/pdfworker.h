#ifndef PDFWORKER_H
#define PDFWORKER_H
#include <QObject>
#include <QImage>
#include <QHash>
#include <poppler-qt5.h>
#include "pdfpreview.h"
#include "searchresult.h"
class PdfWorker : public QObject
{
    Q_OBJECT

private:
    QHash<QString, Poppler::Document *> documentcache;
    Poppler::Document *document(QString path);
public:
    PdfWorker();
public slots:
    void generatePreviews(QVector<SearchResult> paths, double scalefactor);
signals:
    void previewReady(PdfPreview p);
    void previewsFinished();
};

#endif // PDFWORKER_H
