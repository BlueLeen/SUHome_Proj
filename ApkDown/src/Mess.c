#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>

#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

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

int mqsend(const char* name, const char* buf)
{
	mqd_t mqd;
	MsgType msg;
	mqd = mq_open(name, O_WRONLY);
	strcpy(msg.buf, buf);
	msg.len = strlen(msg.buf) + 1;
	int ret = mq_send(mqd, (char*)&msg, sizeof(MsgType), 0);
	mq_close(mqd);
	return ret;
}

void* mqrecv(const char* name, int isNonblock)
{
	int flags;
	mqd_t mqd;
	struct mq_attr attr;
	static void *buf;
	flags = O_RDONLY;
	if(isNonblock)
	{
		flags |= O_NONBLOCK;
	}
	mqd = mq_open(name, flags);
	mq_getattr(mqd, &attr);
	buf = malloc(attr.mq_msgsize);
	ssize_t len = mq_receive(mqd, buf, attr.mq_msgsize, 0);
	printf("read %ld bytes\n", (long)len);
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
