#ifndef COMMANDSEARCH_H
#define COMMANDSEARCH_H
#include "command.h"
#include "../shared/sqlitesearch.h"

class CommandSearch : public Command
{
private:
    SqliteSearch searcher;
public:
    using Command::Command;

    int handle(QStringList arguments) override
    {
        return 23;
    }

};

#endif // COMMANDSEARCH_H
