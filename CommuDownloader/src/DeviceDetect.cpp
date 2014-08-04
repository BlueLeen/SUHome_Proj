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
#include <stdlib.h>
#include <sys/statfs.h>
//#include <sys/stat.h>
#include "PlugEvent.h"
#include "LogFile.h"
#include "SocketSeal.h"
#include "InterfaceFull.h"
#include "CLock.h"

//#define APP_ROOT_PATH "/system/strongunion/"
#define DEV_USB_DEV 							"/proc/bus/usb/devices"
#define DEV_USB_FILE 							"usbinfo"
#define DEV_EXTERNAL_SDCARD_PATH				"/mnt/external_sd"
//#define DEV_USB_FILE "/home/leen/Desktop/new"
#define ROWSIZE 400
#define MAXSIZE 1024

#define SOCKET_CODE_PHONEPLUGIN  	 			 101
#define SOCKET_CODE_PHONEPULLOUT     			 102
#define SOCKET_CODE_BOXPROBLEM  	 			 103
#define SOCKET_CODE_BOXWELL  	     			 104
#define SOCKET_CODE_PHONENOTRECOGNIZED  	     105
#define SOCKET_CODE_PHONEOPENUSBDEBUG  	     	 107
#define SOCKET_CODE_UPANPLUGIN  	     	 	 108
#define SOCKET_CODE_UPANBEREADY 	     	 	 109
#define SOCKET_CODE_UPANPULLOUT 	     	 	 110
#define SOCKET_CODE_SDCARDPLUGIN  	     	 	 111
#define SOCKET_CODE_SDCARDBEREADY 	     	 	 112
#define SOCKET_CODE_SDCARDPULLOUT 	     	 	 113

extern void send_all_client_packs(char* buf, int size);
extern int grap_pack(void* buf, int nCode, const char* content);
extern int execstream(const char *cmdstring, char *buf, int size);
extern void trim(char* str, char trimstr=' ');
typedef struct _PHONEVID
{
	_PHONEVID():count(0)
	{
		memset(phoneVid, 0, sizeof(phoneVid));
	}
	unsigned int phoneVid[MAXSIZE];
	int count;
}PHONEVID;
extern PHONEVID global_phone_vidArr;
//extern bool is_file_exist(const char *path);

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

int binary_search(unsigned int a[], int size, int value)
{
	int beg=0;
	int end=size-1;
	while(beg <= end)
	{
		int pos = (beg+end)/2;
		if(value < a[pos])
			end = pos-1;
		else if(value > a[pos])
			beg = pos+1;
		else
			return pos;
	}
	return -1;
}

int GetStorageInfo(char * MountPoint,  //SD卡随便一个分区
		int *Capacity,  //  想要获取的空间大小
		int type) //获取什么类型的空间
		{
	struct statfs statFS; //系统stat的结构体
	unsigned long usedBytes = 0;
	unsigned long freeBytes = 0;
	unsigned long totalBytes = 0;
	unsigned long endSpace = 0;

	if (statfs(MountPoint, &statFS) == -1) {   //获取分区的状态
		printf("statfs failed for path->[%s]\n", MountPoint);
		return (-1);
	}

	totalBytes = (unsigned long) statFS.f_blocks * (unsigned long) statFS.f_frsize; //详细的分区总容量， 以字节为单位
	freeBytes = (unsigned long) statFS.f_bfree * (unsigned long) statFS.f_frsize; //详细的剩余空间容量，以字节为单位
	usedBytes = (unsigned long)(totalBytes - freeBytes); //详细的使用空间容量，以字节为单位

	switch (type) {
	case 1:
		endSpace = totalBytes / 1048576; //以MB为单位的总容量
		break;

	case 2:
		endSpace = usedBytes / 1048576; //以MB为单位的使用空间
		break;

	case 3:
		endSpace = freeBytes / 1048576; //以MB为单位的剩余空间
		break;

	default:
		return (-1);
	}
	*Capacity = endSpace; //这个不用说了吧
	return 0;
}

