#ifndef _FILE_LOG__0X20140610141327
#define _FILE_LOG__0X20140610141327

#include <stdio.h>

#define MAXSIZE 1024
#define APP_ROOT_PATH "/system/strongunion/"

void write_sys_log_text(char* szWriteString)
{
#ifdef DEBUG
	char szLogPath[MAXSIZE]={0};
	FILE* fp;
	char szTime[50];
	time_t ti;
	//get_current_path(szFilePath, MAXSIZE);
	sprintf(szLogPath, "%s%s", APP_ROOT_PATH, "com");
	fp = fopen(szLogPath, "a+");
	time(&ti);
	struct tm* ptm = localtime(&ti);
	sprintf(szTime, "Now time is:%d/%d/%d %d:%d:%d", ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(fp,"%s>>----- %s\n", szTime, szWriteString);
	fclose(fp);
#endif
}

void write_sys_log_int(int nWrite)
{
#ifdef DEBUG
	char szString[10]={0};
	sprintf(szString, "%d", nWrite);
	return write_sys_log_text(szString);
#endif
}

#endif
