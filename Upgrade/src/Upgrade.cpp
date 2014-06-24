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

#define ROWSIZE  		 200
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

pid_t* find_pid_by_name( char* pidName)
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
    return pidList;
}

void KillProcess()
{
	pid_t* pId;
	pId = find_pid_by_name("adb");
	if(pId != NULL)
	{
	    if(*pId > 0)
	    {
	        kill(*pId, SIGKILL);
	    }
	}
    pId = find_pid_by_name("center");
	if(pId != NULL)
	{
	    if(*pId > 0)
	    {
	        kill(*pId, SIGKILL);
	    }
	}
}

void CopyFile()
{
	char csSrcFile[PATH_MAX] = {0};
	char csDesFile[PATH_MAX] = {0};
	char csPath[PATH_MAX] = {0};
	int nCount=0;
	sprintf(csPath, "%s%s", GetPath(), SETTINGINI);
	if(!is_file_exist(csPath))
		sprintf(csPath, "%s%s", SETTINGPATH, SETTINGINI);
	RegTool::GetPrivateProfileInt(COPY, COUNT, nCount, csPath);
	for(int i=0; i<nCount; i++)
	{
		char csTemp[ROWSIZE];
		sprintf(csTemp, "%s%d", SRCFILE, i+1);
		RegTool::GetPrivateProfileString(COPY, csTemp, csSrcFile, csPath);
		sprintf(csTemp, "%s%d", DESFILE, i+1);
		RegTool::GetPrivateProfileString(COPY, csTemp, csDesFile, csPath);
		rename(csSrcFile, csDesFile);
	}
}

void RunExe()
{
	char szExeFile[PATH_MAX] = {0};
	char csExeFile[PATH_MAX] = {0};
	char csPath[PATH_MAX] = {0};
	int nCount=0;
	sprintf(csPath, "%s%s", GetPath(), SETTINGINI);
	if(!is_file_exist(csPath))
		sprintf(csPath, "%s%s", SETTINGPATH, SETTINGINI);
	RegTool::GetPrivateProfileInt(RUN, COUNT, nCount, csPath);
	for(int i=0; i<nCount; i++)
	{
		char csTemp[PATH_MAX];
		sprintf(csTemp, "%s%d", EXEFILE, i+1);
		RegTool::GetPrivateProfileString(RUN, csTemp, csExeFile, csPath);
		if(strchr(csExeFile, '/')==NULL)
		{
			char szPath[PATH_MAX] = { 0 };
			get_current_path(szPath, PATH_MAX);
			sprintf(szExeFile, "%s%s", szPath, csExeFile);
			if(!is_file_exist(szExeFile))
				sprintf(szExeFile, "%s%s", SETTINGPATH, csExeFile);
		}
		systemdroid(szExeFile);
	}
}

int main() {
	KillProcess();
	CopyFile();
	RunExe();
	//cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
