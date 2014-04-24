/*
 ============================================================================
 Name        : Commu.c
 Author      : leen
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "SocketSeal.h"
#include "FileSeal.h"

#define TINYSIZE  50
#define SMALLSIZE 100
#define CFG_DIR_NAME "cfg"
#define CFG_FILE_NAME "setting"
#define CFG_SEC_TIME "Time"
#define SOCKET_STATR_TOKEN "Start-The socket created by leen.Email:blueleen@163.com"
#define APK_DIR_NAME "ApkDir"

int global_client_sockfd = 0;
char global_apk_path[MAXSIZE]={0};

static void* pthread_func_recv(void*);


char* get_download_time()
{
	char szPath[MAXSIZE] = { 0 };
	get_current_path(szPath, MAXSIZE);
	sprintf(szPath, "%s%s/%s", szPath, CFG_DIR_NAME, CFG_FILE_NAME);
//	char* szTime;
//	szTime= GetIniKeyString(CFG_SEC_TIME, "lastdownload", szPath);
	return GetIniKeyString(CFG_SEC_TIME, "lastdownload", szPath);
}

int main(int argc, char* argv[]) {
	memset(global_client_buffer, 0, sizeof(global_client_buffer));
	char* szTime;
	szTime = get_download_time();
	char buf[BUFSIZ] = { 0 };
    printf("connected to server\n");
    while(buf[0] == 0)
    {
        start_client_socket("192.168.5.7", 8000, &global_client_sockfd);
        send_socket_packs(SOCKET_STATR_TOKEN, strlen(SOCKET_STATR_TOKEN)+1, global_client_sockfd);
        receive_socket_packs(buf, BUFSIZ, global_client_sockfd);
        if(buf[0]!='S' || buf[11]!='W')
        {
        	memset(buf, 0, BUFSIZ);
        	close_client_socket(global_client_sockfd);
        }
        printf("%s\n", buf);
    }

	pthread_t pt_recv = 0;
	void* pBuffer = NULL;
	get_current_path(global_apk_path, MAXSIZE);
	sprintf(global_apk_path, "%s%s", global_apk_path, APK_DIR_NAME);
	while(1)
	{
		while(receive_buffer(global_client_sockfd, &pBuffer))
		{
		}
		int bufferSize = *((int*)pBuffer);
		if(bufferSize > 0)
			pthread_create(&pt_recv, NULL, pthread_func_recv, pBuffer);
	}
//    strcpy(buf, szTime);
//    send_socket_packs(buf, strlen(szTime)+1, global_client_sockfd);
//    /*循环的发送接收信息并打印接收信息--recv返回接收到的字节数，send返回发送的字节数*/
//    memset(buf, 0, sizeof(buf));
//    receive_socket_packs(buf, BUFSIZ, global_client_sockfd);
//    if(buf[1]=='u' && buf[2]=='p')
//    {
//    	//printf("%s\n", buf+2);
//    	send_socket_packs(buf, strlen(buf), global_client_sockfd);
//        while(receive_socket_packs(buf, BUFSIZ, global_client_sockfd) && buf[0] == 's')
//        {
//        	char szFile[MAXSIZE] = { 0 };
//        	char szFileName[SMALLSIZE] = { 0 };
//        	int len = 0;
//        	get_current_path(szFile, MAXSIZE);
//        	strcpy(szFileName, buf+2);
//        	sprintf(szFile, "%s%s/%s.apk", szFile,APK_DIR_NAME,szFileName);
//        	memset(buf, 0, sizeof(buf));
//        	//len=receive_socket_packs(buf, BUFSIZ);
//        	pthread_t pt_recv = 0;
//        	int recvSize = 0;
//        	/*int ret = */pthread_create(&pt_recv, NULL, pthread_func_recv, (void*)szFile);
//        	while((len=receive_socket_packs((char*)global_client_buffer+recvSize, 30, global_client_sockfd)) )
//        	//while(1)
//        	{
//        		if(len < 0)
//        		{
//        			printf("Recieve Data From Server failed!\n");
//        		}
//
//        		recvSize += len;
//
//        		memcpy(buf, &len, 4);
//        		send_socket_packs(buf, 4, global_client_sockfd);
//        	}
//        	int nEnd = -1;
//        	memcpy(global_client_buffer+recvSize, &nEnd, 4);
//
//        }
//    }
    close_client_socket(global_client_sockfd);
	return EXIT_SUCCESS;
}

static void* pthread_func_recv(void* pBuf)
{
	pthread_detach(pthread_self());
	char szApk[MAXSIZE] = { 0 };
	char szFile[TINYSIZE+1] = { 0 };
	strncpy(szFile, (char*)(pBuf+4), TINYSIZE);
	sprintf(szApk, "%s/%s", global_apk_path, szFile);
	if(access(szApk, F_OK)!=-1)
	{
		unlink(szApk);
	}
	FILE* fp = fopen((char*)szApk, "a+b");
	int fileSize = *((int*)pBuf) - 54;
	fseek(fp, 0, SEEK_SET);
	fwrite(pBuf+54, 1, fileSize, fp);
	free(pBuf);
	fclose(fp);
	return NULL;
}
