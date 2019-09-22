#include <QCommandLineParser>
#include "commandlist.h"
#include "databasefactory.h"
#include "logger.h"

int CommandList::handle(QStringList arguments)
{
    QCommandLineParser parser;
    parser.addOptions({
                          { { "r", "reverse" }, "Print most-recent changed files first" },
                          { { "c", "count"}, "Counts the number of paths listed"},
                          { "pattern", "Only list files from index matching the pattern, e. g. */.git/*", "pattern" },
                      });

    parser.addHelpOption();
    parser.addPositionalArgument("list", "Lists paths in the index", "list [options]");

    parser.process(arguments);
    bool reverse = parser.isSet("reverse");
    if(reverse)
    {
        throw QSSGeneralException("Reverse option to be implemented");
    }


    QStringList files = parser.positionalArguments();
    QString queryStrings = files.join(' ');
    auto results = dbService->search(QSSQuery::build(queryStrings));

    for(SearchResult &result : results)
    {
        Logger::info() << result.fileData.absPath << endl;
    }

}
