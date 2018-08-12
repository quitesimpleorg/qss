#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include "searchresult.h"
#include "pdfpreview.h"
int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("quitesimple.org");
    QCoreApplication::setOrganizationDomain("quitesimple.org");
    QCoreApplication::setApplicationName("qss");
    QApplication a(argc, argv);
    qRegisterMetaType<QVector<SearchResult>>("QVector<SearchResult>");
    qRegisterMetaType<QVector<PdfPreview>>("QVector<PdfPreview>");
    qRegisterMetaType<PdfPreview>("PdfPreview");
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