int GetStorageInfo(char *TotalCapacity, char *FreeCapacity, char* path)
{
	char szCmdString[ROWSIZE] = { 0 };
	char szCmdResult[ROWSIZE] = { 0 };
	sprintf(szCmdString, "df %s", path);
	execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
//	strcpy(szCmdResult, "Filesystem               Size     Used     Free   Blksize\n\
///mnt/external_sd         7.2G   449.0M     6.8G   4096");
	trim(szCmdResult);
	char* tmp = strstr(szCmdResult, path);
	if(tmp != NULL)
	{
		strncpy(szCmdResult, tmp, sizeof(szCmdResult));
		tmp = strstr(szCmdResult, " ");
		int i=0;
		while(i < 3)
		{
			if(tmp != NULL)
			{
				i++;
				strncpy(szCmdResult, tmp, sizeof(szCmdResult));
				trim(szCmdResult);
				tmp = strstr(szCmdResult, " ");
				if(i == 1)
					strncpy(TotalCapacity, szCmdResult, tmp-szCmdResult);
				else if(i == 3)
					strncpy(FreeCapacity, szCmdResult, tmp-szCmdResult);
			}
			else
				break;
		}
	}
//#ifdef DEBUG
//	LogFile::write_sys_log(szCmdResult);
//#endif

	return 0;
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
bool DeviceDetect::m_bPlugedDevice = false;

//typedef unsigned long int pthread_t;
unsigned long DeviceDetect::m_nUsbFileSize = 0;
unsigned long DeviceDetect::m_lastAddTime = 0;
unsigned long DeviceDetect::m_lastChangeTime = 0;
unsigned long DeviceDetect::m_lastRemoveTime = 0;
char DeviceDetect::m_szAddTextPath[PATH_MAX] = { 0 };
char DeviceDetect::m_szSdAddTextPath[PATH_MAX] = { 0 };

unsigned long DeviceDetect::m_lastSdAddTime = 0;
unsigned long DeviceDetect::m_lastSdChangeTime = 0;
unsigned long DeviceDetect::m_lastSdRemoveTime = 0;


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
#ifdef DEBUG
		LogFile::write_sys_log("use file:/proc/bus/usb/devices!");
#endif
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

	DeviceDetect::m_lastSdAddTime = GetTickCount();
	DeviceDetect::m_lastSdChangeTime = GetTickCount();
	DeviceDetect::m_lastSdRemoveTime = GetTickCount();
//	if(is_file_exist(DEV_USB_DEV))
//		DeviceDetect::m_bGetDeviceFileMethod = true;
//	else
//		DeviceDetect::m_bGetDeviceFileMethod = false;
	DeviceDetect::m_bGetDeviceFileMethod = false;
	DeviceDetect::m_bPlugedDevice = false;
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
			//LogFile::write_sys_log(buf);
			int nState = plug_opp_dev(buf, strlen(buf));
			if(strstr(buf, "sdmmc") != NULL)
			{
				nState += 3;
			}
			char* tmp = strstr(buf, "usb");
			if(nState == 1 && tmp!=NULL)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastAddTime > 3000)
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
			else if(nState == 2 && tmp!=NULL)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastChangeTime > 3000)
				{
					m_lastChangeTime = curTime;
//					pthread_create(&pt_recv, NULL, pthread_func_call, (void*)nState);
//#ifdef DEBUG
//					LogFile::write_sys_log(buf);
//#endif
				}
			}
			else if(nState == 3 && tmp!=NULL)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastRemoveTime > 3000)
				{
					m_lastRemoveTime = curTime;
					pthread_create(&pt_recv, NULL, pthread_func_call, (void*)nState);
#ifdef DEBUG
					LogFile::write_sys_log(buf);
#endif
				}
			}
			else if(nState == 4)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastSdAddTime > 3000)
				{
					m_lastSdAddTime = curTime;
					char* szText = strstr(buf, "@");
					if(szText != NULL)
						sprintf(m_szSdAddTextPath, "/sys%s", szText+1);
					else
						memset(m_szSdAddTextPath, 0, sizeof(m_szSdAddTextPath));
					pthread_create(&pt_recv, NULL, pthread_func_call, (void*)nState);
#ifdef DEBUG
					LogFile::write_sys_log(buf);
#endif
				}
			}
			else if(nState == 5)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastSdChangeTime > 3000)
				{
					m_lastSdChangeTime = curTime;
					pthread_create(&pt_recv, NULL, pthread_func_call, (void*)nState);
#ifdef DEBUG
					LogFile::write_sys_log(buf);
#endif
				}
			}
			else if(nState == 6)
			{
				unsigned long curTime = GetTickCount();
				if(curTime - m_lastSdRemoveTime > 3000)
				{
					m_lastSdRemoveTime = curTime;
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

#ifdef DEBUG
	LogFile::write_sys_log("******capture the device state start!.******");
#endif

	static bool bPhone = false;
	//DeviceInfo* pDev = (DeviceInfo*)ptr;
	unsigned long nState = (unsigned long)ptr;
#ifdef DEBUG
	char szLog[ROWSIZE] = { 0 };
	sprintf(szLog, "the state number of device is:%d!", nState);
	LogFile::write_sys_log(szLog);
#endif
	if(nState == 1)
	{
		m_bPlugedDevice = true;
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
		snprintf(content, sizeof(content), "%s_%s_%s %s_%s", devinfo.m_szVid, devinfo.m_szPid, devinfo.m_szManFac,
				devinfo.m_szProduct, devinfo.m_szImei);
#ifdef DEBUG
		sprintf(szLog, "get:%s:end", content);
		LogFile::write_sys_log(szLog);
#endif
//		*(int*)(buf+4) = htonl(SOCKET_CODE_PHONEPLUGIN);
//		int ctnlen = strlen(content);
//		memcpy(buf+8, content, ctnlen);
//		*(int*)buf = htonl(ctnlen + 8);
//		send_all_client_packs(buf, ctnlen+8);
		char sztmpVid[20] = { 0 };
		snprintf(sztmpVid, sizeof(sztmpVid), "0X%s", devinfo.m_szVid);
		unsigned int tmpVid = strtol(sztmpVid, NULL, 16);
#ifdef DEBUG
		sprintf(szLog, "the device vid:ten-%d,sixteen-%x", tmpVid, tmpVid);
		LogFile::write_sys_log(szLog);
#endif
//		if(strstr(devinfo.m_szManFac, "USB") || strstr(devinfo.m_szProduct, "USB") ||
//				strstr(devinfo.m_szManFac, "usb") || strstr(devinfo.m_szProduct, "usb"))
		if(binary_search(global_phone_vidArr.phoneVid, global_phone_vidArr.count, tmpVid) < 0)
		{
			bPhone = false;
#ifdef DEBUG
			LogFile::write_sys_log("The device is not android phone in standard phone manufactory!");
#endif
			int nLen = grap_pack(buf, SOCKET_CODE_UPANPLUGIN, content);
			send_all_client_packs(buf, nLen);
			//DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);
			sleep(3);
			nLen = grap_pack(buf, SOCKET_CODE_UPANBEREADY, "1");
			send_all_client_packs(buf, nLen);
		}
		else
		{
			int nLen = 0;
//			int nLen = grap_pack(buf, SOCKET_CODE_PHONEPLUGIN, content);
//			send_all_client_packs(buf, nLen);
#ifdef DEBUG
			LogFile::write_sys_log("usb plug into device!");
#endif
			//DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);

#ifdef DEBUG
			LogFile::write_sys_log("try to open usb debug mode.");
#endif
			bool bDebug = InterfaceFull::open_android_usbdebug();
#ifdef DEBUG
			sprintf(szLog, "the result of open usb debug mode is:%s", bDebug==true?"true":"false");
			LogFile::write_sys_log(szLog);
#endif
			if(bDebug)
			{
				pthread_t pt_detect = 0;
				pthread_create(&pt_detect, NULL, pthread_func_detect, NULL);

				int nLen = grap_pack(buf, SOCKET_CODE_PHONEPLUGIN, content);
				send_all_client_packs(buf, nLen);
				//nLen = grap_pack(buf, SOCKET_CODE_PHONEOPENUSBDEBUG, bDebug==true?"1":"2");
				bPhone = true;
				nLen = grap_pack(buf, SOCKET_CODE_PHONEOPENUSBDEBUG, "1");
				send_all_client_packs(buf, nLen);
#ifdef DEBUG
				LogFile::write_sys_log("open the android phone usb debug mode success.");
#endif
			}
			else if(strstr(buf, "Android") != NULL)
			{
				nLen = grap_pack(buf, SOCKET_CODE_PHONEPLUGIN, content);
				send_all_client_packs(buf, nLen);
				bPhone = true;
				nLen = grap_pack(buf, SOCKET_CODE_PHONEOPENUSBDEBUG, "2");
				send_all_client_packs(buf, nLen);
#ifdef DEBUG
				LogFile::write_sys_log("open the android phone usb debug mode fail, can't recognize the phone.");
#endif
			}
			else
			{
				bPhone = false;
				nLen = grap_pack(buf, SOCKET_CODE_UPANPLUGIN, content);
				send_all_client_packs(buf, nLen);
				sleep(1);
				nLen = grap_pack(buf, SOCKET_CODE_UPANBEREADY, "1");
				send_all_client_packs(buf, nLen);
#ifdef DEBUG
				LogFile::write_sys_log("open device usb debug mode fail, it may be UPan.");
#endif
			}
		}
	}
	else if(nState == 2)
	{
		//DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);
	}
	else if(nState == 3)
	{
#ifdef DEBUG
		LogFile::write_sys_log("begin to remove the usb device.");
#endif
		m_bPlugedDevice = false;
		char buf[MAXSIZE] = { 0 };
//		*(int*)(buf+4) = htonl(SOCKET_CODE_PHONEPULLOUT);
//		*(char*)(buf+8) = ' ';
//		*(int*)buf = htonl(9);
//		send_all_client_packs(buf, 9);
		if(bPhone)
		{
			bPhone = false;
			int nLen = grap_pack(buf, SOCKET_CODE_PHONEPULLOUT, NULL);
			send_all_client_packs(buf, nLen);
		}
		else
		{
			int nLen = grap_pack(buf, SOCKET_CODE_UPANPULLOUT, NULL);
			send_all_client_packs(buf, nLen);
		}
		DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);
	}
	else if(nState == 4)
	{
		//DeviceInfo devinfo;
		char buf[MAXSIZE] = { 0 };
//#ifdef DEBUG
//		sprintf(szLog, "SDCard plug in.Path is:%s", m_szSdAddTextPath);
//		LogFile::write_sys_log(szLog);
//#endif
		//devinfo.get_dev_info(buf, m_szSdAddTextPath);
		char content[ROWSIZE] = { 0 };
		memset(buf, 0, sizeof(buf));
//		int TotalStorage = 0;
//		int FreeStorage = 0;
//		GetStorageInfo(m_szSdAddTextPath, &TotalStorage, 1);
//		GetStorageInfo(m_szSdAddTextPath, &FreeStorage, 3);
		//char szTotalStorage[10] = { 0 };
		//char szFreeStorage[10] = { 0 };
		//GetStorageInfo(szTotalStorage, szFreeStorage);
		//sprintf(content, "%s_%s", szTotalStorage, szFreeStorage);
		strncpy(content, DEV_EXTERNAL_SDCARD_PATH, sizeof(content));
//		sprintf(content, "%s_%s_%s %s_%s", devinfo.m_szVid, devinfo.m_szPid, devinfo.m_szManFac,
//				devinfo.m_szProduct, devinfo.m_szImei);
		int nLen = grap_pack(buf, SOCKET_CODE_SDCARDPLUGIN, content);
		send_all_client_packs(buf, nLen);
		sleep(3);
		nLen = grap_pack(buf, SOCKET_CODE_SDCARDBEREADY, "1");
		send_all_client_packs(buf, nLen);
	}
	else if(nState == 5)
	{

	}
	else if(nState == 6)
	{
		char buf[MAXSIZE] = { 0 };
		int nLen = grap_pack(buf, SOCKET_CODE_SDCARDPULLOUT, NULL);
		send_all_client_packs(buf, nLen);
	}

	//delete pDev;
#ifdef DEBUG
	LogFile::write_sys_log("******capture the device state over!.*******");
#endif


	return 0;
}

