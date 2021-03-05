#include <poppler-qt5.h>
#include <QLabel>
#include <QtDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QtGlobal>
#include <QSettings>
#include <QDateTime>
#include <QProcess>
#include <QComboBox>
#include <QtConcurrent/QtConcurrent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clicklabel.h"
#include "../shared/sqlitesearch.h"
#include "../shared/qssgeneralexception.h"
#include "../shared/common.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	QSettings settings;

	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(Common::databasePath());
	if(!db.open())
	{
		qDebug() << "failed to open database";
		throw std::runtime_error("Failed to open database");
	}
	connectSignals();
	ui->treeResultsList->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	ui->tabWidget->setCurrentIndex(0);
	ui->statusBar->addWidget(ui->lblSearchResults);
	ui->statusBar->addWidget(ui->pdfProcessBar);
	ui->pdfProcessBar->hide();
	ui->comboScale->setCurrentText(settings.value("currentScale").toString());
	pdfPreviewsPerPage = settings.value("pdfPreviewsPerPage", 20).toInt();
	ui->spinPdfPreviewPage->setMinimum(1);
}

void MainWindow::connectSignals()
{
	connect(ui->txtSearch, &QLineEdit::returnPressed, this, &MainWindow::lineEditReturnPressed);
	connect(&searchWatcher, &QFutureWatcher<SearchResult>::finished, this, [&]{
		try
		{
			this->ui->txtSearch->setEnabled(true);
			auto results = searchWatcher.future().result();
			handleSearchResults(results);

		}
		catch(QSSGeneralException &e)
		{
			handleSearchError(e.message);
		}
	});

	connect(&pdfWorkerWatcher, &QFutureWatcher<PdfPreview>::resultReadyAt, this, [&](int index) {
		pdfPreviewReceived(pdfWorkerWatcher.resultAt(index));
	});
	connect(&pdfWorkerWatcher, &QFutureWatcher<PdfPreview>::progressValueChanged, ui->pdfProcessBar, &QProgressBar::setValue);
	connect(ui->treeResultsList, &QTreeWidget::itemActivated, this, &MainWindow::treeSearchItemActivated);
	connect(ui->treeResultsList, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showSearchResultsContextMenu);
	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);
	connect(ui->comboScale, qOverload<const QString &>(&QComboBox::currentIndexChanged), this, &MainWindow::comboScaleChanged);
	connect(ui->spinPdfPreviewPage, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::spinPdfPreviewPageValueChanged);

}

void MainWindow::spinPdfPreviewPageValueChanged(int val)
{
	makePdfPreview(val);
}

void MainWindow::comboScaleChanged(QString text)
{
	QSettings scaleSetting;
	scaleSetting.setValue("currentScale", ui->comboScale->currentText());
	makePdfPreview(ui->spinPdfPreviewPage->value());
}
bool MainWindow::pdfTabActive()
{
	return ui->tabWidget->currentIndex() == 1;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	bool quit = ((event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Q) || event->key() == Qt::Key_Escape);
	if(quit)
	{
		qApp->quit();
	}

	if(event->modifiers() & Qt::ControlModifier)
	{

		if(event->key() == Qt::Key_L)
		{
			ui->txtSearch->setFocus();
			ui->txtSearch->selectAll();
		}
	}
	QWidget::keyPressEvent(event);
}

void MainWindow::tabChanged()
{
	if(pdfTabActive())
	{
		if(pdfDirty)
		{
			makePdfPreview(ui->spinPdfPreviewPage->value());
		}
		ui->pdfProcessBar->show();
	}
	else
	{
		ui->pdfProcessBar->hide();
	}
}

void MainWindow::pdfPreviewReceived(PdfPreview preview)
{
	if(preview.hasPreviewImage())
	{
		ClickLabel *label = new ClickLabel();
		QString docPath = preview.documentPath;
		auto previewPage = preview.page;
		label->setPixmap(QPixmap::fromImage(preview.previewImage));
		label->setToolTip(preview.documentPath);
		ui->scrollAreaWidgetContents->layout()->addWidget(label);
		connect(label, &ClickLabel::leftClick, [docPath, previewPage]() {
			QSettings settings;
			QString command = settings.value("pdfviewer").toString();
			if(command != "" && command.contains("%p") && command.contains("%f"))
			{
				QStringList splitted = command.split(" ");
				if(splitted.size() > 1)
				{
					QString cmd = splitted[0];
					QStringList args = splitted.mid(1);
					args.replaceInStrings("%f", docPath);
					args.replaceInStrings("%p", QString::number(previewPage));

					QProcess::startDetached(cmd, args);
				}
			}
			else
			{
				QDesktopServices::openUrl(QUrl::fromLocalFile(docPath));
			}
		});
	}
}

