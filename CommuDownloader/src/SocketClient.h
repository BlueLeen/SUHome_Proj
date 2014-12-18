/*
 * SocketClient.h
 *
 *  Created on: 2014年12月17日
 *      Author: su
 */

#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_
#include "RevcBuff.h"

class SocketClient {
public:
	SocketClient(int fd);
	virtual ~SocketClient();
	int receive_socket_packs();
	int receive_buffer(char* buf);
	int socket_fd();
	void close();
private:
	RevcBuff m_rb;
	int m_fd;
	bool m_bConnected;

	static const int RCVSIZE=512;
};

#endif /* SOCKETCLIENT_H_ */
