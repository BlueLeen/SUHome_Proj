/*
 * DeviceInfo.cpp
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#include "DeviceDetect.h"
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <limits.h>
//#include <sys/stat.h>
#include "PlugEvent.h"
#include "LogFile.h"
#include "SocketSeal.h"
#include "InterfaceFull.h"

//#define APP_ROOT_PATH "/system/strongunion/"
#define DEV_USB_DEV "/proc/bus/usb/devices"
#define DEV_USB_FILE "usbinfo"
//#define DEV_USB_FILE "/home/leen/Desktop/new"
#define ROWSIZE 200
#define MAXSIZE 1024

#define SOCKET_CODE_PHONEPLUGIN  	 			 101
#define SOCKET_CODE_PHONEPULLOUT     			 102
#define SOCKET_CODE_BOXPROBLEM  	 			 103
#define SOCKET_CODE_BOXWELL  	     			 104
#define SOCKET_CODE_PHONENOTRECOGNIZED  	     105
#define SOCKET_CODE_PHONEOPENUSBDEBUG  	     	 107

extern void send_all_client_packs(char* buf, int size);
extern int grap_pack(void* buf, int nCode, const char* content);

// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount()
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) != 0)
		return 0;
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

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

char* get_current_path()
{
	static char szPath[PATH_MAX] = { 0 };
	int cnt = readlink("/proc/self/exe", szPath, PATH_MAX);
	if(cnt<0 || cnt>=PATH_MAX)
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
	return szPath;
}

char* get_temp_path()
{
	static char szPath[ROWSIZE] = { 0 };
	sprintf(szPath, "/data/local/tmp/");
	return szPath;
}

//unsigned long get_file_size(const char *path)
//{
//    unsigned long filesize = -1;
//    struct stat statbuff;
//    if(stat(path, &statbuff) < 0)
//    {
//    	return filesize;
//    }
//    else
//    {
//    	filesize = statbuff.st_size;
//    }
//    return filesize;
//}

bool DeviceDetect::m_bGetDeviceFileMethod = false;

//typedef unsigned long int pthread_t;
unsigned long DeviceDetect::m_nUsbFileSize = 0;
unsigned long DeviceDetect::m_lastAddTime = 0;
unsigned long DeviceDetect::m_lastChangeTime = 0;
unsigned long DeviceDetect::m_lastRemoveTime = 0;
char DeviceDetect::m_szAddTextPath[PATH_MAX] = { 0 };


DeviceDetect::DeviceDetect() {
	// TODO Auto-generated constructor stub

}

DeviceDetect::~DeviceDetect() {
	// TODO Auto-generated destructor stub
}

unsigned long DeviceDetect::get_file_size(const char *path)
{
	//char szPath[ROWSIZE] = { 0 };
	//get_current_path(szPath, sizeof(szPath));
	if(m_bGetDeviceFileMethod)
	{
		char szcmd[ROWSIZE] = { 0 };
		char szTmpPath[PATH_MAX] = { 0 };
		char* szPath = get_temp_path();
		sprintf(szTmpPath, "%s%s", szPath, DEV_USB_FILE);
		sprintf(szcmd, "cat %s > %s", DEV_USB_DEV, szTmpPath);
		systemdroid(szcmd);
	    unsigned long filesize = -1;
	    FILE *fp;
	    fp = fopen(szTmpPath, "r");
	    if(fp == NULL)
	        return filesize;
	    fseek(fp, 0L, SEEK_END);
	    filesize = ftell(fp);
	    fclose(fp);
	    return filesize;
	}
	else
		return 0;
}

void DeviceDetect::read_file_pos(char* buf, const char *path, long int pos)
{
	char szcmd[ROWSIZE] = { 0 };
	sprintf(szcmd, "cat %s > %s", DEV_USB_DEV, path);
	systemdroid(szcmd);
	long int filesize = -1;
    FILE *fp;
    fp = fopen(path, "r");
    if(fp == NULL)
        return ;
    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    if(filesize == pos)
    {
    	strcpy(buf, "\0");
    	fclose(fp);
    }
    fseek(fp, pos, SEEK_SET);
    fread(buf, filesize-pos, 1, fp);
#ifdef DEBUG
    LogFile::write_sys_log(buf);
#endif
    fclose(fp);
}

void DeviceDetect::plug_dev_detect() {
	pthread_t pt_plug = 0;
	DeviceDetect::m_lastAddTime = GetTickCount();
	DeviceDetect::m_lastChangeTime = GetTickCount();
	DeviceDetect::m_lastRemoveTime = GetTickCount();
	pthread_create(&pt_plug, NULL, pthread_func_plug, NULL);
}

void* DeviceDetect::pthread_func_plug(void* ptr)
{
	pthread_detach((unsigned long int)pthread_self());

	DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);

	PlugEvent event;

	while(1)
	{
		/* Netlink message buffer */
		int recvlen = 0;
		char buf[UEVENT_BUFFER_SIZE] = {0};
		recvlen = event.recv_hotplug_sock(buf, sizeof(buf));
		if(recvlen > 0)
		{
			pthread_t pt_recv = 0;
			//DeviceInfo* pDev = new DeviceInfo();
			//pDev->m_nCode = plug_opp_dev(buf, strlen(buf));
			//pDev->m_nState = plug_opp_dev(buf, strlen(buf));
			int nState = plug_opp_dev(buf, strlen(buf));
			if(nState == 1)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastAddTime > 2000)
				{
					m_lastAddTime = curTime;
					char* szText = strstr(buf, "@");
					if(szText != NULL)
						sprintf(m_szAddTextPath, "/sys%s", szText+1);
					else
						memset(m_szAddTextPath, 0, sizeof(m_szAddTextPath));
					pthread_create(&pt_recv, NULL, pthread_func_call, (void*)nState);
#ifdef DEBUG
					LogFile::write_sys_log(buf);
#endif
				}
			}
			else if(nState == 2)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastChangeTime > 2000)
				{
					m_lastChangeTime = curTime;
//					pthread_create(&pt_recv, NULL, pthread_func_call, (void*)nState);
//#ifdef DEBUG
//					LogFile::write_sys_log(buf);
//#endif
				}
			}
			else if(nState == 3)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastRemoveTime > 2000)
				{
					m_lastRemoveTime = curTime;
					pthread_create(&pt_recv, NULL, pthread_func_call, (void*)nState);
#ifdef DEBUG
					LogFile::write_sys_log(buf);
#endif
				}
			}
		}
	}
	return 0;
}

