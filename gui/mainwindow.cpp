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
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QSettings().value("dbpath").toString());
    if(!db.open())
    {
        qDebug() << "failed to open database";
        throw std::runtime_error("Failed to open database");
    }
    connectSignals();
    searchThread.start();
    ui->treeResultsList->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    ui->tabWidget->setCurrentIndex(0);
    ui->statusBar->addWidget(ui->lblSearchResults);
    ui->statusBar->addWidget(ui->pdfProcessBar);
    ui->pdfProcessBar->hide();
    QSettings settings;
    ui->comboScale->setCurrentText(settings.value("currentScale").toString());
}

void MainWindow::connectSignals()
{
    connect(ui->txtSearch, &QLineEdit::returnPressed, this, &MainWindow::lineEditReturnPressed);
   // connect(this, &MainWindow::beginSearch, searchWorker, &SearchWorker::search);
    connect(&searchWatcher, &QFutureWatcher<SearchResult>::finished, this, [&]{
        try
        {
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


   // connect(searchWorker, &SearchWorker::searchCancelled, this, &MainWindow::handleCancelledSearch);
  //  connect(searchWorker, &SearchWorker::searchError, this, &MainWindow::handleSearchError);
    connect(ui->treeResultsList, &QTreeWidget::itemActivated, this, &MainWindow::treeSearchItemActivated);
    connect(ui->treeResultsList, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showSearchResultsContextMenu);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);
    connect(ui->comboScale, qOverload<const QString &>(&QComboBox::currentIndexChanged), this, &MainWindow::comboScaleChanged);
}

void MainWindow::comboScaleChanged(QString text)
{
    QSettings scaleSetting;
    scaleSetting.setValue("currentScale", ui->comboScale->currentText());
    makePdfPreview();
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
            makePdfPreview();
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
    ClickLabel *label = new ClickLabel();
    label->setPixmap(QPixmap::fromImage(preview.previewImage));
    ui->scrollAreaWidgetContents->layout()->addWidget(label);
    connect(label, &ClickLabel::clicked, [=]() {
        QSettings settings;
        QString command = settings.value("pdfviewer").toString();
        if(command != "" && command.contains("%p") && command.contains("%f"))
        {
            command = command.replace("%f", preview.documentPath);
            command = command.replace("%p", QString::number(preview.page));
            QStringList splitted = command.split(" ");
            if(splitted.size() > 1)
            {
                QString cmd = splitted[0];
                QStringList args = splitted.mid(1);
                QProcess::startDetached(cmd, args);
            }
        }
        else
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(preview.documentPath));
        }
    });
}

void MainWindow::lineEditReturnPressed()
{
    QString q = ui->txtSearch->text();
    if(!SqliteSearch::checkParanthesis(q))
    {
        ui->lblSearchResults->setText("Invalid paranthesis");
        return;
    }
    //TODO: validate q;
    ui->lblSearchResults->setText("Searching...");
    QFuture<QVector<SearchResult>> searchFuture = QtConcurrent::run([&, q]() {
        SqliteSearch searcher(db);
        return searcher.search(q);
    });
    searchWatcher.setFuture(searchFuture);

}

void MainWindow::handleSearchResults(const QVector<SearchResult> &results)
{
   this->pdfSearchResults.clear();
   ui->treeResultsList->clear();
   ui->lblSearchResults->setText("Results: " + QString::number(results.size()));
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

       //TODO: this must be user defined or done more intelligently
       if(this->pdfSearchResults.size() < 300)
       {
           if(result.fileData.absPath.endsWith(".pdf"))
           {
               this->pdfSearchResults.append(result);
           }
       }
   }
   ui->treeResultsList->resizeColumnToContents(0);
   ui->treeResultsList->resizeColumnToContents(1);
   pdfDirty = ! this->pdfSearchResults.empty();
   if(pdfTabActive() && pdfDirty)
   {
       makePdfPreview();
   }

}

void MainWindow::makePdfPreview()
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
   PdfWorker worker;
   this->pdfWorkerWatcher.setFuture(worker.generatePreviews(this->pdfSearchResults, scaleText.toInt() / 100.));
   ui->pdfProcessBar->setMaximum(this->pdfWorkerWatcher.progressMaximum());
   ui->pdfProcessBar->setMinimum(this->pdfWorkerWatcher.progressMinimum());

}


void MainWindow::handleCancelledSearch()
{

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
