/*
 * DeviceInfo.h
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#ifndef DEVICEDETECT_H_
#define DEVICEDETECT_H_

#include <stdio.h>
#include <iostream>
#include <limits.h>
#include <string.h>
#include "DeviceInfo.h"

using std::string;

typedef struct _USBHUB
{
	_USBHUB():usbNum(0),bInUse(false),type(0),state(0),phoneOpenUsbDebug(false)
	{
		memset(addpath, 0, sizeof(addpath));
		memset(phoneImei, 0, sizeof(phoneImei));
	}
	void clear()
	{
		usbNum = 0;
		bInUse=false;
		type=0;
		state=0;
		memset(addpath, 0, sizeof(addpath));
		memset(phoneImei, 0, sizeof(phoneImei));
		phoneOpenUsbDebug=false;
	}
	int usbNum; //(0-8)
	bool bInUse;
	int type; //1-UPan;2-Phone;3-USB Hub;4:sdcard
	int state;
	char addpath[PATH_MAX];
	char phoneImei[100];//the phone's imei number
	bool phoneOpenUsbDebug;
}USBHUB;


class DeviceDetect {
public:
	DeviceDetect();
	virtual ~DeviceDetect();

	void plug_dev_detect();

	static void send_usb_info();

private:
	static bool m_bGetDeviceFileMethod;
	static bool m_bPlugedDevice;

	static unsigned long m_lastAddTime;
	static unsigned long m_lastChangeTime;
	static unsigned long m_lastRemoveTime;
	static unsigned long m_nUsbFileSize;

	static unsigned long m_lastSdAddTime;
	static unsigned long m_lastSdChangeTime;
	static unsigned long m_lastSdRemoveTime;

	static char m_szAddTextPath[PATH_MAX];
	static char m_szSdAddTextPath[PATH_MAX];

	//static const int UEVENT_BUFFER_SIZE = 2048;
	static const int UEVENT_BUFFER_SIZE = 64*1024;;

	static void* pthread_func_plug(void* ptr);
	static void* pthread_func_call(void* ptr);
	static void* pthread_func_detect(void* ptr);
	static int plug_opp_dev(char* usb_message, int nLen);
	static int plug_opp_dev(char* usb_message);
	static void plug_opp_dev(string& strMessage, DeviceInfo* pDev);
	static bool usb_plug_dev(const char* buf, int& num);
	static bool usb_plug_dev(const char* buf, int& num, int len);
	static bool usb_pull_dev(const char* buf, int& num);
	static bool usb_pull_dev(const char* buf, int& num, int len);
	static unsigned long get_file_size(const char *path);
	static void read_file_pos(char* buf, const char *path, long int pos = 0);
};

#endif /* DEVICEINFO_H_ */
