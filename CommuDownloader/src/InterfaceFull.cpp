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
#include "CLock.h"

#define MAXSIZE 1024
#define MINSIZE  100

//#define APK_DIR_NAME "apkdir"
#define ADB_ADB_NAME "adb"
//#define APP_ROOT_PATH "/system/strongunion/"
#define APK_TEMP_PATH  "/data/local/tmp/strongunion/tmp"
#define DEVICE_EMULATOR  "emulator"//"8fd35188"  //"emulator"

char* get_current_path();

SerialLine  InterfaceFull::m_Sl[USBCOUNT];
int InterfaceFull::m_nCount = 0;

InterfaceFull::InterfaceFull() {
	// TODO Auto-generated constructor stub

}

InterfaceFull::~InterfaceFull() {
	// TODO Auto-generated destructor stub
}

char* InterfaceFull::get_adb_path()
{
	static char szAdbPath[PATH_MAX] = { 0 };
	sprintf(szAdbPath, "%s%s", get_current_path(), ADB_ADB_NAME);
	return szAdbPath;
}

void InterfaceFull::detect_device()
{
	char szInfo[MAXSIZE] = { 0 };
	char shellCommDevice[MAXSIZE] = { 0 };
	sprintf(shellCommDevice, "%s devices", get_adb_path());
	//execstream(shellCommDevice, szInfo, sizeof(szInfo));

	int iNum=0;
	char szSer[USBCOUNT][ROWSIZE];
	FILE* stream;
	stream = popen(shellCommDevice, "r");
	if(NULL == stream)
	{
		LogFile::write_sys_log("execute adb command failed!");
		strcpy(szInfo, "failed");
	}
	else
	{
		int j=0;
		while(NULL != fgets(szInfo, sizeof(szInfo), stream))
		{
			if(j==0)
			{
				j++;
				continue;
			}
			strncpy(szSer[iNum++], szInfo, ROWSIZE);
		}
		pclose(stream);
	}
	m_nCount = 0;
	for(int i=0; i<iNum; i++)
	{
		char* szTmp = strstr(szSer[i], "\t");
		char* szEmulator = strstr(szSer[i], DEVICE_EMULATOR);
		if(szTmp!=NULL && szEmulator==NULL)
		{
			//char szSrl[ROWSIZE] = { 0 };
			memset(m_Sl[m_nCount].szSerial, 0, ROWSIZE);
			int size = szTmp-szSer[i];
			strncpy((char*)m_Sl[m_nCount].szSerial, szSer[i], size);
			m_Sl[m_nCount].szSerial[size] = '\0';
#ifdef DEBUG
			char szLog[MINSIZE] = { 0 };
			snprintf(szLog, sizeof(szLog), "device %d serialno:%s---", m_nCount, m_Sl[m_nCount].szSerial);
			LogFile::write_sys_log(szLog);
#endif
			m_nCount++;
		}
	}
}

bool InterfaceFull::open_android_usbdebug(char* szSerialno)
{
	static bool bExit = false;
	static int nCount = 0;
	char shellComm[MAXSIZE] = { 0 };
	char shellCommDevice[MAXSIZE] = { 0 };
	char shellCommState[MAXSIZE] = { 0 };
	char szInfo[MAXSIZE] = { 0 };
	char szFile[MAXSIZE] = { 0 };
	char* szAdbPath = get_adb_path();
	sprintf(shellComm, "%s kill-server", szAdbPath);
	sprintf(shellCommDevice, "%s devices", szAdbPath);
	sprintf(shellCommState, "%s -s %s get-state", szAdbPath, szSerialno);
	sprintf(szFile, "%s/%s", APK_TEMP_PATH, "text");
	//sprintf(shellCommDevice, "%s devices > %s", szAdbPath, szFile);
    for(int i=0; i<4; i++)
    {
    	bExit = phone_is_online(szInfo, shellCommState);
    	if(bExit)
    		break;
    	sleep(1);
    }
	while(!bExit && nCount<=4)
	{
		systemdroid(shellComm);
		execstream(shellCommDevice, szInfo, sizeof(szInfo));
		//bExit = phone_is_online(szInfo);
		bExit = phone_is_online(szInfo, shellCommState);
		nCount++;
		usleep(500);
	}
    return bExit;
}

