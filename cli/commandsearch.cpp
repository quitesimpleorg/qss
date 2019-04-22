#include <QCommandLineParser>
#include "commandsearch.h"
#include "databasefactory.h"
#include "logger.h"


int CommandSearch::handle(QStringList arguments)
{
    QCommandLineParser parser;
    parser.addOptions({
                          { { "r", "reverse" }, "Print most-recent changed files first" },
                          { "pattern", "Only delete files from index matching the pattern, e. g. */.git/*. Only applies to --deleted or standalone.", "pattern" },
                      });

    parser.addHelpOption();
    parser.addPositionalArgument("delete", "Delete paths from the index", "delete [paths...]");

    parser.process(arguments);
    bool reverse = parser.isSet("reverse");
    if(reverse)
    {
        throw QSSGeneralException("Reverse option to be implemented");
    }


    QStringList files = parser.positionalArguments();
    QString queryStrings = files.join(' ');

    auto results = dbService->search(queryStrings);

    for(SearchResult &result : results)
    {
        Logger::info() << result.fileData.absPath << endl;
    }

}
