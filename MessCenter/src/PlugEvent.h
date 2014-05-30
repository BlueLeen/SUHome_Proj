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

	int recv_hotplug_sock(char* buf, int len);

private:
	static const int UEVENT_BUFFER_SIZE = 2048;

	int m_sckfd;

	void init_hotplug_sock();
	void close_hotplug_sock();
};

#endif /* PLUGEVENT_H_ */