bool InterfaceFull::open_android_usbdebug()
{
	int nCount = 0;
	m_nCount = 0;
#ifdef DEBUG
	char szLog[MINSIZE] = { 0 };
	snprintf(szLog, sizeof(szLog), "current device count:%d.", m_nCount);
	LogFile::write_sys_log(szLog);
#endif
	while(m_nCount <= 0 && nCount < 5)
	{
		detect_device();
		sleep(1);
		nCount++;
	}
	if(m_nCount==1 || m_nCount==0)
		return open_android_usbdebug(m_Sl[0].szSerial);
	else
		return true;
//    CLock lock;
//	static bool bExit = false;
//	static int nCount = 0;
//	char shellComm[MAXSIZE] = { 0 };
//	char shellCommDevice[MAXSIZE] = { 0 };
//	char shellCommState[MAXSIZE] = { 0 };
//	char szAdbPath[PATH_MAX] = { 0 };
//	char szInfo[MAXSIZE] = { 0 };
//	char szFile[MAXSIZE] = { 0 };
//	sprintf(szAdbPath, "%s%s", get_current_path(), ADB_ADB_NAME);
//	sprintf(shellComm, "%s kill-server", szAdbPath);
//	sprintf(shellCommDevice, "%s devices", szAdbPath);
//	sprintf(shellCommState, "%s get-state", szAdbPath);
//	sprintf(szFile, "%s/%s", APK_TEMP_PATH, "text");
//	//sprintf(shellCommDevice, "%s devices > %s", szAdbPath, szFile);
//    lock.Lock();
//    for(int i=0; i<5; i++)
//    {
//    	bExit = phone_is_online(szInfo, shellCommState);
//    	if(bExit)
//    		break;
//    	sleep(1);
//    }
//	while(!bExit && nCount<=6)
//	{
//		systemdroid(shellComm);
//		execstream(shellCommDevice, szInfo, sizeof(szInfo));
//		//bExit = phone_is_online(szInfo);
//		bExit = phone_is_online(szInfo, shellCommState);
//		nCount++;
//		usleep(500);
//	}
//    lock.Unlock();
//    return bExit;
}

bool InterfaceFull::phone_state_off(char* szSerialno)
{
	char shellCommState[MAXSIZE] = { 0 };
	char szInfo[MAXSIZE] = { 0 };
	char* szAdbPath = get_adb_path();
	sprintf(shellCommState, "%s -s %s get-state", szSerialno, szAdbPath);
	return phone_is_online(szInfo, shellCommState) ? false:true;
}

void InterfaceFull::phone_plug_out()
{
	m_nCount--;
}

bool InterfaceFull::phone_state_off()
{
	if(m_nCount == 1)
		return phone_state_off(m_Sl[0].szSerial);
	else
		return true;
//	char shellCommState[MAXSIZE] = { 0 };
//	char szAdbPath[PATH_MAX] = { 0 };
//	char szInfo[MAXSIZE] = { 0 };
//	sprintf(szAdbPath, "%s%s", get_current_path(), ADB_ADB_NAME);
//	sprintf(shellCommState, "%s get-state", szAdbPath);
//	return phone_is_online(szInfo, shellCommState) ? false:true;
}

int InterfaceFull::install_android_apk(char* szApk, char* szSerialno)
{
	char shellComm[MAXSIZE] = { 0 };
	char szApkPath[MAXSIZE] = { 0 };
	char* szAdbPath = get_adb_path();
	strcpy(szApkPath, szApk);

	char szInfo[MAXSIZE] = { 0 };
	sprintf(shellComm, "%s -s %s install -r %s", szAdbPath, szSerialno, szApkPath);
#ifdef DEBUG
	LogFile::write_sys_log(shellComm);
#endif
	int result = execstream(shellComm, szInfo, sizeof(szInfo));
#ifdef DEBUG
	LogFile::write_sys_log(szInfo);
#endif
	return result;
}

