#ifndef COMMANDSEARCH_H
#define COMMANDSEARCH_H
#include "command.h"
#include "../shared/sqlitesearch.h"

class CommandSearch : public Command
{
public:
    using Command::Command;

    int handle(QStringList arguments) override;

};

#endif // COMMANDSEARCH_H
