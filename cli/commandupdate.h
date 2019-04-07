#ifndef COMMANDUPDATE_H
#define COMMANDUPDATE_H
#include "command.h"

class CommandUpdate : public Command
{
public:
    CommandUpdate();


    int handle(QStringList arguments) override;
};

#endif // COMMANDUPDATE_H
