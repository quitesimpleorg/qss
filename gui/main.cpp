#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include "mainwindow.h"
#include "searchresult.h"
#include "pdfpreview.h"
#include "../shared/common.h"

int main(int argc, char *argv[])
{
	Common::setupAppInfo();
	QApplication a(argc, argv);
	try
	{
		Common::ensureConfigured();
	}
	catch(QSSGeneralException &e)
	{
		qDebug() << e.message;
		QMessageBox::critical(nullptr, "Error", e.message);
		return 1;
	}
	qRegisterMetaType<QVector<SearchResult>>("QVector<SearchResult>");
	qRegisterMetaType<QVector<PdfPreview>>("QVector<PdfPreview>");
	qRegisterMetaType<PdfPreview>("PdfPreview");
	MainWindow w;
	w.showMaximized();

	return a.exec();
}
