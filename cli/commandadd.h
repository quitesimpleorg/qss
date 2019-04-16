#ifndef COMMANDADD_H
#define COMMANDADD_H
#include <QMutex>
#include "command.h"
#include "filesaver.h"
class CommandAdd : public Command
{
private:
    SaveFileResult addFile(QString path);
public:
    using Command::Command;

    int handle(QStringList arguments) override;
};

#endif // COMMANDADD_H
