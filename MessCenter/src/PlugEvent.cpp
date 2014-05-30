/*
 * PlugEvent.cpp
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#include "PlugEvent.h"
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <unistd.h> //[getpid()]

PlugEvent::PlugEvent() {
	// TODO Auto-generated constructor stub
	init_hotplug_sock();
}

PlugEvent::~PlugEvent() {
	// TODO Auto-generated destructor stub
	close_hotplug_sock();
}

void PlugEvent::init_hotplug_sock()
{
	const int buffersize = 1024;
	int ret;

	struct sockaddr_nl snl;
	//bzero(&snl, sizeof(struct sockaddr_nl));
	memset(&snl, 0, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	m_sckfd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (m_sckfd == -1)
	{
		perror("socket");
		return;
	}
	setsockopt(m_sckfd, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

	ret = bind(m_sckfd, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
	if (ret < 0)
	{
		perror("bind");
		close(m_sckfd);
		return;
	}

	return;
}

void PlugEvent::close_hotplug_sock()
{
	close(m_sckfd);
}

int PlugEvent::recv_hotplug_sock(char* buf, int len)
{
	int recvlen = 0;
	recvlen = recv(m_sckfd, buf, len, 0);
	return recvlen;
}
