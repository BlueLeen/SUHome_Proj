/*
 * LogFile.cpp
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#include "LogFile.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define APP_LOG_FILE  "log"
#define APP_LOG_PATH  "/data/local/tmp/"

LogFile::LogFile() {
	// TODO Auto-generated constructor stub

}

LogFile::~LogFile() {
	// TODO Auto-generated destructor stub
}

//void LogFile::write_sys_log(char* szWriteString, char* szFile)
//{
//	char szLogPath[SIZE]={0};
//	FILE* fp;
//	char szTime[50];
//	time_t ti;
//	//get_current_path(szFilePath, MAXSIZE);
//	sprintf(szLogPath, "%s%s", szFile, APP_LOG_FILE);
//	fp = fopen(szLogPath, "a+");
//	time(&ti);
//	struct tm* ptm = localtime(&ti);
//	sprintf(szTime, "Now time is:%d/%d/%d %d:%d:%d", ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
//	fprintf(fp,"%s>>----- %s\n", szTime, szWriteString);
//	fclose(fp);
//}
//
//void LogFile::write_sys_log(int nWrite, char* szFile)
//{
//	char szString[10]={0};
//	sprintf(szString, "%d", nWrite);
//	return write_sys_log(szString, szFile);
//}

void LogFile::write_sys_log(char* szWriteString)
{
	char szLogPath[SIZE]={0};
	FILE* fp;
	char szTime[50];
	time_t ti;
	//get_current_path(szFilePath, MAXSIZE);
	sprintf(szLogPath, "%s%s", APP_LOG_PATH, APP_LOG_FILE);
	fp = fopen(szLogPath, "a+");
	time(&ti);
	struct tm* ptm = localtime(&ti);
	sprintf(szTime, "Now time is:%d/%d/%d %d:%d:%d", ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(fp,"%s>>----- %s\n", szTime, szWriteString);
	fclose(fp);
}

void LogFile::write_sys_log(int nWrite)
{
	char szString[10]={0};
	sprintf(szString, "%d", nWrite);
	return write_sys_log(szString);
}
