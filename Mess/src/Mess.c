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

#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define MESSAGE_CENTER  "/MessCenter.7687"

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

int main(int argc, char *argv[]) {
	//puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	if(argc == 1)
		printf("Error Arguments!\n");
	else if(!strcmp(argv[1], "c"))
		mqcreate(MESSAGE_CENTER);
	else if(!strcmp(argv[1], "u"))
		mqunlink(MESSAGE_CENTER);
	return EXIT_SUCCESS;
}
