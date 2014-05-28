/*
 ============================================================================
 Name        : Mess.c
 Author      : leen
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include "InitOpp.h"
#include "DeviceState.h"

#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define MESSAGE_CENTER  "/MessCenter.7687"

#define MESSIZE     50   //定义buf大小

typedef struct _MsgType{
    int len;
    char buf[MESSIZE];
}MsgType;

void mqcreate(char* name)
{
	int flags;
	mqd_t mqd;
	flags = O_RDWR | O_CREAT | O_EXCL;
	mqd = mq_open(name, flags, FILE_MODE, NULL);
	if(mqd == -1)
		printf("Error num:%d\n", errno);
	mq_close(mqd);
}

void mqunlink(const char* name)
{
	if(mq_unlink(name) == -1)
		printf("Error num:%d\n", errno);
}

struct mq_attr mqgetattr(const char* name)
{
	mqd_t mqd;
	static struct mq_attr attr;
	mqd = mq_open(name, O_RDONLY);
	mq_getattr(mqd, &attr);
	mq_close(mqd);
	return attr;
}

int mqsend(const char* name, size_t len)
{
	mqd_t mqd;
	void *ptr;
	mqd = mq_open(name, O_WRONLY);
	ptr = malloc(sizeof(char)*len);
	int ret = mq_send(mqd, ptr, len, 0);
	mq_close(mqd);
	return ret;
}

void* mqrecv(const char* name, int isNonblock)
{
	int flags;
	mqd_t mqd;
	struct mq_attr attr;
	static void *buf;
	MsgType msg;
	flags = O_RDONLY;
	if(isNonblock)
	{
		flags |= O_NONBLOCK;
	}
	mqd = mq_open(name, flags);
	mq_getattr(mqd, &attr);
	ssize_t len = mq_receive(mqd, (char*)&msg, attr.mq_msgsize, 0);
	printf("read %ld bytes, the content is: %s\n", (long)len, msg.buf);
	buf = malloc(msg.len);
	strcpy(buf, msg.buf);
    if(mq_close(mqd) == -1)
    {
        perror("mq_close");
        exit(1);
    }
	if(len > 0)
		return buf;
	else
		return 0;
}

void mesoperate()
{
	char* msg = mqrecv(MESSAGE_CENTER, 0);
	if(msg[0] == 'i')
	{
		install_android_apk(msg+2);
	}
}

//void mqnotify(const char* name)
//{
//	int signo;
//	mqd_t mqd;
//	void *buff;
//	ssize_t n;
//	sigset_t newmask;
//	struct mq_attr attr;
//	struct sigevent sigev;
//	mqd = mq_open(name, O_RDONLY|O_NONBLOCK);
//	mq_getattr(mqd, &attr);
//	buff = malloc(attr.mq_msgsize);
//	sigemptyset(&newmask);
//	sigaddset(&newmask,SIGUSR);
//	sigprocmask(SIG_BLOCK, &newmask, NULL);
//	sigev.sigev_notify = SIGEV_SIGNAL;
//	sigev.sigev_signo = SIGUSR;
//	mq_notify(mqd, &sigev);
//	for(;;)
//	{
//		sigwait(&newmask, &signo);
//		if(signo == SIGUSR)
//		{
//			mq_notify(mqd, &sigev);
//			while(n == mq_receive(mqd, buff, attr.mq_msgsize,NULL) >= 0)
//			{
//
//			}
//			if(errno == EAGAIN)
//				printf("mq_receive error!\n");
//		}
//	}
//}

void GlobalCallBackFuc(int code, void *pStruc)
{
	if(code == 1)
	{
		plgdevicestate* result = (plgdevicestate*)pStruc;
		printf("Device code:%d,Device state:%d\n", result->devcode, result->state);
	}
}

int main(int argc, char *argv[]) {
////	//puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
////	if(argc == 1)
////		printf("Error Arguments!\n");
////	else if(!strcmp(argv[1], "c"))
////		mqcreate(MESSAGE_CENTER);
////	else if(!strcmp(argv[1], "u"))
////		mqunlink(MESSAGE_CENTER);
////	return EXIT_SUCCESS;
//
//	mqcreate(MESSAGE_CENTER);
//	while(1)
//	{
//		mesoperate();
//	}
	Init((void*)GlobalCallBackFuc);
	while(1)
	{
	}
	return EXIT_SUCCESS;
}
