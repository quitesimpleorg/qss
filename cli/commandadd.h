#ifndef COMMANDADD_H
#define COMMANDADD_H
#include <QMutex>
#include "command.h"
enum AddFileResult
{
    OK,
    SKIPPED,
    DBFAIL
};

class CommandAdd : public Command
{
private:
    AddFileResult addFile(QString path);
    QMutex writeMutex;
public:
    using Command::Command;

    int handle(QStringList arguments) override;
};

#endif // COMMANDADD_H