void DeviceDetect::send_usb_info()
{
	if(m_bPlugedDevice == true)
	{
		char buf[MAXSIZE] = { 0 };
		char content[ROWSIZE] = { 0 };
		DeviceInfo devinfo;
		devinfo.get_dev_info(buf, m_szAddTextPath);
		memset(buf, 0, sizeof(buf));
		sprintf(content, "%s_%s_%s %s_%s", devinfo.m_szVid, devinfo.m_szPid, devinfo.m_szManFac,
				devinfo.m_szProduct, devinfo.m_szImei);
#ifdef DEBUG
		char szLog[ROWSIZE] = { 0 };
		sprintf(szLog, "get:%s:end", content);
		LogFile::write_sys_log(szLog);
#endif
		if(strstr(devinfo.m_szManFac, "USB") || strstr(devinfo.m_szProduct, "USB") ||
				strstr(devinfo.m_szManFac, "usb") || strstr(devinfo.m_szProduct, "usb"))
		{
			int nLen = grap_pack(buf, SOCKET_CODE_UPANPLUGIN, content);
			send_all_client_packs(buf, nLen);
			DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);
		}
		else
		{
			int nLen = grap_pack(buf, SOCKET_CODE_PHONEPLUGIN, content);
			send_all_client_packs(buf, nLen);
			DeviceDetect::m_nUsbFileSize = get_file_size(DEV_USB_FILE);
			bool bDebug = InterfaceFull::open_android_usbdebug();
			nLen = grap_pack(buf, SOCKET_CODE_PHONEOPENUSBDEBUG, bDebug==true?"1":"2");
			send_all_client_packs(buf, nLen);
		}
	}
}

void* DeviceDetect::pthread_func_detect(void* ptr)
{
	char buf[MAXSIZE] = { 0 };
	while(true)
	{
		if(!m_bPlugedDevice)
			break;
		else if(!InterfaceFull::phone_state_off())
		{
			sleep(3);
			continue;
		}
		else
		{
			m_bPlugedDevice = false;
			int nLen = grap_pack(buf, SOCKET_CODE_PHONEPULLOUT, NULL);
			send_all_client_packs(buf, nLen);
			break;
		}
	}
	return 0;
}