int InterfaceFull::install_android_apk(char* szApk)
{
	int result = 0;
	if(m_nCount == 1 )
		return install_android_apk(szApk, m_Sl[0].szSerial);
	else
		for(int i=0; i<m_nCount; i++)
		{
			result = install_android_apk(szApk, m_Sl[i].szSerial);
		}
	return result;
//	char shellComm[MAXSIZE] = { 0 };
//	//char szPath[MAXSIZE] = { 0 };
//	char szApkPath[MAXSIZE] = { 0 };
//	//sprintf(szPath, "%s", APP_ROOT_PATH);
//	//sprintf(szApkPath, "%s%s/%s", szPath, APK_DIR_NAME, szApk);
//	char szAdbPath[PATH_MAX] = { 0 };
//	sprintf(szAdbPath, "%s%s", get_current_path(), ADB_ADB_NAME);
//	strcpy(szApkPath, szApk);
////	sprintf(shellComm, "%s kill-server", ADB_ADB_NAME);
////
////	char shellCommDevice[MAXSIZE] = { 0 };
////	char szFile[MAXSIZE] = { 0 };
////	char szInfo[MAXSIZE] = { 0 };
////	sprintf(szFile, "%s%s", APP_ROOT_PATH, "text");
////	sprintf(shellCommDevice, "%s devices > %s", ADB_ADB_NAME, szFile);
////	LogFile::write_sys_log(shellComm, APP_ROOT_PATH);
////	execstream("adb wait-for-device", szInfo, sizeof(szInfo));
////	int num1 = execstream(shellComm, szInfo, sizeof(szInfo));
////	LogFile::write_sys_log(szInfo, APP_ROOT_PATH);
////	while(systemdroid(shellComm)==0)
////	{
////		FILE *fpin;
////		char line[ROWSIZE] = { 0 };
////		char line_last[ROWSIZE] = { 0 };
////		int num2 = execstream(shellCommDevice, szInfo, sizeof(szInfo));
////		LogFile::write_sys_log(szInfo, APP_ROOT_PATH);
////		LogFile::write_sys_log(shellCommDevice, APP_ROOT_PATH);
////		{
////			fpin = fopen(szFile, "r");
////			while(fgets(line, ROWSIZE, fpin) != NULL)
////			{
////				LogFile::write_sys_log(line, APP_ROOT_PATH);
////				if(!strcmp(line, "\n")) continue;
////				strcpy(line_last, line);
////			}
////			if(strcmp(line_last, "") && phone_is_online(line_last))
////			{
////				fclose(fpin);
////				break;
////			}
////			fclose(fpin);
////		}
////	}
//	char szInfo[MAXSIZE] = { 0 };
//	//sprintf(shellComm, "%s install -r %s", ADB_ADB_NAME, szApkPath);
//	sprintf(shellComm, "%s install -r %s", szAdbPath, szApkPath);
//#ifdef DEBUG
//	LogFile::write_sys_log(shellComm);
//#endif
//	int result = execstream(shellComm, szInfo, sizeof(szInfo));
//#ifdef DEBUG
//	LogFile::write_sys_log(szInfo);
//#endif
//	//systemdroid("exit");
//	return result;
}

bool InterfaceFull::phone_is_online(char* buf, char* cmd)
{
	char szInfo[ROWSIZE] = { 0 };
	execstream(cmd, szInfo, sizeof(szInfo));
	char* tmp = strchr(szInfo, '\n');
	if(tmp != NULL)
		*tmp = '\0';
#ifdef DEBUG
		char szLog[ROWSIZE] = { 0 };
		sprintf(szLog, "The phone's state is:%s", szInfo);
		LogFile::write_sys_log(szLog);
#endif
	if(!strcmp(szInfo, "device"))
		return true;
	else
		return false;
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
