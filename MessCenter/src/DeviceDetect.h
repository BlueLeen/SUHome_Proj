/*
 * DeviceInfo.h
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#ifndef DEVICEDETECT_H_
#define DEVICEDETECT_H_

#include <stdio.h>
#include "AsynCall.h"

class DeviceDetect {
public:
	DeviceDetect();
	virtual ~DeviceDetect();

	void plug_dev_detect(AsynCall* call);

private:
	static const int UEVENT_BUFFER_SIZE = 2048;

	static AsynCall* m_call;

	static void* pthread_func_plug(void* ptr);
	static void* pthread_func_call(void* ptr);
	static int plug_opp_dev(char* usb_message, int nLen);
};

#endif /* DEVICEINFO_H_ */
