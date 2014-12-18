/*
 * SqliteManager.cpp
 *
 *  Created on: Jun 13, 2014
 *      Author: leen
 */

#include "SqliteManager.h"
#include "LogFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define APP_ROOT_PATH "/system/strongunion/"
#define MINSIZE  500

SqliteManager::SqliteManager():m_sqdb(NULL) {
	// TODO Auto-generated constructor stub

}

SqliteManager::~SqliteManager() {
	// TODO Auto-generated destructor stub
}

SqliteManager::SqliteManager(const char *dbname)
{
	open_sqlite_db(dbname);
}

bool SqliteManager::open_sqlite_db(const char *dbname)
{
    int rc;
    rc=sqlite3_open(dbname, &m_sqdb);
    if( rc != SQLITE_OK )
    {
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
#ifdef DEBUG
    	char szLog[MINSIZE] = { 0 };
    	sprintf(szLog, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	LogFile::write_sys_log(szLog);
#endif
		sqlite3_close(m_sqdb);
		return false;
    }

    return true;
}
bool SqliteManager::create_sqlite_table(const char *tablename,const char *SQL)
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
    	char szLog[MINSIZE] = { 0 };
    	sprintf(szLog, "Can't create table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog);
#endif
    	return false;
    }
    return true;
}
bool SqliteManager::insert_sqlite_table(const char *tablename,const char *col, const char *str)
{
    char sql[1024];
    char *errmsg;
    int rc;
    sprintf(sql,"INSERT INTO %s(%s) VALUES(%s);",tablename,col,str);
    rc = sqlite3_exec(m_sqdb,sql,0,0,&errmsg);
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	char szLog[MINSIZE] = { 0 };
    	sprintf(szLog, "Can't insert table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog);
#endif
    	return false;
    }
    return true;
}
bool SqliteManager::query_sqlite_table(const char *tablename, TABLEENTITY& te)
{
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
#endif
    char sql[1024];
    //int nrow = 0, ncolumn = 0;
    char *errmsg;
    //char **azResult;
    int rc;
    sprintf(sql,"SELECT * FROM %s",tablename);
    if(m_te.result != NULL)
    	sqlite3_free_table(m_te.result);
    rc = sqlite3_get_table(m_sqdb,sql,&m_te.result, &m_te.row , &m_te.column , &errmsg );
    te.row = m_te.row;
    te.column = m_te.column;
    te.result = m_te.result;
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	sprintf(szLog, "Can't query table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog);
#endif
    	return false;
    }
//#ifdef DEBUG
//    sprintf(szLog, "row:%d column=%d\n",m_te.row, m_te.column);
//    for( i=1 ; i<=m_te.row; i++ ){
//        sprintf(szLog, "%sthe %d row is ",szLog, i);
//        for(j=i*m_te.column;j<(i+1)*m_te.column;j++)
//            sprintf( szLog, "%s%s ",  szLog, m_te.result[j]);
//        sprintf(szLog, "%s\n", szLog);
//    }
//    LogFile::write_sys_log(szLog);
//#endif
//    printf("row:%d column=%d\n",m_te.row, m_te.column);
//    for( i=1 ; i<=nrow; i++ ){
//        printf("the %d row is ",i);
//        for(j=i*ncolumn;j<(i+1)*ncolumn;j++)
//            printf( "%s ",azResult[j] );
//        printf("\n");
//    }
//    sqlite3_free_table(azResult);
    return true;
}

bool SqliteManager::query_sqlite_table(const char *tablename, const char *field, char* value)
{
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
#endif
    char sql[1024];
    int nrow = 0, ncolumn = 0;
    char *errmsg;
    char **azResult;
    int rc;
    sprintf(sql,"SELECT %s FROM %s", field, tablename);
    rc = sqlite3_get_table(m_sqdb,sql,&azResult, &nrow , &ncolumn, &errmsg );
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	sprintf(szLog, "Can't query table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog);
#endif
    	return false;
    }
    strcpy(value, azResult[1*ncolumn]);
    sqlite3_free_table(azResult);
    return true;
}

bool SqliteManager::query_sqlite_table(const char *tablename, const char *field, char* value, const char* rowfield, const char* rowval)
{
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
#endif
    char sql[1024];
    int nrow = 0, ncolumn = 0;
    char *errmsg;
    char **azResult;
    int rc;
    sprintf(sql,"SELECT %s FROM %s where %s = '%s'", field, tablename, rowfield, rowval);
    rc = sqlite3_get_table(m_sqdb,sql,&azResult, &nrow , &ncolumn, &errmsg );
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	sprintf(szLog, "Can't query table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog);
#endif
    	return false;
    }
    strcpy(value, azResult[1*ncolumn]);
    sqlite3_free_table(azResult);
    return true;
}

int SqliteManager::query_sqlite_table_row_num(const char *tablename)
{
    char sql[1024];
    int count = 0;
    int nrow = 0, ncolumn = 0;
    char *errmsg;
    char **azResult;
    int rc;
    sprintf(sql,"SELECT count(*) FROM %s",tablename);
    rc = sqlite3_get_table(m_sqdb,sql,&azResult, &nrow , &ncolumn , &errmsg );
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
    	char szLog[MINSIZE] = { 0 };
    	sprintf(szLog, "Can't query table: %s\n", errmsg);
    	LogFile::write_sys_log(szLog);
#endif
    	return 0;
    }
    count = atoi((char*)azResult[1*ncolumn]);
    sqlite3_free_table(azResult);
    return count;
}

bool SqliteManager::execute_sqlite_table(const char *sql)
{
    char *errmsg;
    int rc;
    rc = sqlite3_exec(m_sqdb,sql,0,0,&errmsg);
    if( rc != SQLITE_OK )
    {
#ifdef DEBUG
		//fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqdb));
    	char szLog[MINSIZE] = { 0 };
    	sprintf(szLog, "Execute sql fail!: %s", errmsg);
    	LogFile::write_sys_log(szLog);
#endif
    	return false;
    }
    return true;
}

void SqliteManager::close_sqlite_db()
{
    if(m_te.result != NULL)
    	sqlite3_free_table(m_te.result);
	sqlite3_close(m_sqdb);
}
