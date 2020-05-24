#include <QProcessEnvironment>
#include <QSettings>
#include "common.h"

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
		return settings.value("dbpath").toString();
	}
	return env;
}
