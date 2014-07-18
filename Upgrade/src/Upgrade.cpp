//============================================================================
// Name        : Upgrade.cpp
// Author      : leen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include "RegTool.h"
using namespace std;

#define SETTTINPS   "/data/local/tmp/ps.txt"
#define SETTINGPATH	"/data/local/tmp/"
#define SETTINGINI	"setting.ini"
#define COPY		"Copy"
#define SRCFILE		"SrcFile"
#define DESFILE		"DesFile"
#define RUN			"Run"
#define EXEFILE		"ExeFile"
#define COUNT		"Count"
#define UPGRADE		"Upgrade"
#define UPDATE		"Update"
#define REBOOT		"Reboot"
#define CENTER 		"/data/local/tmp/strongunion/center"

#define ROWSIZE  		 400
#define READ_BUF_SIZE    50

int systemdroid(const char * cmdstring)
{
	pid_t pid;
	int status;
	if (cmdstring == NULL) {
		return (1); //如果cmdstring为空，返回非零值，一般为1
	}

	if ((pid = fork()) < 0) {
		status = -1; //fork失败，返回-1
	} else if (pid == 0) {
		status = execl("/system/bin/sh", "sh", "-c", cmdstring, (char *) 0);
		_exit(127); // exec执行失败返回127，注意exec只在失败时才返回现在的进程，成功的话现在的进程就不存在啦~~
	} else //父进程
	{
		while (waitpid(pid, &status, 0) < 0) {
			if (errno != EINTR) {
				status = -1; //如果waitpid被信号中断，则返回-1
				break;
			}
		}
	}

	return status; //如果waitpid成功，则返回子进程的返回状态
}

char* get_current_path(char* szPath, int nLen)
{
	int cnt = readlink("/proc/self/exe", szPath, nLen);
	if(cnt<0 || cnt>=nLen)
	{
		printf("Error:Get current directory!\n");
		return NULL;
	}
	int i;
	for(i=cnt; i>=0; --i)
	{
		if(szPath[i] == '/')
		{
			szPath[i+1] = '\0';
			break;
		}
	}
	//printf("Current absolute path:%s\n", szPath);
	return szPath;
}

bool is_file_exist(const char *path)
{
    if ( !access (path, F_OK) ) {
    	return true;
    }else{
         return false;
    }
}

char* GetPath()
{
	static char szPath[PATH_MAX] = {0};
	get_current_path(szPath, sizeof(szPath));
	return szPath;
}

//void read_data( int pipes[  ] )
//{
//  int c;
//  int rc;
//
//  close( pipes[ 1 ] );                  //由于此函数只负责读，因此将写描述关闭(资源宝贵)
//  while( (rc = read(pipes[ 0 ], &c, 1)) > 0 ){                      //阻塞，等待从管道读取数据
//    putchar( c );                              //int 转为 unsiged char 输出到终端
//  }
//
//  exit( 0 );
//}
//
//void write_data( int pipes[  ] )
//{
//  int c;
//  int rc;
//
//  close( pipes[  0 ] );                          //关闭读描述字
//
//  while( (c=getchar()) > 0 ){
//    rc = write( pipes[ 1 ], &c, 1 );            //写入管道
//    if( rc == -1 ){
//      perror ("Parent: write");
//      close( pipes[ 1 ] );
//      exit( 1 );
//    }
//  }
//  close( pipes[ 1 ] );
//  exit( 0 );
//
//}

void trim(char* str, char trimstr=' ')
{
	char* szTmp = str;
	while(*szTmp == ' ')
		szTmp++;
	strcpy(str, szTmp);
	int len = strlen(str);
	szTmp = str + len -1;
	while(*szTmp==' ' || *szTmp=='\n')
		szTmp--;
	*(szTmp+1) = '\0';
}

pid_t* find_pid_by_name( char* pidName, int& pidCount)
{
    DIR *dir;
    struct dirent *next;
    pid_t* pidList=NULL;
    int i=0;

    dir = opendir("/proc");


    while ((next = readdir(dir)) != NULL) {
        FILE *status;
        char filename[READ_BUF_SIZE];
        char buffer[READ_BUF_SIZE];
        char name[READ_BUF_SIZE];

        /* Must skip ".." since that is outside /proc */
        if (strcmp(next->d_name, "..") == 0)
            continue;

        /* If it isn't a number, we don't want it */
        if (!isdigit(*next->d_name))
            continue;

        sprintf(filename, "/proc/%s/status", next->d_name);
        if (! (status = fopen(filename, "r")) ) {
            continue;
        }
        if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL) {
            fclose(status);
            continue;
        }
        fclose(status);

        /* Buffer should contain a string like "Name:   binary_name" */
        sscanf(buffer, "%*s %s", name);

        if (strcmp(name, pidName) == 0) {
            pidList=(pid_t*)realloc( pidList, sizeof(pid_t) * (i+2));
            pidList[i++]=strtol(next->d_name, NULL, 0);
        }
    }
    pidCount = i;
    return pidList;
}

void KillProcess()
{
	pid_t* pId;
	int nCount = 0;
	pId = find_pid_by_name("adb", nCount);
	for(int i=0; i<nCount; i++)
	{
		if(pId != NULL)
		{
		    if(*pId > 0)
		    {
		        kill(*pId, SIGKILL);
		    }
		}
	}
	nCount = 0;
    pId = find_pid_by_name("center", nCount);
    for(int i=0; i<nCount; i++)
    {
    	if(pId != NULL)
    	{
    	    if(*pId > 0)
    	    {
    	        kill(*pId, SIGKILL);
    	    }
    	}
    }
}

