/*
 * InterfaceFull.h
 *
 *  Created on: May 30, 2014
 *      Author: leen
 */

#ifndef INTERFACEFULL_H_
#define INTERFACEFULL_H_

#define MINROWSIZE 50
#define USBCOUNT 8

typedef struct _SerialLine
{
	char szSerial[MINROWSIZE];
	bool bOnline;
}SerialLine;

class InterfaceFull {
public:
	InterfaceFull();
	virtual ~InterfaceFull();

	//static int install_android_apk(char* szApk);
	static int install_android_apk(char* szApk, char* szSerialno);
	static void start_adb();
//	static bool open_android_usbdebug();
	static bool open_android_usbdebug(char* szSerialno);
	//static bool phone_state_off();
	static bool phone_state_off(char* szSerialno);
	//static void phone_plug_out();
private:
	static int phone_is_online(char* buf);
	static bool phone_is_online(char* buf, char* cmd);
	static int systemdroid(const char * cmdstring);
	static int execstream(const char *cmdstring, char *buf, int size);
	//static void detect_device();
	static char* get_adb_path();
//	static SerialLine m_Sl[USBCOUNT];
//	static int m_nCount;
};

#endif /* INTERFACEFULL_H_ */
