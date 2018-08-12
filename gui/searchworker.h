#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVector>
#include "searchresult.h"

class SearchWorker : public QObject
{
    Q_OBJECT
private:
    QSqlQuery *queryFile;
    QSqlQuery *queryContent;
public:
    SearchWorker();
    SearchWorker(const QString &dbpath);
public slots:
    void searchForFile(const QString &query);
    void searchForContent(const QString &query);
signals:
    void searchResultsReady(const QVector<SearchResult> &results);
    void searchCancelled();
};

#endif // SEARCHWORKER_H
