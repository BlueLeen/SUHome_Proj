#ifndef _SOCKET_SEAL__0X20140423165500
#define _SOCKET_SEAL__0X20140423165500

#include <sys/socket.h>
#include "BufferSeal.h"
#include "FileSeal.h"

#define RCVSIZE   512
#define MAXSIZE   1024
#define BUFFERSIZE MAXSIZE*MAXSIZE*4
#define LENGTH_OF_LISTEN_QUEUE 5


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
	//remote_addr.sin_addr.s_addr=inet_addr(szRemoteSrvAddr);
	remote_addr.sin_addr.s_addr=INADDR_ANY;
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

int start_server_socket(int nConnPort)
{
	int sockSrvfd;
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(nConnPort);
    /*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/
	if((sockSrvfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		return 0;
	}
	/*将套接字绑定到服务器的网络地址上*/
    if( bind(sockSrvfd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", nConnPort);
        return 0;
    }
    //server_socket用于监听
    if ( listen(sockSrvfd, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!");
        return 0;
    }
	return sockSrvfd;
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

int accept_client_socket(int sockCltfd, const int sockSrvfd)
{
	struct sockaddr_in remote_addr; //客户端网络地址结构体
	int sin_size;
	sin_size=sizeof(struct sockaddr_in);
	if((sockCltfd=accept(sockSrvfd,(struct sockaddr *)&remote_addr,&sin_size))<0)
	{
		perror("accept");
		return 0;
	}
	printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr));
	return sockCltfd;
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

void close_server_socket(int nServerSockfd)
{
	if(nServerSockfd)
	{
		close(nServerSockfd);
	}
}















#endif
