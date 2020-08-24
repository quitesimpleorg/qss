#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QDebug>
#include "qssgeneralexception.h"
#include "common.h"

#define SETTINGS_KEY_DBPATH "dbpath"
#define SETTINGS_KEY_FIRSTRUN "firstrun"

inline void initResources() { Q_INIT_RESOURCE(create); }

bool Common::initSqliteDatabase(QString path)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(path);
	if(!db.open())
	{
		qDebug() << "failed to open database: " << path;
		return false;
	}
	initResources();
	QFile file(":./create.sql");
	if(!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "Failed to load SQL creation script from embedded resource";
		return false;
	}
	QTextStream stream(&file);
	db.transaction();
	while(!stream.atEnd())
	{
		QString sql = stream.readLine();
		qDebug() << sql;
		QSqlQuery sqlQuery;
		if(!sqlQuery.exec(sql))
		{
			qDebug() << "Failed to execute sql statement while initializing database: " << sqlQuery.lastError();
			db.rollback();
			return false;
		}
	}
	db.commit();
	db.close();
	file.close();
	return true;
}


void Common::ensureConfigured()
{
	QSettings settings;
	QVariant firstRun = settings.value(SETTINGS_KEY_FIRSTRUN);
	if(!firstRun.isValid())
	{
		QString dbpath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
		QDir dir;
		if(!dir.exists(dbpath))
		{
			if(!dir.mkpath(dbpath))
			{
				throw QSSGeneralException("Failed to create dbpath directory");
			}
		}
		dbpath += "/qss.sqlite";
		if(!initSqliteDatabase(dbpath))
		{
			throw QSSGeneralException("Failed to initialize sqlite database");
		}
		settings.setValue(SETTINGS_KEY_FIRSTRUN, false);
		settings.setValue(SETTINGS_KEY_DBPATH, dbpath);
	}
	else
	{

		QString dbpath = databasePath();
		if(!QFile::exists(dbpath))
		{
			throw QSSGeneralException("Database " + dbpath + " was not found");
		}
	}
}

void Common::setupAppInfo()
{
	QCoreApplication::setOrganizationName("quitesimple.org");
	QCoreApplication::setOrganizationDomain("quitesimple.org");
	QCoreApplication::setApplicationName("qss");
}

QString Common::databasePath()
{
	QString env = QProcessEnvironment::systemEnvironment().value("QSS_DB_OVERRIDE");
	if(env == "")
	{
		QSettings settings;
		return settings.value(SETTINGS_KEY_DBPATH).toString();
	}
	return env;
}
