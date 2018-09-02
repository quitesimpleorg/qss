#include <poppler-qt5.h>
#include <QLabel>
#include <QtDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QSettings>
#include <QDateTime>
#include <QProcess>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clicklabel.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    searchWorker = new SearchWorker(QSettings().value("dbpath").toString());
    pdfWorker = new PdfWorker();
     searchWorker->moveToThread(&searchThread);
    pdfWorker->moveToThread(&pdfWorkerThread);
    connectSignals();
    searchThread.start();
    ui->treeResultsList->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    ui->tabWidget->setCurrentIndex(0);
    ui->statusBar->addWidget(ui->lblSearchResults);
    ui->statusBar->addWidget(ui->pdfProcessBar);
    ui->pdfProcessBar->hide();
}

void MainWindow::connectSignals()
{
    connect(ui->txtSearch, &QLineEdit::returnPressed, this, &MainWindow::lineEditReturnPressed);
    connect(this, &MainWindow::beginSearch, searchWorker, &SearchWorker::search);
    connect(searchWorker, &SearchWorker::searchResultsReady, this, &MainWindow::handleSearchResults);
    connect(searchWorker, &SearchWorker::searchCancelled, this, &MainWindow::handleCancelledSearch);
    connect(ui->treeResultsList, &QTreeWidget::itemActivated, this, &MainWindow::treeSearchItemActivated);
    connect(ui->treeResultsList, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showSearchResultsContextMenu);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);
    connect(this, &MainWindow::startPdfPreviewGeneration, pdfWorker, &PdfWorker::generatePreviews);
    connect(pdfWorker, &PdfWorker::previewReady, this, &MainWindow::pdfPreviewReceived);
    connect(pdfWorker, &PdfWorker::previewsFinished, [&] { this->pdfDirty = false; });
}

bool MainWindow::pdfTabActive()
{
    return ui->tabWidget->currentIndex() == 1;
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
    ui->pdfProcessBar->setValue(++processedPdfPreviews);
    connect(label, &ClickLabel::clicked, [=]() {
        QSettings settings;
        QString command = settings.value("pdfviewer").toString();
        qDebug() << command;
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
    //TODO: validate q;
    ui->lblSearchResults->setText("Searching...");
    emit beginSearch(q);
}

void MainWindow::handleSearchResults(const QVector<SearchResult> &results)
{
   this->pdfSearchResults.clear();
   ui->treeResultsList->clear();
   ui->lblSearchResults->setText("Results: " + QString::number(results.size()));
   for(const SearchResult &result : results)
   {
       QFileInfo pathInfo(result.path);
       QString fileName =pathInfo.fileName();
       QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeResultsList);
       QDateTime dt = QDateTime::fromSecsSinceEpoch(result.mtime);
       item->setIcon(0, iconProvider.icon(pathInfo));
       item->setText(0, fileName);
       item->setText(1, result.path);
       item->setText(2, dt.toString(Qt::ISODate));
       //TODO: this must be user definied or done more intelligently
       if(this->pdfSearchResults.size() < 300)
       {
           if(result.path.endsWith(".pdf"))
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
    if(!pdfWorkerThread.isRunning())
        pdfWorkerThread.start();

   pdfWorker->cancelAndWait();
   QCoreApplication::processEvents(); //Process not processed images
   qDeleteAll(ui->scrollAreaWidgetContents->children());

   ui->scrollAreaWidgetContents->setLayout(new QHBoxLayout());
   ui->pdfProcessBar->setMaximum(this->pdfSearchResults.size());
   processedPdfPreviews = 0;
   emit startPdfPreviewGeneration(this->pdfSearchResults, 0.75);
}


void MainWindow::handleCancelledSearch()
{

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
