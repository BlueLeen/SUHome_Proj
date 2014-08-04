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
#include "DeviceInfo.h"

using std::string;

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
	static void plug_opp_dev(string& strMessage, DeviceInfo* pDev);
	static unsigned long get_file_size(const char *path);
	static void read_file_pos(char* buf, const char *path, long int pos = 0);
};

#endif /* DEVICEINFO_H_ */
