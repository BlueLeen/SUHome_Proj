/*
 * SocketSeal.h
 *
 *  Created on: Jun 11, 2014
 *      Author: leen
 */

#ifndef SOCKETSEAL_H_
#define SOCKETSEAL_H_

class SocketSeal {
public:
	SocketSeal();
	SocketSeal(int nConnPort);
	virtual ~SocketSeal();

	void start_server_socket(int nConnPort);
	int accept_client_socket();
	int send_socket_packs(char* szBuf, int nSize, int nClientSockfd);
	int receive_socket_packs(char* szBuf, int nSize, int nClientSockfd);
	int receive_buffer(int nClientSockfd, void** pBuf);
	void close_client_socket(int nClientSockfd);
	void close_server_socket();

private:
	int m_sockSrvfd;
	bool bContinue;
	static const int LENGTH_OF_LISTEN_QUEUE=5;
};

#endif /* SOCKETSEAL_H_ */
