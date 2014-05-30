/*
 * AsynCall.cpp
 *
 *  Created on: May 28, 2014
 *      Author: leen
 */

#include "AsynCall.h"
#include "DeviceDetect.h"

AsynCall::AsynCall() {
	// TODO Auto-generated constructor stub

}

AsynCall::~AsynCall() {
	// TODO Auto-generated destructor stub
}

void AsynCall::Init() {
//	int nCode = 5;
//	char szText[100] = {"hello world"};
//	return AsynCallback(nCode, szText);

	DeviceDetect dev;
	dev.plug_dev_detect(this);
}

void AsynCall::AsynCallback(int code, DeviceInfo& dev) {
}

