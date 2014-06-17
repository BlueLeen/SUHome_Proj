/*
 * SqliteManager.h
 *
 *  Created on: Jun 13, 2014
 *      Author: leen
 */

#ifndef SQLITEMANAGER_H_
#define SQLITEMANAGER_H_

#include <sqlite3.h>

class SqliteManager {
public:
	SqliteManager();
	SqliteManager(char *dbname);
	virtual ~SqliteManager();
	bool open_sqlite_db(char *dbname);
	bool create_sqlite_table(char *tablename,char *SQL);
	bool insert_sqlite_table(char *tablename,char *str);
	bool query_sqlite_table(char *tablename);
	void close_sqlite_db();
private:
	sqlite3* m_sqdb;
};

#endif /* SQLITEMANAGER_H_ */
