//============================================================================
// Name        : CommuClient.cpp
// Author      : leen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "SocketSeal.h"
using namespace std;


#define MINSIZE  100
#define SOCKET_RECVPACK_CONTENT	512
#define ADB_TCP_SERVER_PORT 7100
#define SOCKET_SELECT_OPTION "Please select options: 1.send packets 2.receive packets 3.exit!\n"
#define SOCKET_SEND_COMMENTS "Edit the packet--The packet format is \"code content\" Stop send packet write 0\n"
#define SOCKET_RECV_COMMENTS "Start receive the packete(If want to stop the receive,please send packet'code equal 0 or 90)--The packet is:\n"

int global_client_sockfd = 0;

int extract_pack(void* buf)
{
	char szLog[MINSIZE] = { 0 };
	unsigned int len = ntohl(*(int*)buf);
	unsigned int code = ntohl(*(int*)((unsigned char*)buf+4));
	char szContent[SOCKET_RECVPACK_CONTENT] = { 0 };
	if(len > 8)
	{
		memcpy(szContent, (unsigned char*)buf+8, len-8);
		//parse_code(code, szContent, global_client_sockfd);
		sprintf(szLog, "The receive package is:%d %d %s", len,
				code, szContent);
		printf("%s\n", szLog);
	}
	else
	{
		sprintf(szLog, "The receive package is:%d %d,the content is:no!", len,
				code);
		printf("%s\n", szLog);
	}
	return len;
}

int grap_pack(void* buf, int nCode, const char* content)
{
	int nLen = 0;
	if(content != NULL)
		nLen = strlen(content);
	*(int*)((unsigned char*)buf+4) = htonl(nCode);
	if(nLen > 0)
	{
		memcpy((unsigned char*)buf+8, content, nLen);
		*(int*)buf = htonl(8+nLen);
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The send package is:%d %d %s", 8+nLen,
				nCode, content);
		printf("%s\n", szLog);
		return nLen+8;
	}
	else
	{
		*(int*)buf = htonl(8);
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The send package is:%d %d", 8+nLen,
				nCode);
		printf("%s\n", szLog);
		return 8;
	}
}

static void* pthread_func_recv(void* pSockClt)
{
	pthread_detach(pthread_self());
	char buf[BUFSIZ] = { 0 };
	while(true)
	{
		receive_socket_packs(buf, BUFSIZ, global_client_sockfd);
		extract_pack(buf);
	}
	return NULL;
}

int main() {
	char buf[BUFSIZ] = { 0 };
    printf("connected to server\n");

	start_client_socket("127.0.0.1", ADB_TCP_SERVER_PORT, &global_client_sockfd);
	//send_socket_packs(SOCKET_STATR_TOKEN, strlen(SOCKET_STATR_TOKEN)+1, global_client_sockfd);
	receive_socket_packs(buf, BUFSIZ, global_client_sockfd);
	extract_pack(buf);

	pthread_t pt_recv = 0;
	pthread_create(&pt_recv, NULL, pthread_func_recv, &global_client_sockfd);

	while(true)
	{
		int nOption = 0;
		printf(SOCKET_SELECT_OPTION);
		scanf("%d", &nOption);
		if (nOption == 1)
		{
			while(true)
			{
				printf(SOCKET_SEND_COMMENTS);
				int len;
				int code;
				char content[200] = { 0 };
				scanf("%d %s", &code, content);
//				*(int*)(buf+4) = htonl(code);
//				memcpy(buf+8, content, strlen(content)+1);
//				len = 8+strlen(content)+1;
//				*(int*)buf = htonl(len);
				len = grap_pack(buf, code, content);
				send_socket_packs(buf, len, global_client_sockfd);//发送欢迎信息
				printf("Send packets:%d %d %s\n", len, code, content);
				//receive_socket_packs(buf, BUFSIZ, global_client_sockfd);
				sleep(2);
				if (code == 0)
				{
					break;
				}
			}
		}
		else if (nOption == 2)
		{
			while(true)
			{
				printf(SOCKET_RECV_COMMENTS);
				int len;
				int code;
				char content[200] = { 0 };
				len = receive_socket_packs(buf, BUFSIZ, global_client_sockfd);//发送欢迎信息
				code = ntohl(*(int*)(buf+4));
				memcpy(content, buf+8, len-8);
				if (code == 0 || code ==90)
				{
					break;
				}

				printf("\n>>>>Recv packets:%d %d %s\n\n", len, code, content);
			}
		}
		else
		{
			break;
		}
	}
	printf("The communication is\n");
	//cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
