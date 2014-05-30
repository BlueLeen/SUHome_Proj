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
#include "PlugEvent.h"
#include "LogFile.h"
#include "DeviceInfo.h"

//typedef unsigned long int pthread_t;

DeviceDetect::DeviceDetect() {
	// TODO Auto-generated constructor stub

}

DeviceDetect::~DeviceDetect() {
	// TODO Auto-generated destructor stub
}

void DeviceDetect::plug_dev_detect(AsynCall* call) {
	pthread_t pt_plug = 0;
	pthread_create(&pt_plug, NULL, pthread_func_plug, call);
}

void* DeviceDetect::pthread_func_plug(void* ptr)
{
	pthread_detach((unsigned long int)pthread_self());

	AsynCall* call = (AsynCall*)ptr;

	while(1)
	{
		/* Netlink message buffer */
		PlugEvent event;
		int recvlen = 0;
		char buf[UEVENT_BUFFER_SIZE] = {0};
		LogFile::write_sys_log("begin my detect new devices:");
		recvlen = event.recv_hotplug_sock(buf, sizeof(buf));
		if(recvlen > 0)
		{
			DeviceInfo* pDev = new DeviceInfo();
			pDev->m_nState = 3;
			LogFile::write_sys_log(buf);
			call->AsynCallback(1 , *pDev);
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

	}
	else if(!strcmp(messFlag, "remove"))
	{
		return 6;//if remove the device,return 1
	}
	return 0;
}
