#ifndef COMMANDDELETE_H
#define COMMANDDELETE_H
#include "command.h"

class CommandDelete : public Command
{
public:
    using Command::Command;

    int handle(QStringList arguments) override;
};

#endif // COMMANDDELETE_H
