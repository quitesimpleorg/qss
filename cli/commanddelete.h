#ifndef COMMANDDELETE_H
#define COMMANDDELETE_H
#include "command.h"

class CommandDelete : public Command
{
public:
	using Command::Command;

	int handle(QStringList arguments) override;
private:
	int remove(QString pattern, bool onlyDeleted, bool verbose, bool dryRun);
	int removePaths(const QStringList &paths, bool verbose, bool dryRun);
};

#endif // COMMANDDELETE_H
