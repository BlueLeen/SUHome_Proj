/*
 * PlugEvent.h
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#ifndef PLUGEVENT_H_
#define PLUGEVENT_H_

class PlugEvent {
public:
	PlugEvent();
	virtual ~PlugEvent();

	static int recv_hotplug_sock(char* buf, int len);

private:
	static const int UEVENT_BUFFER_SIZE = 2048;

	static int m_sckfd;

	static void init_hotplug_sock();
	static void close_hotplug_sock();
};

#endif /* PLUGEVENT_H_ */