int DeviceDetect::plug_opp_dev(char* usb_message, int nLen)
{
	char messFlag[10] = { 0 };
	int i = 0;
	while(*usb_message && i<nLen)
	{
		if(*usb_message == '@')
		{
			break;
		}
		messFlag[i++] = *usb_message;
		++usb_message;
	}
	messFlag[i] = '\0';
	if(!strcmp(messFlag, "add"))
	{
		return 1;//if plug the device,return 1
	}
	else if(!strcmp(messFlag, "change"))
	{
		return 2;
	}
	else if(!strcmp(messFlag, "remove"))
	{
		return 3;//if remove the device,return 1
	}
	return 0;
}

void DeviceDetect::plug_opp_dev(string& strMessage, DeviceInfo* pDev)
{
	string::size_type pos = strMessage.find('@');
	string szCode = strMessage.substr(0, pos);
	if(!szCode.compare("add"))
	{
		pDev->m_nCode = 1;
	}
	else if(!szCode.compare("change"))
	{
		pDev->m_nCode = 2;
	}
	else if(!szCode.compare("remove"))
	{
		pDev->m_nCode = 3;
	}
	//pos = strMessage.find("usb");
}

void* DeviceDetect::pthread_func_call(void* ptr)
{
	pthread_detach((unsigned long int)pthread_self());

	//DeviceInfo* pDev = (DeviceInfo*)ptr;
	unsigned long nState = (unsigned long)ptr;
	if(nState == 1)
	{
		DeviceInfo devinfo;
		//char szPath[ROWSIZE] = { 0 };
		char buf[MAXSIZE] = { 0 };
		//get_current_path(szPath, sizeof(szPath));
		if(m_bGetDeviceFileMethod)
		{
			char szTmpPath[PATH_MAX] = { 0 };
			char* szPath = get_temp_path();
			sprintf(szTmpPath, "%s%s", szPath, DEV_USB_FILE);
			read_file_pos(buf, szTmpPath, DeviceDetect::m_nUsbFileSize);
			devinfo.get_dev_info(buf);
		}
		else
		{
			devinfo.get_dev_info(buf, m_szAddTextPath);
		}
		//global_sock_srv.send_socket_packs()
		char content[ROWSIZE] = { 0 };
		memset(buf, 0, sizeof(buf));
		sprintf(content, "%s_%s_%s %s_%s", devinfo.m_szVid, devinfo.m_szPid, devinfo.m_szManFac,
				devinfo.m_szProduct, devinfo.m_szImei);
#ifdef DEBUG
		char szLog[ROWSIZE] = { 0 };
		sprintf(szLog, "get:%s:end", content);
		LogFile::write_sys_log(szLog);
#endif
//		*(int*)(buf+4) = htonl(SOCKET_CODE_PHONEPLUGIN);
//		int ctnlen = strlen(content);
//		memcpy(buf+8, content, ctnlen);
//		*(int*)buf = htonl(ctnlen + 8);
//		send_all_client_packs(buf, ctnlen+8);
		int nLen = grap_pack(buf, SOCKET_CODE_PHONEPLUGIN, content);
		send_all_client_packs(buf, nLen);
		DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);

		InterfaceFull::open_android_usbdebug();
		nLen = grap_pack(buf, SOCKET_CODE_PHONEOPENUSBDEBUG, "1");
		send_all_client_packs(buf, nLen);
	}
	else if(nState == 2)
	{
		//DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);
	}
	else if(nState == 3)
	{
		char buf[MAXSIZE] = { 0 };
//		*(int*)(buf+4) = htonl(SOCKET_CODE_PHONEPULLOUT);
//		*(char*)(buf+8) = ' ';
//		*(int*)buf = htonl(9);
//		send_all_client_packs(buf, 9);
		int nLen = grap_pack(buf, SOCKET_CODE_PHONEPULLOUT, NULL);
		send_all_client_packs(buf, nLen);
		DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);
	}

	//delete pDev;

	return 0;
}
