#ifndef _SOCKET_SEAL__0X20140423165500
#define _SOCKET_SEAL__0X20140423165500

#include <sys/socket.h>
#include "BufferSeal.h"
#include "FileSeal.h"

#define RCVSIZE   512
#define MAXSIZE   1024
#define BUFFERSIZE MAXSIZE*MAXSIZE*4


unsigned char global_client_buffer[BUFFERSIZE];


//typedef struct _bufRecv
//{
//	unsigned char* szStart;
//	int nSize;
//}RecvBuf;

int start_client_socket(const char* szRemoteSrvAddr, int nConnPort, int* pnClientSockfd)
{
	struct sockaddr_in remote_addr; //服务器端网络地址结构体
	memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零
	remote_addr.sin_family=AF_INET;
	remote_addr.sin_addr.s_addr=inet_addr(szRemoteSrvAddr);
	remote_addr.sin_port=htons(nConnPort);

	/*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
    if((*pnClientSockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
        return 0;
    }

    /*将套接字绑定到服务器的网络地址上*/
    //while表示如果连接服务器不成功一直尝试进行连接
    while(connect(*pnClientSockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
    {
    	sleep(2);
        perror("connect");
        //return 0;
    }
    return *pnClientSockfd;
}

int send_socket_packs(char* szBuf, int nSize, int nClientSockfd)
{
	int len = send(nClientSockfd, szBuf, nSize,0);
	return len;
}

int receive_socket_packs(char* szBuf, int nSize, int nClientSockfd)
{
	int len=recv(nClientSockfd, szBuf, nSize, 0);
	szBuf[len] = '\0';
	return len;
}

int send_buffer(const char* szBuf, int nSize, int nClientSockfd)
{
	char* buf = (char*)malloc(nSize + 5);
	*((int*)&buf) = nSize;
	memcpy(buf+4, szBuf, nSize);
	buf[nSize+5]='\0';
	int len = send(nClientSockfd, buf, nSize+5, 0);
	free(buf);
	return len;
}

int receive_buffer(int nClientSockfd, void** pBuf)
{
	static int nFlag = 1;
	static int nRec = 0;
	static int fileSize = 0;
	unsigned char buf[RCVSIZE] = { 0 };
	int len=recv(nClientSockfd, buf, RCVSIZE, 0);
	if(nFlag)
	{
		fileSize = (int)(*(int*)buf);
		nFlag = 0;
		*pBuf = malloc(fileSize);
#ifdef _DEBUG
		char szLog[100] = { 0 };
		sprintf(szLog, "<receive_buffer>File Size:%d", fileSize);
		write_sys_log(szLog, "CommuLog");
#endif
	}
	memcpy(*pBuf+nRec, buf, len);
	nRec += len;
#ifdef _DEBUG
		char szLog[100] = { 0 };
		sprintf(szLog, "<receive_buffer>File Receive Current Length:%d", nRec);
		write_sys_log(szLog, "CommuLog");
#endif
	if(nRec >= fileSize)
	{
		nFlag = 1;
		nRec = 0;
		return 0;
	}
	return 1;
}

void* parse_socket_packs(char* szBuf, int nLen, void* pac)
{
	AllocMem(szBuf, nLen, pac);
	return pac;
}

void close_client_socket(int nClientSockfd)
{
	if(nClientSockfd)
	{
		close(nClientSockfd);
	}
}















#endif
