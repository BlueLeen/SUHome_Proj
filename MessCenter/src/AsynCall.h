/*
 * AsynCall.h
 *
 *  Created on: May 28, 2014
 *      Author: leen
 */

#ifndef ASYNCALL_H_
#define ASYNCALL_H_

#include "DeviceInfo.h"

class AsynCall {
public:
	AsynCall();
	virtual ~AsynCall();

	void Init();
	virtual void AsynCallback(int code, DeviceInfo& dev);
};

#endif /* ASYNCALL_H_ */
