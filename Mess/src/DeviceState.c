#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include <pthread.h>
#include "DeviceState.h"

#define UEVENT_BUFFER_SIZE 2048
#define MAXSIZE 1024
#define MINSIZE 100

typedef void (*pGlobalCallBackFuc)(int code, void *pStruc);
extern pGlobalCallBackFuc global_callback_func;


// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount()
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) != 0)
		return 0;
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

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

extern void write_sys_log(char* szWriteString);
extern void write_sys_log_int(int nWrite);

//static void* pthread_func_install(void* pBuf)
//{
//	pthread_detach(pthread_self());
//	sleep(2);
//	install_android_apk("eserve.apk");
//	//mqrecv(MESSAGE_CENTER, 0);
//	write_sys_log("-------After install the apk File!-------");
//	return NULL;
//}

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

static void* pthread_func_plug(void* pBuf)
{
	pthread_detach(pthread_self());
	sleep(2);
	int hotplug_sock = init_hotplug_sock();

	write_sys_log_int(hotplug_sock);

	unsigned long lastPlugTime = GetTickCount();

	write_sys_log_int((int)lastPlugTime);

	while(1)
	{
	/* Netlink message buffer */
	int recvlen = 0;
	char buf[UEVENT_BUFFER_SIZE * 2] = {0};
	recvlen = recv(hotplug_sock, &buf, sizeof(buf), 0);
	if(recvlen > 0)
	{
		//char vendorId[10] = { 0 };
		if(plug_opp_dev(buf, strlen(buf)) == 1)
		{
			//printf("%s\n", buf);
			//if(android_vendor_id(vendorId, sizeof(vendorId)))
			{
				unsigned long curTime = GetTickCount();
				char szLog[MAXSIZE];
				sprintf(szLog, "--The real ct::%ld,\n", curTime);
				write_sys_log(szLog);

				if(curTime - lastPlugTime > 2000)
				{
					//pthread_t pt_recv = 0;
					//attemp to revise my file to see the effect
					sprintf(szLog, "CurrentTime::%ld,LastPlug::%ld,CurrentTime-LastPlug=%ld\n", curTime, lastPlugTime, curTime-lastPlugTime);
					write_sys_log(szLog);
//					pthread_create(&pt_recv, NULL, pthread_func_install, NULL);
//					lastPlugTime = curTime;
//					char msgbuf[MINSIZE];
//					sprintf(msgbuf, "i eserve.apk");
//					printf("sendMessage::%s\n",msgbuf);
					plgdevicestate *pPlg = malloc(sizeof(plgdevicestate));
					pPlg->devcode = 1;
					pPlg->state = 1;
					global_callback_func(1, pPlg);
				}
			}
		}
	}
	/* USB 设备的插拔会出现字符信息，通过比较不同的信息确定特定设备的插拔，在这添加比较代码 */

	}
	close(hotplug_sock);
	return 0;
}

unsigned long plug_dev_detect()
{
	pthread_t pt_plug = 0;
	pthread_create(&pt_plug, NULL, pthread_func_plug, NULL);
	return pt_plug;
}