void MainWindow::lineEditReturnPressed()
{
	QString q = ui->txtSearch->text();
	if(!QSSQuery::checkParanthesis(q))
	{
		ui->lblSearchResults->setText("Invalid paranthesis");
		return;
	}
	//TODO: validate q;
	ui->lblSearchResults->setText("Searching...");
	this->ui->txtSearch->setEnabled(false);
	QFuture<QVector<SearchResult>> searchFuture = QtConcurrent::run([&, q]() {
		SqliteSearch searcher(db);
		this->currentQuery = QSSQuery::build(q);
		return searcher.search(this->currentQuery);
	});
	searchWatcher.setFuture(searchFuture);

}

void MainWindow::handleSearchResults(const QVector<SearchResult> &results)
{
	this->pdfSearchResults.clear();
	ui->treeResultsList->clear();
	ui->lblSearchResults->setText("Results: " + QString::number(results.size()) + " files");
	for(const SearchResult &result : results)
	{
		QFileInfo pathInfo(result.fileData.absPath);
		QString fileName =pathInfo.fileName();
		QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeResultsList);

		QDateTime dt = QDateTime::fromSecsSinceEpoch(result.fileData.mtime);
		item->setIcon(0, iconProvider.icon(pathInfo));
		item->setText(0, fileName);
		item->setText(1, result.fileData.absPath);
		item->setText(2, dt.toString(Qt::ISODate));
		item->setText(3, this->locale().formattedDataSize(result.fileData.size));
		if(result.fileData.absPath.endsWith(".pdf"))
		{
			this->pdfSearchResults.append(result);
		}
	}
	ui->treeResultsList->resizeColumnToContents(0);
	ui->treeResultsList->resizeColumnToContents(1);
	pdfDirty = ! this->pdfSearchResults.empty();

	int numpages = ceil(static_cast<double>(this->pdfSearchResults.size()) / pdfPreviewsPerPage);
	ui->spinPdfPreviewPage->setMinimum(1);
	ui->spinPdfPreviewPage->setMaximum(numpages);
	ui->spinPdfPreviewPage->setValue(1);
	if(pdfTabActive() && pdfDirty)
	{
		makePdfPreview(1);
	}

}

void MainWindow::makePdfPreview(int page)
{

	this->pdfWorkerWatcher.cancel();
	this->pdfWorkerWatcher.waitForFinished();

	QCoreApplication::processEvents(); //Maybe not necessary anymore, depends on whether it's possible that a slot is still to be fired.
	qDeleteAll(ui->scrollAreaWidgetContents->children());

	ui->scrollAreaWidgetContents->setLayout(new QHBoxLayout());
	ui->pdfProcessBar->setMaximum(this->pdfSearchResults.size());
	processedPdfPreviews = 0;
	QString scaleText = ui->comboScale->currentText();
	scaleText.chop(1);

	QVector<QString> wordsToHighlight;
	QRegularExpression extractor(R"#("([^"]*)"|(\w+))#");
	for(const Token &token : this->currentQuery.getTokens())
	{
		if(token.type == FILTER_CONTENT_CONTAINS)
		{
			QRegularExpressionMatchIterator i = extractor.globalMatch(token.value);
			while(i.hasNext())
			{
				QRegularExpressionMatch m = i.next();
				QString value = m.captured(1);
				if(value.isEmpty())
				{
					value = m.captured(2);
				}
				wordsToHighlight.append(value);

			}
		}
	}
	PdfWorker worker;
	int end = pdfPreviewsPerPage;
	int begin = page * pdfPreviewsPerPage - pdfPreviewsPerPage;
	this->pdfWorkerWatcher.setFuture(worker.generatePreviews(this->pdfSearchResults.mid(begin, end), wordsToHighlight, scaleText.toInt() / 100.));
	ui->pdfProcessBar->setMaximum(this->pdfWorkerWatcher.progressMaximum());
	ui->pdfProcessBar->setMinimum(this->pdfWorkerWatcher.progressMinimum());

}


void MainWindow::handleSearchError(QString error)
{
	ui->lblSearchResults->setText("Error:"  + error);
}

void MainWindow::treeSearchItemActivated(QTreeWidgetItem *item, int i)
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(item->text(1)));
}

void MainWindow::showSearchResultsContextMenu(const QPoint &point)
{
	QTreeWidgetItem *item = ui->treeResultsList->itemAt(point);
	if(item == nullptr)
	{
		return;
	}
	QMenu menu("SearchResult", this);
	menu.addAction("Copy filename to clipboard", [&] { QGuiApplication::clipboard()->setText(item->text(0));});
	menu.addAction("Copy full path to clipboard", [&] {  QGuiApplication::clipboard()->setText(item->text(1)); });
	menu.addAction("Open containing folder", [&] {
		QFileInfo pathinfo(item->text(1));
		QString dir = pathinfo.absolutePath();
		QDesktopServices::openUrl(QUrl::fromLocalFile(dir));

	});
	menu.exec(QCursor::pos());
}


MainWindow::~MainWindow()
{
	delete ui;
}

