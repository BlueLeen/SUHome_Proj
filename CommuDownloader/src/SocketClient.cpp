/*
 * SocketClient.cpp
 *
 *  Created on: 2014年12月17日
 *      Author: su
 */

#include "SocketClient.h"
#include <stdio.h>
#include <sys/socket.h>
#include "LogFile.h"

SocketClient::SocketClient(int fd) {
	// TODO Auto-generated constructor stub
	m_fd = fd;
	m_bConnected = true;
}

SocketClient::~SocketClient() {
	// TODO Auto-generated destructor stub
}

int SocketClient::receive_socket_packs()
{
	char buf[RCVSIZE] = { 0 };
	int len=recv(m_fd, buf, RCVSIZE, 0);
	if(len <= 0)
	{
		return 2;
	}
//#ifdef DEBUG
//	char szLog[200] = { 0 };
//	sprintf(szLog, "<<<<<<<<receive data length:%d>>>>>>>>>", len);
//	LogFile::write_sys_log(szLog);
//#endif
	m_rb.putData(buf, len);
	return 0;
}

int SocketClient::receive_buffer(char* buf)
{
	int recvLen = 0;
	if(m_bConnected)
	{
		if(!m_rb.getData(buf, recvLen))
			return 1;
	}
	else
	{
		m_rb.resetData();
		return 0;
	}
	return 2;
}

int SocketClient::socket_fd()
{
	return m_fd;
}

void SocketClient::close()
{
	//m_rb.resetData();
	m_bConnected = false;
}

