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
#define MINSIZE  200

typedef struct _SocketRb
{
	SocketSeal* ss;
	int nClt;
}SocketRb;

//void* pthread_func_put(void* ptr)
//{
//	SocketRb* srb = (SocketRb*)ptr;
//	char buf[RCVSIZE] = { 0 };
//	int len=recv(srb->nClt, buf, RCVSIZE, 0);
//	if(len <= 0)
//	{
//		srb->ss->close_client_socket(srb->nClt);
//		//remove_client_fd(sockClt);
//		shutdown(srb->nClt, SHUT_RDWR);
//#ifdef DEBUG
//		char szLog[MINSIZE] = { 0 };
//		sprintf(szLog, "%s:%d!", "close client socket", srb->nClt);
//		LogFile::write_sys_log(szLog);
//#endif
//		return NULL;
//	}
//	srb->ss->rb.putData(buf, len);
//	return NULL;
//}

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

int SocketSeal::start_server_socket(int nConnPort)
{
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(nConnPort);
    /*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/
	if((m_sockSrvfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
#ifdef DEBUG
		LogFile::write_sys_log("create server socket failed!");
#endif
		return 0;
	}
    // 设置socket选项，这是可选的，可以避免服务器程序结束后无法快速重新运行
    int val=1;
    setsockopt(m_sockSrvfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	/*将套接字绑定到服务器的网络地址上*/
    if( bind(m_sockSrvfd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", nConnPort);
#ifdef DEBUG
        char szLog[100];
        sprintf(szLog, "server bind port:%d failed!", nConnPort);
		LogFile::write_sys_log(szLog);
#endif
        return 0;
    }
    //server_socket用于监听
    if ( listen(m_sockSrvfd, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!");
#ifdef DEBUG
		LogFile::write_sys_log("server listen failed!");
#endif
        return 0;
    }
    return m_sockSrvfd;
}

//int SocketSeal::accept_client_socket()
//{
//	int sockCltfd;
//	struct sockaddr_in remote_addr; //客户端网络地址结构体
//	socklen_t sin_size;
//	sin_size=sizeof(struct sockaddr_in);
//	//sin_size=sizeof(struct sockaddr);
//	bzero(&remote_addr,sizeof(remote_addr)); //把一段内存区的内容全部设置为0
//	if((sockCltfd=accept(m_sockSrvfd,(struct sockaddr *)&remote_addr,&sin_size))<0)
//	{
//		perror("accept");
//#ifdef DEBUG
//		LogFile::write_sys_log("accept client socket failed!");
//#endif
//		return 0;
//	}
//	printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr));
//	return sockCltfd;
//}
//
//int SocketSeal::go_for_receive(int nClientSockfd)
//{
////		pthread_t pt_recv = 0;
////		SocketRb* srb = new SocketRb();
////		srb->ss = this;
////		srb->nClt = nClientSockfd;
////		pthread_create(&pt_recv, NULL, pthread_func_put, (void*)srb);
//		char buf[RCVSIZE] = { 0 };
//		int len=recv(nClientSockfd, buf, RCVSIZE, 0);
//		if(len <= 0)
//		{
////			close_client_socket(nClientSockfd);
////			//remove_client_fd(sockClt);
////			shutdown(nClientSockfd, SHUT_RDWR);
////	#ifdef DEBUG
////			char szLog[MINSIZE] = { 0 };
////			sprintf(szLog, "%s:%d!", "close client socket", nClientSockfd);
////			LogFile::write_sys_log(szLog);
////	#endif
//			return 2;
//		}
//		rb.putData(buf, len);
//		return 0;
//}

SocketClient* SocketSeal::accept_client_socket()
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
#ifdef DEBUG
	char szLog[200] = { 0 };
	sprintf(szLog, "accept client %s, client filediscriptor:%d\n",inet_ntoa(remote_addr.sin_addr), sockCltfd);
	LogFile::write_sys_log(szLog);
#endif

	return new SocketClient(sockCltfd);
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

bool SocketSeal::receive_buffer(int& nClientSockfd, void** pBuf)
{
//	static int nFlag = 1;
//	static int nRec = 0;
//	static int fileSize = 0;
//	unsigned char buf[RCVSIZE] = { 0 };
//	int len=recv(nClientSockfd, buf, RCVSIZE, 0);
//	if(len <= 0)
//	{
//		return 2;
//	}
//	if(nFlag)
//	{
//		nRec = 0;
//		fileSize = ntohl((int)(*(int*)buf));
//		if(fileSize == 0 || len > 4000)
//			return 2;
//		nFlag = 0;
//		*pBuf = malloc(fileSize);
//		if(*pBuf == NULL)
//			LogFile::write_sys_log("system have not enough memory space!");
//#ifdef DEBUG
//		char szLog[100] = { 0 };
//		sprintf(szLog, "<receive_buffer>File Size:%d", fileSize);
//		LogFile::write_sys_log(szLog);
//#endif
//	}
//#ifdef DEBUG
//		char szLog[100] = { 0 };
//		sprintf(szLog, "nRec:%d", nRec);
//		LogFile::write_sys_log(szLog);
//#endif
//	memcpy((unsigned char*)(*pBuf)+nRec, buf, len);
//	nRec += len;
//#ifdef DEBUG
//		//char szLog[100] = { 0 };
//		sprintf(szLog, "<receive_buffer>File Receive Current Length:%d", nRec);
//		LogFile::write_sys_log(szLog);
//#endif
//	if(nRec >= fileSize)
//	{
//		nFlag = 1;
//		nRec = 0;
//		return 0;
//	}
//	return 1;


	char szBuf[1024] = { 0 };
	int recvLen = 0;
	if(!rb.getData(szBuf, recvLen))
		return false;
	int packSize = ntohl(*(int*)szBuf);
	*pBuf = malloc(packSize+1);
	memset(*pBuf, 0, packSize+1);
	memcpy(*pBuf, szBuf, packSize);
	return true;
}

//void SocketSeal::close_client_socket(int nClientSockfd)
//{
//	if(nClientSockfd)
//	{
//		close(nClientSockfd);
//	}
//}
void SocketSeal::close_client_socket(SocketClient* sc)
{
	int nClientSockfd = sc->socket_fd();
	if(nClientSockfd)
	{
		sc->close();
		close(nClientSockfd);
	}
	delete sc;
}

void SocketSeal::close_server_socket()
{
	close(m_sockSrvfd);
}
