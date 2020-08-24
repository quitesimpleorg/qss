#ifndef COMMON_H
#define COMMON_H
#include <QCoreApplication>

namespace Common
{
	void setupAppInfo();
	QString databasePath();
	bool initSqliteDatabase(QString path);
	void ensureConfigured();
}
#endif
