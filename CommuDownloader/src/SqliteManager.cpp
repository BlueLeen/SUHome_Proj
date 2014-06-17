/*
 * SqliteManager.cpp
 *
 *  Created on: Jun 13, 2014
 *      Author: leen
 */

#include "SqliteManager.h"
#include "LogFile.h"
#include <stdio.h>

#define APP_ROOT_PATH "/system/strongunion/"

SqliteManager::SqliteManager():m_sqdb(NULL) {
	// TODO Auto-generated constructor stub

}

SqliteManager::~SqliteManager() {
	// TODO Auto-generated destructor stub
}

SqliteManager::SqliteManager(char *dbname)
{
	open_sqlite_db(dbname);
}

bool SqliteManager::open_sqlite_db(char *dbname)
{
    int rc;
    rc=sqlite3_open(dbname, &m_sqdb);
    if( rc != SQLITE_OK )
    {
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
#ifdef DEBUG
    	char szLog[100] = { 0 };
    	sprintf(szLog, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	LogFile::write_sys_log(szLog, APP_ROOT_PATH);
#endif
		sqlite3_close(m_sqdb);
		return false;
    }

    return true;
}
bool SqliteManager::create_sqlite_table(char *tablename,char *SQL)
{
    char sql[1024];
    char *errmsg;
    int rc;
    sprintf(sql,"CREATE TABLE %s(%s);",tablename,SQL);
    rc = sqlite3_exec(m_sqdb,sql,0,0,&errmsg);
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	char szLog[100] = { 0 };
    	sprintf(szLog, "Can't create table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog, APP_ROOT_PATH);
#endif
    	return false;
    }
    return true;
}
bool SqliteManager::insert_sqlite_table(char *tablename,char *str)
{
    char sql[1024];
    char *errmsg;
    int rc;
    sprintf(sql,"INSERT INTO %s VALUES(%s);",tablename,str);
    rc = sqlite3_exec(m_sqdb,sql,0,0,&errmsg);
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	char szLog[100] = { 0 };
    	sprintf(szLog, "Can't insert table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog, APP_ROOT_PATH);
#endif
    	return false;
    }
    return true;
}
bool SqliteManager::query_sqlite_table(char *tablename)
{
    char sql[1024];
    int nrow = 0, ncolumn = 0;
    char *errmsg;
    char **azResult;
    int rc;
    int i=0,j=0;
    sprintf(sql,"SELECT * FROM %s",tablename);
    rc = sqlite3_get_table(m_sqdb,sql,&azResult, &nrow , &ncolumn , &errmsg );
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));

    	char szLog[100] = { 0 };
    	sprintf(szLog, "Can't query table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog, APP_ROOT_PATH);
#endif
    	return false;
    }
    printf("row:%d column=%d\n",nrow,ncolumn);
    for( i=1 ; i<=nrow; i++ ){
        printf("the %d row is ",i);
        for(j=i*ncolumn;j<(i+1)*ncolumn;j++)
            printf( "%s ",azResult[j] );
        printf("\n");
    }
    sqlite3_free_table(azResult);
    return true;
}
void SqliteManager::close_sqlite_db()
{
	sqlite3_close(m_sqdb);
}
