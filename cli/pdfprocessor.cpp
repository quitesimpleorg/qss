#include <QScopedPointer>
#include <poppler-qt5.h>
#include "pdfprocessor.h"
PdfProcessor::PdfProcessor()
{

}


QVector<PageData> PdfProcessor::process(const QByteArray &data) const
{
    QVector<PageData> result;
    QScopedPointer<Poppler::Document> doc(Poppler::Document::loadFromData(data));
    if(doc.isNull())
    {
        throw QSSGeneralException("Failed to process pdf data");
    }
    if(doc->isLocked())
    {
        throw QSSGeneralException("Doc is locked");
    }

    QRectF entirePage;

    auto pagecount = doc->numPages();
    QString entire;
    entire.reserve(data.size()); //TODO too much
    for(auto i = 0; i < pagecount; i++ )
    {
        QString text =doc->page(i)->text(entirePage);
        result.append({static_cast<unsigned int>(i+1),text });
        /*TODO: hack, so we can fts search several words over the whole document, not just pages.
         * this of course uses more space and should be solved differently.
        */
        entire += text;
    }
    result.append({0, entire});
    return result;
}
