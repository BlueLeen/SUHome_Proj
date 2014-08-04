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
	static bool open_android_usbdebug();
	static bool phone_state_off();
private:
	static int phone_is_online(char* buf);
	static bool phone_is_online(char* buf, char* cmd);
	static int systemdroid(const char * cmdstring);
	static int execstream(const char *cmdstring, char *buf, int size);
};

#endif /* INTERFACEFULL_H_ */
