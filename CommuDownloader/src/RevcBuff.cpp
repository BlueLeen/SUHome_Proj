/*
 * RevcBuff.cpp
 *
 *  Created on: 2014年12月12日
 *      Author: su
 */

#include "RevcBuff.h"
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>


RevcBuff::RevcBuff():cursize(0)
{
	// TODO Auto-generated constructor stub
	memset(buffer, 0, sizeof(buffer));
}

RevcBuff::~RevcBuff() {
	// TODO Auto-generated destructor stub
}

int RevcBuff::putData(const char* buf, int len)
{
	while(cursize+len >= sizeof(buffer))
	{
		printf("buffer leak:%d",  cursize+len);
		sleep(3);
	}
	bufLock.Lock();
	memcpy(buffer+cursize, buf, len);
	cursize += len;
	bufLock.Unlock();
	return cursize;
}

bool RevcBuff::getData(char* buf, int& len)
{
	//if(buffer[0] == '\0')
	if(cursize ==  0)
		return false;
	int mesSize = ntohl(*(int*)buffer);
	if(mesSize > cursize)
		return false;
	else
	{
		memcpy(buf, buffer, mesSize);
		bufLock.Lock();
		cursize -= mesSize;
		memcpy(buffer, buffer+mesSize,  cursize);
		bufLock.Unlock();
		return true;
	}
}

void RevcBuff::resetData()
{
	bufLock.Lock();
	cursize = 0;
	bufLock.Unlock();
}
