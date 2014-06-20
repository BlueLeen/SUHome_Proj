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
#include <limits.h>
#include "LogFile.h"

#define MAXSIZE 1024
#define ROWSIZE 200
//#define APK_DIR_NAME "apkdir"
#define ADB_ADB_NAME "adb"
//#define APP_ROOT_PATH "/system/strongunion/"
#define APK_TEMP_PATH  "/data/local/tmp/strongunion/tmp"

char* get_current_path();

InterfaceFull::InterfaceFull() {
	// TODO Auto-generated constructor stub

}

InterfaceFull::~InterfaceFull() {
	// TODO Auto-generated destructor stub
}

void InterfaceFull::open_android_usbdebug()
{
	//bool bExit = false;
	char shellComm[MAXSIZE] = { 0 };
	char shellCommDevice[MAXSIZE] = { 0 };
	char szAdbPath[PATH_MAX] = { 0 };
	char szInfo[MAXSIZE] = { 0 };
	char szFile[MAXSIZE] = { 0 };
	sprintf(szAdbPath, "%s%s", get_current_path(), ADB_ADB_NAME);
	sprintf(shellComm, "%s kill-server", szAdbPath);
	//sprintf(shellCommDevice, "%s devices", szAdbPath);
	sprintf(szFile, "%s/%s", APK_TEMP_PATH, "text");
	sprintf(shellCommDevice, "%s devices > %s", szAdbPath, szFile);
//	while(!bExit)
//	{
//		systemdroid(shellComm);
//		execstream(shellCommDevice, szInfo, sizeof(szInfo));
//		bExit = phone_is_online(szInfo);
//	}
	while(systemdroid(shellComm)==0)
	{
		FILE *fpin;
		char line[ROWSIZE] = { 0 };
		char line_last[ROWSIZE] = { 0 };
		execstream(shellCommDevice, szInfo, sizeof(szInfo));
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
}

int InterfaceFull::install_android_apk(char* szApk)
{
	char shellComm[MAXSIZE] = { 0 };
	//char szPath[MAXSIZE] = { 0 };
	char szApkPath[MAXSIZE] = { 0 };
	//sprintf(szPath, "%s", APP_ROOT_PATH);
	//sprintf(szApkPath, "%s%s/%s", szPath, APK_DIR_NAME, szApk);
	char szAdbPath[PATH_MAX] = { 0 };
	sprintf(szAdbPath, "%s%s", get_current_path(), ADB_ADB_NAME);
	strcpy(szApkPath, szApk);
//	sprintf(shellComm, "%s kill-server", ADB_ADB_NAME);
//
//	char shellCommDevice[MAXSIZE] = { 0 };
//	char szFile[MAXSIZE] = { 0 };
//	char szInfo[MAXSIZE] = { 0 };
//	sprintf(szFile, "%s%s", APP_ROOT_PATH, "text");
//	sprintf(shellCommDevice, "%s devices > %s", ADB_ADB_NAME, szFile);
//	LogFile::write_sys_log(shellComm, APP_ROOT_PATH);
//	execstream("adb wait-for-device", szInfo, sizeof(szInfo));
//	int num1 = execstream(shellComm, szInfo, sizeof(szInfo));
//	LogFile::write_sys_log(szInfo, APP_ROOT_PATH);
//	while(systemdroid(shellComm)==0)
//	{
//		FILE *fpin;
//		char line[ROWSIZE] = { 0 };
//		char line_last[ROWSIZE] = { 0 };
//		int num2 = execstream(shellCommDevice, szInfo, sizeof(szInfo));
//		LogFile::write_sys_log(szInfo, APP_ROOT_PATH);
//		LogFile::write_sys_log(shellCommDevice, APP_ROOT_PATH);
//		{
//			fpin = fopen(szFile, "r");
//			while(fgets(line, ROWSIZE, fpin) != NULL)
//			{
//				LogFile::write_sys_log(line, APP_ROOT_PATH);
//				if(!strcmp(line, "\n")) continue;
//				strcpy(line_last, line);
//			}
//			if(strcmp(line_last, "") && phone_is_online(line_last))
//			{
//				fclose(fpin);
//				break;
//			}
//			fclose(fpin);
//		}
//	}
	char szInfo[MAXSIZE] = { 0 };
	//sprintf(shellComm, "%s install -r %s", ADB_ADB_NAME, szApkPath);
	sprintf(shellComm, "%s install -r %s", szAdbPath, szApkPath);
	LogFile::write_sys_log(shellComm);
	int result = execstream(shellComm, szInfo, sizeof(szInfo));
	LogFile::write_sys_log(szInfo);
	//systemdroid("exit");
	return result;
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
