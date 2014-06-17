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

#define DEV_USB_FILE "/proc/bus/usb/devices"
#define ROWSIZE 200


//typedef unsigned long int pthread_t;

AsynCall* DeviceDetect::m_call = NULL;

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

	m_call = (AsynCall*)ptr;

	while(1)
	{
		/* Netlink message buffer */
		PlugEvent event;
		int recvlen = 0;
		char buf[UEVENT_BUFFER_SIZE] = {0};
		recvlen = event.recv_hotplug_sock(buf, sizeof(buf));
		if(recvlen > 0)
		{
			pthread_t pt_recv = 0;
			DeviceInfo* pDev = new DeviceInfo();
			pDev->m_nCode = plug_opp_dev(buf, strlen(buf));
			pDev->m_nState = 3;
			LogFile::write_sys_log(buf);

			pthread_create(&pt_recv, NULL, pthread_func_call, pDev);
		}
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
		//pDev->m_nCode = 6;
	}
	else if(!szCode.compare("remove"))
	{
		pDev->m_nCode = 6;
	}
	pos = strMessage.find("usb");
}

void* DeviceDetect::pthread_func_call(void* ptr)
{
	pthread_detach((unsigned long int)pthread_self());

	DeviceInfo* pDev = (DeviceInfo*)ptr;

	m_call->AsynCallback(pDev->m_nCode, *pDev);

	delete pDev;

	return 0;
}
