/*
 * SocketSeal.h
 *
 *  Created on: Jun 11, 2014
 *      Author: leen
 */

#ifndef SOCKETSEAL_H_
#define SOCKETSEAL_H_

//#include "RevcBuff.h"
#include "SocketClient.h"


class SocketSeal {
public:
	SocketSeal();
	SocketSeal(int nConnPort);
	virtual ~SocketSeal();

//	int start_server_socket(int nConnPort);
//	int accept_client_socket();
//	int go_for_receive(int nClientSockfd);
	int start_server_socket(int nConnPort);
	SocketClient* accept_client_socket();
	int send_socket_packs(char* szBuf, int nSize, int nClientSockfd);
	int receive_socket_packs(char* szBuf, int nSize, int nClientSockfd);
	//int receive_buffer(int nClientSockfd, void** pBuf);
	//int receive_buffer(int& nClientSockfd, void** pBuf);
	//int receive_buffer(int& nClientSockfd, void** pBuf);
	bool receive_buffer(int& nClientSockfd, void** pBuf);
//	void close_client_socket(int nClientSockfd);
	void close_client_socket(SocketClient* sc);
	void close_server_socket();

public:
	RevcBuff  rb;

private:
	int m_sockSrvfd;
	bool bContinue;
	static const int LENGTH_OF_LISTEN_QUEUE=5;
};

#endif /* SOCKETSEAL_H_ */
