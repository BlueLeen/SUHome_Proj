#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include "ApkInstall.h"

#define UEVENT_BUFFER_SIZE 2048
#define MINSIZE 100
#define ROWSIZE 200
#define MAXSIZE 1024
#define APK_DIR_NAME "apkdir"
#define ADB_ADB_NAME "adb"
#define APP_ROOT_PATH "/system/strongunion/"

// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount()
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) != 0)
		return 0;
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

int phone_is_online(char* buf)
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
	if(nTextLen > sizeof(text))
		return 2;
	if(!strcmp(buf+nCur+1, "offline") || !strcmp(buf+nCur+1, "unauthorized"))
		return 0;
	else
		return 1;
}

void write_sys_log(char* szWriteString)
{
	char szLogPath[MAXSIZE]={0};
	FILE* fp;
	char szTime[50];
	time_t ti;
	//get_current_path(szFilePath, MAXSIZE);
	sprintf(szLogPath, "%s%s", APP_ROOT_PATH, "log");
	fp = fopen(szLogPath, "a+");
	time(&ti);
	struct tm* ptm = localtime(&ti);
	sprintf(szTime, "Now time is:%d/%d/%d %d:%d:%d", ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(fp,"%s>>----- %s\n", szTime, szWriteString);
	fclose(fp);
}

void write_sys_log_int(int nWrite)
{
	char szString[10]={0};
	sprintf(szString, "%d", nWrite);
	return write_sys_log(szString);
}

int systemdroid(const char * cmdstring) {
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

int install_android_apk(char* szApk)
{
	char shellComm[MAXSIZE] = { 0 };
	char szPath[MAXSIZE] = { 0 };
	char szApkPath[MAXSIZE] = { 0 };
	sprintf(szPath, "%s", APP_ROOT_PATH);
	sprintf(szApkPath, "%s%s/%s", szPath, APK_DIR_NAME, szApk);
	sprintf(shellComm, "%s kill-server", ADB_ADB_NAME);

	char shellCommDevice[MAXSIZE] = { 0 };
	char szFile[MAXSIZE] = { 0 };
	sprintf(szFile, "%s%s", APP_ROOT_PATH, "text");
	sprintf(shellCommDevice, "%s devices > %s", ADB_ADB_NAME, szFile);
	write_sys_log(shellComm);
	int num1 = systemdroid(shellComm);
	write_sys_log_int(num1);
	while(systemdroid(shellComm)==0)
	{
		FILE *fpin;
		char line[ROWSIZE] = { 0 };
		char line_last[ROWSIZE] = { 0 };
		int num2 = systemdroid(shellCommDevice);
		write_sys_log_int(num2);
		write_sys_log(shellCommDevice);
		{
			fpin = fopen(szFile, "r");
			while(fgets(line, ROWSIZE, fpin) != NULL)
			{
				write_sys_log(line);
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
	write_sys_log(shellComm);
	systemdroid(shellComm);
	systemdroid("exit");
	return 0;
}
