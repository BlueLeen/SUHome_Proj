/*
 ============================================================================
 Name        : Detect.c
 Author      : leen
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
//#include <error.h>
//#include <errno.h>

#define UEVENT_BUFFER_SIZE 2048
#define DEV_USB_FILE "/proc/bus/usb/devices"
#define MINSIZE 100
#define ROWSIZE 200
#define MAXSIZE 1024
#define APK_DIR_NAME "ApkDir"
#define ADB_DIR_NAME "Adb"

static int init_hotplug_sock()
{
	const int buffersize = 1024;
	int ret;

	struct sockaddr_nl snl;
	bzero(&snl, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (s == -1)
	{
		perror("socket");
		return -1;
	}
	setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

	ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
	if (ret < 0)
	{
		perror("bind");
		close(s);
		return -1;
	}

	return s;
}

int plug_opp_dev(char* usb_message, int nLen)
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
	if(!strcmp(messFlag, "remove"))
	{
		return 2;//if remove the device,return 1
	}
	return 0;
}

int android_vendor_id(char* vendorId, int nlen)
{
	FILE *fp;
	char szRow[ROWSIZE] = {0};
	int bJump = 0;
	if((fp = fopen(DEV_USB_FILE, "r")) == NULL)
	{
		printf("Can not open file:%s!\n", DEV_USB_FILE);
	}
	if(nlen < 4)
		printf("Can not get the vendor id because of short length\n");
	while(fgets(szRow, ROWSIZE, fp) != NULL )
	{
		if(szRow[0]=='\n' && bJump)
		{
			break;
		}
		else if(szRow[0] == 'P')
		{
			strncpy(vendorId, szRow+11, 4);
		}
		else if(szRow[0] == 'S')
		{
			char szManufacFlag[10] = { 0 };
			char szProductFlag[10] = { 0 };
			if(szRow[4] == 'M')
			{
				char *szManufac = szRow+4;
				int nManufacLen = -1;
				int j = 0;
				while(szManufac++)
				{
					if(*szManufac!='=' && nManufacLen<0)
						continue;
					else if(*szManufac == '=')
					{
						++nManufacLen;
						continue;
					}
					if(*szManufac!=' ' && *szManufac!='\n')
					{
						++nManufacLen;
						szManufacFlag[j++] = *szManufac;
					}
					else
						break;
				}
			}
			else if(szRow[4] == 'P')
			{
				char *szProduc = szRow + 4;
				int nProducLen = -1;
				int j = 0;
				while(szProduc++)
				{
					if(*szProduc!='=' && nProducLen<0)
						continue;
					else if(*szProduc == '=')
					{
						++nProducLen;
						continue;
					}
					if(*szProduc!=' ' && *szProduc!='\n')
					{
						++nProducLen;
						szProductFlag[j++] = *szProduc;
					}
					else
						break;
				}
			}
//			if(!strcmp(szManufacFlag, "") || !strcmp(szProductFlag, ""))
//				printf("First--%s::%s--End\n", szManufacFlag, szProductFlag);
			if(!strcmp(szManufacFlag, "Android") || !strcmp(szProductFlag, "Android"))
			{
				bJump = 1;
			}
		}
	}
	fclose(fp);
	if(bJump)
		return 1;
	return 0;
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

void systemDown(const char * cmdstring) {
	pid_t pid;
	int status;
	if (cmdstring == NULL) {
//		return (1); //如果cmdstring为空，返回非零值，一般为1
		return;
	}

	if ((pid = fork()) < 0) {
		status = -1; //fork失败，返回-1
	} else if (pid == 0) {
		execl("/bin/sh", "sh", "-c", cmdstring, (char *) 0);
		_exit(127); // exec执行失败返回127，注意exec只在失败时才返回现在的进程，成功的话现在的进程就不存在啦~~
	} else //父进程
	{
//		while (waitpid(pid, &status, 0) < 0) {
//			if (errno != EINTR) {
//				status = -1; //如果waitpid被信号中断，则返回-1
//				break;
//			}
//		}
	}

//	return status; //如果waitpid成功，则返回子进程的返回状态
}


int install_android_apk(char* szApk)
{
	char shellComm[MAXSIZE] = { 0 };
	char szPath[MAXSIZE] = { 0 };
	char szApkPath[MAXSIZE] = { 0 };
	char szAdbPath[MAXSIZE] = { 0 };
	get_current_path(szPath, MAXSIZE);
	sprintf(szApkPath, "%s%s/%s", szPath, APK_DIR_NAME, szApk);
	sprintf(szAdbPath, "%s%s/%s", szPath, ADB_DIR_NAME, "adb");
//	sprintf(shellComm, "%s remount", szAdbPath);
//	systemDown(shellComm);
	sprintf(shellComm, "%s install %s", szAdbPath, szApkPath);
	write_sys_log(shellComm);
	systemDown(shellComm);
	system("exit");
	return 0;
}

int listDir(char *path, char** fileArr)
{
	int 			 nCount = 0;
	DIR              *pDir ;
	struct dirent    *ent  ;
	char              childpath[512];

	pDir=opendir(path);
	memset(childpath,0,sizeof(childpath));

	while((ent=readdir(pDir))!=NULL)
	{
		if(ent->d_type & DT_DIR)
		{
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
					continue;
			sprintf(childpath,"%s/%s",path,ent->d_name);
			printf("path:%s/n",childpath);
			//listDir(childpath);
		}
		else
		{
			//cout<<ent->d_name<<endl;
			strcpy(*fileArr, ent->d_name);
			//printf("Addr:%x\n", (*fileArr));
			//printf("String:%s\n", *fileArr);
			fileArr++;
			nCount++;
		}
	}
	return nCount;
}

int listDirApk(char** fileArr)
{
	char szPath[MAXSIZE] = { 0 };
	char szApkRootPath[MAXSIZE] = { 0 };
	get_current_path(szPath, MAXSIZE);
	sprintf(szApkRootPath, "%s%s", szPath, APK_DIR_NAME);
	return listDir(szApkRootPath, fileArr);
}

int install_all_android_apk(char** szApkArr, int nCount)
{
	int i;
	for(i=0; i<nCount; i++)
		install_android_apk(szApkArr[i]);
	return 0;
}

void write_sys_log(char* szWriteString)
{
	char szFilePath[MAXSIZE]={0};
	char szLogPath[MAXSIZE]={0};
	FILE* fp;
	char szTime[50];
	time_t ti;
	get_current_path(szFilePath, MAXSIZE);
	sprintf(szLogPath, "%s%s", szFilePath, "log");
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

// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount()
{
    struct timespec ts;

     clock_gettime(CLOCK_MONOTONIC, &ts);

     return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

char** AllocApkFileName()
{
	int i;
	char** szArr = malloc(MINSIZE*sizeof(char*));
	char** szArrTmp = szArr;
	for(i=0; i<MINSIZE; i++)
	{
		*szArrTmp = malloc(MINSIZE*sizeof(char));
		szArrTmp++;
	}
	return szArr;
}

void FreeApkFileName(char** szArr)
{
	int i;
	for(i=0; i<MINSIZE; i++)
	{
		free(*szArr);
	}
	free(szArr);
}

//install Apk to Android Phone
static void* pthread_func_install(void*);

int main(int argc, char* argv[]) {
	char** pApkArray;

	int hotplug_sock = init_hotplug_sock();

	write_sys_log_int(hotplug_sock);

	unsigned long lastPlugTime = GetTickCount();

	write_sys_log_int((int)lastPlugTime);

	pApkArray = AllocApkFileName();
	//printf("Addr:%lx\n", pApkArray);

	int nApkCount = listDirApk(pApkArray);

	void* pBuffer = malloc(2*sizeof(unsigned char*));
	*(unsigned char**)pBuffer = (unsigned char*)pApkArray;
	*((unsigned char**)pBuffer+1) = (unsigned char*)nApkCount;

	//printf("ConvertAddr:%lx\n", (long)*(unsigned char**)pBuffer);
	//printf("ConvertAddr:%lx\n", (long)*((unsigned char**)pBuffer+1));

	while(1)
	{
	/* Netlink message buffer */
	int recvlen = 0;
	char buf[UEVENT_BUFFER_SIZE * 2] = {0};
	recvlen = recv(hotplug_sock, &buf, sizeof(buf), 0);
	if(recvlen > 0)
	{
		char vendorId[10] = { 0 };
		if(plug_opp_dev(buf, strlen(buf)) == 1)
		{
			//printf("%s\n", buf);
			//if(android_vendor_id(vendorId, sizeof(vendorId)))
			{
				unsigned long curTime = GetTickCount();
//				char shellComm[MAXSIZE] = { 0 };
//				get_current_path(shellComm, MAXSIZE);
//				sprintf(shellComm, "%sHello %s", shellComm, vendorId);
//				system(shellComm);
				//printf("VendorID:%s\n", vendorId);
				//write_sys_log(buf);
				char szLog[MAXSIZE];
				sprintf(szLog, "--The real ct::%ld,\n", curTime);
				write_sys_log(szLog);

				if(curTime - lastPlugTime > 2000)
				{
					pthread_t pt_recv = 0;

					//attemp to revise my file to see the effect
					sprintf(szLog, "CurrentTime::%ld,LastPlug::%ld,CurrentTime-LastPlug=%ld\n", curTime, lastPlugTime, curTime-lastPlugTime);
					write_sys_log(szLog);
					//install_android_apk("FirstProj.apk");
					//install_all_android_apk(pApkArray, nApkCount);
					pthread_create(&pt_recv, NULL, pthread_func_install, pBuffer);
					lastPlugTime = curTime;
				}
			}
		}
	}

	/* USB 设备的插拔会出现字符信息，通过比较不同的信息确定特定设备的插拔，在这添加比较代码 */

	}
	close(hotplug_sock);
	FreeApkFileName(pApkArray);
	free(pBuffer);
	return 0;
}

static void* pthread_func_install(void* pBuf)
{
	pthread_detach(pthread_self());
	sleep(2);
	char** pApkArray = (char**)*(unsigned char**)pBuf;
	int nApkCount = (int)*((unsigned char**)pBuf+1);
	write_sys_log("-------Begin install Apk into the Android Phone!The count of apk File:-------");
	write_sys_log_int(nApkCount);
	install_all_android_apk(pApkArray, nApkCount);
	write_sys_log("-------After install the apk File!-------");
	return NULL;
}
