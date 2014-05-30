/*
 * InterfaceFull.h
 *
 *  Created on: May 30, 2014
 *      Author: leen
 */

#ifndef INTERFACEFULL_H_
#define INTERFACEFULL_H_

class InterfaceFull {
public:
	InterfaceFull();
	virtual ~InterfaceFull();

	static int install_android_apk(char* szApk);
private:
	static int phone_is_online(char* buf);
	static int systemdroid(const char * cmdstring);
};

#endif /* INTERFACEFULL_H_ */
