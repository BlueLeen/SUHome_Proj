/*
 * InterfaceFull.cpp
 *
 *  Created on: May 30, 2014
 *      Author: leen
 */

#include "InterfaceFull.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "LogFile.h"

#define MAXSIZE 1024
#define ROWSIZE 200
#define APK_DIR_NAME "apkdir"
#define ADB_ADB_NAME "adb"
#define APP_ROOT_PATH "/system/strongunion/"


InterfaceFull::InterfaceFull() {
	// TODO Auto-generated constructor stub

}

InterfaceFull::~InterfaceFull() {
	// TODO Auto-generated destructor stub
}

int InterfaceFull::install_android_apk(char* szApk)
{
	char shellComm[MAXSIZE] = { 0 };
	char szPath[MAXSIZE] = { 0 };
	char szApkPath[MAXSIZE] = { 0 };
	sprintf(szPath, "%s", APP_ROOT_PATH);
	sprintf(szApkPath, "%s%s/%s", szPath, APK_DIR_NAME, szApk);
	sprintf(shellComm, "%s kill-server", ADB_ADB_NAME);

	char shellCommDevice[MAXSIZE] = { 0 };
	char szFile[MAXSIZE] = { 0 };
	char szInfo[MAXSIZE] = { 0 };
	sprintf(szFile, "%s%s", APP_ROOT_PATH, "text");
	sprintf(shellCommDevice, "%s devices > %s", ADB_ADB_NAME, szFile);
	LogFile::write_sys_log(shellComm);
	execstream("adb wait-for-device", szInfo, sizeof(szInfo));
	int num1 = execstream(shellComm, szInfo, sizeof(szInfo));
	LogFile::write_sys_log(szInfo);
	while(systemdroid(shellComm)==0)
	{
		FILE *fpin;
		char line[ROWSIZE] = { 0 };
		char line_last[ROWSIZE] = { 0 };
		int num2 = execstream(shellCommDevice, szInfo, sizeof(szInfo));
		LogFile::write_sys_log(szInfo);
		LogFile::write_sys_log(shellCommDevice);
		{
			fpin = fopen(szFile, "r");
			while(fgets(line, ROWSIZE, fpin) != NULL)
			{
				LogFile::write_sys_log(line);
				if(!strcmp(line, "\n")) continue;
				strcpy(line_last, line);
			}
			if(strcmp(line_last, "") && phone_is_online(line_last))
			{
				fclose(fpin);
				break;
			}
			fclose(fpin);
		}
	}
	sprintf(shellComm, "%s install -r %s", ADB_ADB_NAME, szApkPath);
	LogFile::write_sys_log(shellComm);
	execstream(shellComm, szInfo, sizeof(szInfo));
	LogFile::write_sys_log(szInfo);
	//systemdroid("exit");
	return 0;
}

int InterfaceFull::phone_is_online(char* buf)
{
	char text[20];
	int nLen = strlen(buf);
	int nCur = nLen - 1;
	while(buf[nCur]==' ' || buf[nCur]=='\n')
	{
		buf[nCur] = '\0';
		nCur--;
	}
	while(buf[nCur]!=' ' && nCur>=0 && buf[nCur]!='\t')
	{
		nCur--;
	}
	int nTextLen = strlen(buf) - nCur;
	if(nTextLen > (int)sizeof(text))
		return 2;
	if(!strcmp(buf+nCur+1, "offline") || !strcmp(buf+nCur+1, "unauthorized"))
		return 0;
	else
		return 1;
}

int InterfaceFull::systemdroid(const char * cmdstring)
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

int InterfaceFull::execstream(const char *cmdstring, char *buf, int size)
{
	FILE* stream;
	stream = popen(cmdstring, "r");
	if(NULL == stream)
	{
		LogFile::write_sys_log("execute adb command failed!");
		strcpy(buf, "failed");
		return 1;
	}
	else
	{
		while(NULL != fgets(buf, size, stream))
		{
		}
		pclose(stream);
		return 0;
	}
}
