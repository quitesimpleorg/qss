#ifndef COMMANDDELETE_H
#define COMMANDDELETE_H
#include "command.h"

class CommandDelete : public Command
{
public:
    using Command::Command;

    int handle(QStringList arguments) override;
private:
    int removeNonExistent(bool verbose, bool dryRun, QString pattern);
    int removePaths(const QStringList &paths, bool verbose, bool dryRun);
};

#endif // COMMANDDELETE_H
