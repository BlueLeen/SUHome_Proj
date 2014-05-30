/*
 * MessCenterCall.cpp
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#include "MessCenterCall.h"
#include "InterfaceFull.h"
#include <stdio.h>


MessCenterCall::MessCenterCall() {
	// TODO Auto-generated constructor stub

}

MessCenterCall::~MessCenterCall() {
	// TODO Auto-generated destructor stub
}

void MessCenterCall::AsynCallback(int code, DeviceInfo& dev)
{
	if(code == 1)
	{
		//if the device is phone
		char szApkFile[20];
		sprintf(szApkFile, "%s", "eserve.apk");
		InterfaceFull::install_android_apk(szApkFile);
	}
	else if(code == 2)
	{
		//if the device is U pan
	}
	else if(code == 3)
	{
		//if the device is storage card
	}
	else
	{
	}
}

