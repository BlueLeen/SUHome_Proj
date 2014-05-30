/*
 * MessCenterCall.h
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#ifndef MESSCENTERCALL_H_
#define MESSCENTERCALL_H_

#include "AsynCall.h"

class MessCenterCall: public AsynCall {
public:
	MessCenterCall();
	virtual ~MessCenterCall();

	virtual void AsynCallback(int code, DeviceInfo& dev);
};

#endif /* MESSCENTERCALL_H_ */
