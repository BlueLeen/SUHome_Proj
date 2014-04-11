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
//#include <error.h>
//#include <errno.h>

#define UEVENT_BUFFER_SIZE 2048
#define DEV_USB_FILE "/proc/bus/usb/devices"
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

int install_android_apk(char* szApk)
{
	char shellComm[MAXSIZE] = { 0 };
	char szPath[MAXSIZE] = { 0 };
	char szApkPath[MAXSIZE] = { 0 };
	char szAdbPath[MAXSIZE] = { 0 };
	get_current_path(szPath, MAXSIZE);
	sprintf(szApkPath, "%s%s/%s", szPath, APK_DIR_NAME, szApk);
	sprintf(szAdbPath, "%s%s/%s", szPath, ADB_DIR_NAME, "adb");
	sprintf(shellComm, "%s install %s", szAdbPath, szApkPath);
	system(shellComm);
	return 0;
}

void write_sys_log(char* szWriteString)
{
	char szFilePath[MAXSIZE];
	FILE* fp;
	char szTime[50];
	time_t ti;
	get_current_path(szFilePath, MAXSIZE);
	sprintf(szFilePath, "%s%s", szFilePath, "log");
	fp = fopen(szFilePath, "a+");
	time(&ti);
	struct tm* ptm = localtime(&ti);
	sprintf(szTime, "当前时间为:%d/%d/%d %d:%d:%d", ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(fp,"%s>>----- %s\n", szTime, szWriteString);
	fclose(fp);
}

// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount()
{
    struct timespec ts;

     clock_gettime(CLOCK_MONOTONIC, &ts);

     return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

int main(int argc, char* argv[]) {
//	//puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
//	struct sockaddr_nl client;
//	struct timeval tv;
//	int cpplive, rcvlen, ret;
//	fd_set fds;
//	int buffersize = 1024;
//	cpplive = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
//	memset(&client, 0, sizeof(client));
//	client.nl_family = AF_NETLINK;
//	client.nl_pid = getpid();
//	client.nl_groups = 1;
//	setsockopt(cpplive, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));
//	bind(cpplive, (struct sockaddr*)&client, sizeof(client));
//	while(1){
//		char buf[UEVENT_BUFFER_SIZE] = { 0 };
//		FD_ZERO(&fds);
//		FD_SET(cpplive, &fds);
//		tv.tv_sec = 0;
//		tv.tv_usec = 100 * 1000;
//		ret = select(cpplive+1, &fds, NULL, NULL, &tv);
//		if(ret < 0)
//			continue;
//		//if(!(ret > 0 && FD_ISSET(cpplive, &fds)))
//			//continue;
//		rcvlen = recv(cpplive, &buf, sizeof(buf), 0);
//		if(rcvlen > 0){
//			printf("%s\n", buf);
//		}
//	}
//	close(cpplive);
//	return EXIT_SUCCESS;
	int hotplug_sock = init_hotplug_sock();

	unsigned long lastPlugTime = GetTickCount();

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
			if(android_vendor_id(vendorId, sizeof(vendorId)))
			{
				unsigned long curTime = GetTickCount();
				char shellComm[MAXSIZE] = { 0 };
				get_current_path(shellComm, MAXSIZE);
				sprintf(shellComm, "%sHello %s", shellComm, vendorId);
				system(shellComm);
				//printf("VendorID:%s\n", vendorId);
				write_sys_log(buf);
				char szLog[MAXSIZE];
				sprintf(szLog, "--The real ct::%ld,\n", curTime);
				write_sys_log(szLog);

				if(curTime - lastPlugTime > 2000)
				{
					//attemp to revise my file to see the effect
					sprintf(szLog, "CurrentTime::%ld,LastPlug::%ld,CurrentTime-LastPlug=%ld\n", curTime, lastPlugTime, curTime-lastPlugTime);
					write_sys_log(szLog);
					//install_android_apk("FirstProj.apk");
					lastPlugTime = curTime;
				}
			}
		}
	}

	/* USB 设备的插拔会出现字符信息，通过比较不同的信息确定特定设备的插拔，在这添加比较代码 */

	}
	close(hotplug_sock);
	return 0;
}