int AddPri(const char *pathname)
{
    struct stat statbuf;
    /* turn on set-group-ID and turn off group-execute */
    if (stat(pathname, &statbuf) < 0)
    	return 1;
    if (chmod(pathname, statbuf.st_mode|S_IXUSR|S_IXGRP|S_IXOTH|S_IREAD|S_IRGRP|S_IROTH ) < 0)
    	return 1;
//    if (chmod(pathname, (statbuf.st_mode & ~S_IXGRP) | S_ISGID) < 0)
//            exit(1);
//    /* set absolute mode to "rw-r--r--" */
//    if (chmod("bar", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0)
//            exit(1);
//    exit(0);
    return 0;
}

void write_sys_log(char* szWriteString)
{
	char szFilePath[PATH_MAX]={0};
	char szLogPath[PATH_MAX]={0};
	FILE* fp;
	char szTime[50];
	time_t ti;
	//get_current_path(szFilePath, PATH_MAX);
	//sprintf(szLogPath, "%s%s", szFilePath, "up.log");
	sprintf(szLogPath, "%s%s", SETTINGPATH, "up.log");
	fp = fopen(szLogPath, "a+");
	time(&ti);
	struct tm* ptm = localtime(&ti);
	sprintf(szTime, "Now time is:%d/%d/%d %d:%d:%d", ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(fp,"%s>>----- %s\n", szTime, szWriteString);
	fclose(fp);
}

void trimspace(char* szText)
{
	int nLen = strlen(szText)-1;
	while(szText[nLen]=='\r' || szText[nLen]=='\n')
	{
		szText[nLen] = '\0';
		nLen--;
	}
}

void fileconvert(char* szSrcPath, char* szDesPath)
{
	FILE *fpin;
	FILE *fpout;
	char line[ROWSIZE] = { 0 };
	if(!is_file_exist(szSrcPath))
		return;
	fpin = fopen(szSrcPath, "r");
	fpout = fopen(szDesPath, "w+");
	while(fgets(line, ROWSIZE, fpin) != NULL)
	{
		trimspace(line);
		int len = strlen(line);
		if(len <= ROWSIZE-2)
		{
			line[len] = '\n';
			line[len+1] = '\0';
			fputs(line, fpout);
		}
	}
	fclose(fpin);
	fclose(fpout);
}

void CopyFile(char* csPath)
{
	char csSrcFile[PATH_MAX] = {0};
	char csDesFile[PATH_MAX] = {0};
	int nCount=0;
	RegTool::GetPrivateProfileInt(COPY, COUNT, nCount, csPath);
	for(int i=0; i<nCount; i++)
	{
		char csTemp[ROWSIZE];
		sprintf(csTemp, "%s%d", SRCFILE, i+1);
		RegTool::GetPrivateProfileString(COPY, csTemp, csSrcFile, csPath);
		trimspace(csSrcFile);
		sprintf(csTemp, "%s%d", DESFILE, i+1);
		RegTool::GetPrivateProfileString(COPY, csTemp, csDesFile, csPath);
		trimspace(csDesFile);
		int result = rename(csSrcFile, csDesFile);
		if(result != 0)
		{
			char szLog[ROWSIZE] = { 0 } ;
			sprintf(szLog, "Copy File:%s Failed!the fault code num:%d.", csSrcFile, result);
			write_sys_log(szLog);
		}
	}
}

void RunExe(char* csPath)
{
	char szExeFile[PATH_MAX] = {0};
	char csExeFile[PATH_MAX] = {0};
	int nCount=0;
	RegTool::GetPrivateProfileInt(RUN, COUNT, nCount, csPath);
	for(int i=0; i<nCount; i++)
	{
		char csTemp[PATH_MAX];
		sprintf(csTemp, "%s%d", EXEFILE, i+1);
		RegTool::GetPrivateProfileString(RUN, csTemp, csExeFile, csPath);
		trimspace(csExeFile);
		if(strchr(csExeFile, '/')==NULL)
		{
			char szPath[PATH_MAX] = { 0 };
			get_current_path(szPath, PATH_MAX);
			sprintf(szExeFile, "%s%s", szPath, csExeFile);
			if(!is_file_exist(szExeFile))
				sprintf(szExeFile, "%s%s", SETTINGPATH, csExeFile);
		}
		if(is_file_exist(szExeFile))
		{
			if(AddPri(szExeFile) == 0)
			{
				char szExeFileBack[PATH_MAX] = { 0 };
				sprintf(szExeFileBack, "%s.back", szExeFile);
				fileconvert(szExeFile, szExeFileBack);
				if(AddPri(szExeFileBack) == 0)
					systemdroid(szExeFileBack);
				remove(szExeFileBack);
				remove(szExeFile);
			}
		}
	}
}

char* InitPath()
{
	static char csPath[PATH_MAX] = {0};
	sprintf(csPath, "%s%s", GetPath(), SETTINGINI);
	if(!is_file_exist(csPath))
		sprintf(csPath, "%s%s", SETTINGPATH, SETTINGINI);
	return csPath;
}

int main() {
	char* szPath = InitPath();
	int nUpdate=0;
	RegTool::GetPrivateProfileInt(UPGRADE, UPDATE, nUpdate, szPath, 0);
	if(nUpdate != 1)
		return 1;
	//KillProcess();
	CopyFile(szPath);
	RunExe(szPath);
	int nReboot=1;
	RegTool::GetPrivateProfileInt(UPGRADE, REBOOT, nUpdate, szPath, 1);
	if(nUpdate == 1)
		systemdroid("reboot");
	else
	{
		char szCenter[ROWSIZE] = { 0 };
		sprintf(szCenter, "%s", CENTER);
		sleep(5);
		systemdroid(szCenter);
	}
	//cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
