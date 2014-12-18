/*
 * SqliteManager.h
 *
 *  Created on: Jun 13, 2014
 *      Author: leen
 */

#ifndef SQLITEMANAGER_H_
#define SQLITEMANAGER_H_

#include <sqlite3.h>
#include <stddef.h>

typedef  struct _TABLEENTITY
{
	_TABLEENTITY():row(0), column(0), result(NULL)
	{
	}
	int row;
	int column;
	char**  result;
}TABLEENTITY;


class SqliteManager {
public:
	SqliteManager();
	SqliteManager(const char *dbname);
	virtual ~SqliteManager();
	bool open_sqlite_db(const char *dbname);
	bool create_sqlite_table(const char *tablename,const char *SQL);
	bool insert_sqlite_table(const char *tablename,const char *col, const char *str);
	bool query_sqlite_table(const char *tablename, TABLEENTITY& te);
	bool query_sqlite_table(const char *tablename, const char *field, char* value);
	bool query_sqlite_table(const char *tablename, const char *field, char* value, const char* rowfield, const char* rowval);
	int query_sqlite_table_row_num(const char *tablename);
	bool execute_sqlite_table(const char *sql);
	void close_sqlite_db();
private:
	sqlite3* m_sqdb;
	TABLEENTITY m_te;
};

#endif /* SQLITEMANAGER_H_ */
