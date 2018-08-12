#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTreeWidgetItem>
#include <QFileIconProvider>

#include "searchworker.h"
#include "pdfworker.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void beginFileSearch(const QString &query);
    void beginContentSearch(const QString &query);
    void startPdfPreviewGeneration(QVector<SearchResult> paths, double scalefactor);
private:
    Ui::MainWindow *ui;
    QFileIconProvider iconProvider;
    bool pdfDirty;
    SearchWorker *searchWorker;
    PdfWorker *pdfWorker;
    void add(QString path, unsigned int page);
    QThread searchThread;
    QThread pdfWorkerThread;
    QVector<SearchResult> pdfSearchResults;
    void connectSignals();
    void makePdfPreview();
    bool pdfTabActive();
private slots:
    void lineEditReturnPressed();
    void lineEditTextChanged();
    void handleSearchResults(const QVector<SearchResult> &results);
    void handleCancelledSearch();
    void treeSearchItemActivated(QTreeWidgetItem *item, int i);

    void showSearchResultsContextMenu(const QPoint &point);
    void tabChanged();
    void pdfPreviewReceived(PdfPreview preview);
};

#endif // MAINWINDOW_H
