#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QSqlError>
#include "commanddelete.h"
#include "logger.h"

int CommandDelete::remove(QString pattern, bool onlyDeleted, bool verbose, bool dryRun)
{
	int deleted = 0;
	int offset = 0;
	int limit = 1000;
	QVector<FileData> files;
	int processedRows = this->dbService->getFiles(files, pattern, 0, limit);
	while(processedRows > 0 )
	{
		for(FileData &file : files)
		{
			if(onlyDeleted && QFileInfo::exists(file.absPath))
			{
				if(verbose)
				{
					Logger::info() << "Skipping " << file.absPath << " as the file exists on the file system" << endl;
				}
			}
			else
			{
				if(!dryRun)
				{
					if(this->dbService->deleteFile(file.absPath))
					{
						if(verbose)
						{
							Logger::info() << "Deleted" << file.absPath << endl;
						}
						++deleted;
					}
					else
					{
						Logger::error()<< "Failed to delete:" << file.absPath << ", exiting." << endl;
						return 1;
					}
				}
				else
				{
					Logger::info() << "Would delete " << file.absPath << endl;
				}
			}
		}
		if(dryRun)
		{
			offset += limit;
		}
		else
		{
			offset = offset + limit - deleted;
		}
		files.clear();
		processedRows = this->dbService->getFiles(files, pattern, offset, limit);
		deleted = 0;
	}
	return 0;
}


int CommandDelete::removePaths(const QStringList &paths, bool verbose, bool dryRun)
{
    int result = 0;
    for(const QString &file : paths)
    {
        QFileInfo fileInfo(file);
        QString absPath = fileInfo.absoluteFilePath();
        if(this->dbService->fileExistsInDatabase(absPath))
        {
            if(!dryRun)
            {
                if(this->dbService->deleteFile(absPath))
                {
                    if(verbose)
                    {
                        Logger::info() << "Deleted" << absPath << endl;
                    }
                }
                else
                {
                    Logger::error()<< "Failed to delete:" << absPath << endl;
                    result = 1;
                }
            }

        }
        else
        {
            Logger::error() << "No such file in database:" << absPath << endl;
            result = 1;
        }
    }
    return result;
}

int CommandDelete::handle(QStringList arguments)
{
	QCommandLineParser parser;
	parser.addOptions({
						  { { "v", "verbose" }, "Print path of the files while deleting them" },
						  { { "n", "dry-run"}, "Only print which files would be deleted from the database, don't delete them"},
						  { "pattern", "Only delete files from index matching the pattern, e. g. */.git/*. Can be used to restrict --deleted or standalone.", "pattern" },
						  { "deleted", "Delete all files from the index that don't exist anymore. Can be restricted by --pattern." }
					  });

	parser.addHelpOption();
	parser.addPositionalArgument("delete", "Delete paths from the index", "delete [paths...]");

	parser.process(arguments);
	bool verbose = parser.isSet("verbose");
	bool dryRun = parser.isSet("dry-run");
	bool deleted = parser.isSet("deleted");
	QString pattern = parser.value("pattern");


	if(deleted || ! pattern.isEmpty())
	{
		int result = this->remove(pattern, deleted, verbose, dryRun);
		if(result != 0)
		{
			Logger::error() << "Removal operation did not succeed" << endl;
			return result;
		}
	}


	QStringList files = parser.positionalArguments();
	if(files.length() > 0)
	{
		int result = removePaths(files, verbose, dryRun);
		if(result != 0)
		{
			return result;
		}
	}

	return 0;
}
