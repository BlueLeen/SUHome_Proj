/*
 * SocketSeal.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: leen
 */

#include "SocketSeal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "LogFile.h"

//#define APP_ROOT_PATH "/system/strongunion/"
#define RCVSIZE   512

SocketSeal::SocketSeal()
:m_sockSrvfd(-1),bContinue(1)
{
	// TODO Auto-generated constructor stub

}

SocketSeal::SocketSeal(int nConnPort)
{
	// TODO Auto-generated constructor stub
	start_server_socket(nConnPort);
}

SocketSeal::~SocketSeal() {
	// TODO Auto-generated destructor stub
	close_server_socket();
}

void SocketSeal::start_server_socket(int nConnPort)
{
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(nConnPort);
    /*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/
	if((m_sockSrvfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
#ifdef DEBUG
		LogFile::write_sys_log("create server socket failed!");
#endif
		return;
	}
	/*将套接字绑定到服务器的网络地址上*/
    if( bind(m_sockSrvfd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", nConnPort);
#ifdef DEBUG
        char szLog[100];
        sprintf(szLog, "server bind port:%d failed!", nConnPort);
		LogFile::write_sys_log(szLog);
#endif
        return;
    }
    //server_socket用于监听
    if ( listen(m_sockSrvfd, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!");
#ifdef DEBUG
		LogFile::write_sys_log("server listen failed!");
#endif
        return;
    }
}

int SocketSeal::accept_client_socket()
{
	int sockCltfd;
	struct sockaddr_in remote_addr; //客户端网络地址结构体
	socklen_t sin_size;
	sin_size=sizeof(struct sockaddr_in);
	//sin_size=sizeof(struct sockaddr);
	bzero(&remote_addr,sizeof(remote_addr)); //把一段内存区的内容全部设置为0
	if((sockCltfd=accept(m_sockSrvfd,(struct sockaddr *)&remote_addr,&sin_size))<0)
	{
		perror("accept");
#ifdef DEBUG
		LogFile::write_sys_log("accept client socket failed!");
#endif
		return 0;
	}
	printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr));
	return sockCltfd;
}

int SocketSeal::send_socket_packs(char* szBuf, int nSize, int nClientSockfd)
{
	int len = send(nClientSockfd, szBuf, nSize,0);
	return len;
}

int SocketSeal::receive_socket_packs(char* szBuf, int nSize, int nClientSockfd)
{
	int len=recv(nClientSockfd, szBuf, nSize, 0);
	szBuf[len] = '\0';
	return len;
}

int SocketSeal::receive_buffer(int& nClientSockfd, void** pBuf)
{
	static int nFlag = 1;
	static int nRec = 0;
	static int fileSize = 0;
	unsigned char buf[RCVSIZE] = { 0 };
	static int len = 1;
	if(len <= 0)
	{
		close(nClientSockfd);
		nClientSockfd = -1;
		return 2;
	}
	else
		len=recv(nClientSockfd, buf, RCVSIZE, 0);
	if(nFlag)
	{
		fileSize = ntohl((int)(*(int*)buf));
		if(fileSize == 0 || len > 4000)
			return 2;
		nFlag = 0;
		*pBuf = malloc(fileSize);
#ifdef DEBUG
		char szLog[100] = { 0 };
		sprintf(szLog, "<receive_buffer>File Size:%d", fileSize);
		LogFile::write_sys_log(szLog);
#endif
	}
	memcpy(*pBuf+nRec, buf, len);
	nRec += len;
#ifdef DEBUG
		char szLog[100] = { 0 };
		sprintf(szLog, "<receive_buffer>File Receive Current Length:%d", nRec);
		LogFile::write_sys_log(szLog);
#endif
	if(nRec >= fileSize)
	{
		nFlag = 1;
		nRec = 0;
		return 0;
	}
	return 1;
}

void SocketSeal::close_client_socket(int nClientSockfd)
{
	if(nClientSockfd)
	{
		close(nClientSockfd);
	}
}

void SocketSeal::close_server_socket()
{
	close(m_sockSrvfd);
}
