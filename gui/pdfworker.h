#ifndef PDFWORKER_H
#define PDFWORKER_H
#include <QObject>
#include <QImage>
#include <QHash>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QMutex>
#include <QFuture>
#include <poppler-qt5.h>
#include "pdfpreview.h"
#include "searchresult.h"


class PdfWorker : public QObject
{
    Q_OBJECT
public:
    QFuture<PdfPreview> generatePreviews(QVector<SearchResult> paths, double scalefactor);
};

#endif // PDFWORKER_H
