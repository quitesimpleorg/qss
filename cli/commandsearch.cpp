#include <QCommandLineParser>
#include "commandsearch.h"
#include "databasefactory.h"
#include "logger.h"

int CommandSearch::handle(QStringList arguments)
{
    QCommandLineParser parser;
    parser.addOptions({
                          { { "r", "reverse" }, "Print most-recent changed files first. This is short for adding \"sort:(mtime asc)\" to the query." },
                      });

    parser.addHelpOption();
    parser.process(arguments);


    QStringList files = parser.positionalArguments();
    QString queryStrings = files.join(' ');
    QSSQuery query = QSSQuery::build(queryStrings);
    bool reverse = parser.isSet("reverse");
    if(reverse)
    {
        SortCondition sc;
        sc.field = FILE_MTIME;
        sc.order = ASC;
        query.addSortCondition(sc);
    }

    auto results = dbService->search(query);

    for(SearchResult &result : results)
    {
        Logger::info() << result.fileData.absPath << endl;
    }

}
