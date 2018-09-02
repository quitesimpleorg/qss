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
    class Command
    {
    public:
        QString key;
        QString value;

        Command(QString key="", QString value="")
        {
            this->key = key;
            this->value = value;
        }

    };


    Q_OBJECT
private:
    QVector<Command> tokenize(QString expression);
    QString createSql(const Command &cmd);
    QString makeSql(const QVector<Command> &tokens);
    QSqlDatabase db;
public:
    SearchWorker();
    SearchWorker(const QString &dbpath);
    bool checkParanthesis(QString expression);
public slots:
    void search(const QString &query);
signals:
    void searchResultsReady(const QVector<SearchResult> &results);
    void searchCancelled();
    void searchError(QString e);
};

#endif // SEARCHWORKER_H
