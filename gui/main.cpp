#include <QApplication>
#include <QSettings>
#include "mainwindow.h"
#include "searchresult.h"
#include "pdfpreview.h"
#include "../shared/common.h"

int main(int argc, char *argv[])
{
	Common::setupAppInfo();
	QApplication a(argc, argv);
	qRegisterMetaType<QVector<SearchResult>>("QVector<SearchResult>");
	qRegisterMetaType<QVector<PdfPreview>>("QVector<PdfPreview>");
	qRegisterMetaType<PdfPreview>("PdfPreview");
	MainWindow w;
	w.showMaximized();

	return a.exec();
}
