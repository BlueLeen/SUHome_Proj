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
#include "Log.h"

#define TINYSIZE  50
#define SMALLSIZE 100
#define CFG_DIR_NAME "cfg"
#define CFG_FILE_NAME "setting"
#define CFG_SEC_TIME "Time"
#define SOCKET_STATR_TOKEN "Start-The socket created by leen.Email:blueleen@163.com"
#define APK_DIR_NAME "ApkDir"
#define ADB_TCP_SERVER_PORT 7100

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

	int sockSrv;

	sockSrv = start_server_socket(ADB_TCP_SERVER_PORT);

	//wait for the client to connect
	while(1)
	{
		int sockClt;
		char buf[BUFSIZ]; //数据传送的缓冲区
		int len = 0;
		sockClt = accept_client_socket(sockClt, sockSrv);
		if (sockClt != 0)
		{
			printf("%s\n", buf);
			//long long ulTime = _atoi64(buf);
			//printf("%lld\n", ulTime);
			int i;
//			for(i=0; i<10; i++)
//			{
//			send_socket_packs("Start-The socket created by leen.Email:blueleen@163.com", strlen("Start-The socket created by leen.Email:blueleen@163.com")+1, sockClt);//发送欢迎信息
//			}
			receive_socket_packs(buf, BUFSIZ, sockClt);
			write_sys_log_text(buf);
			if (buf[0]=='S' && buf[10]=='s')
			{
//				send_socket_packs(sockClt, SOCKET_START_TOKEN, sizeof(SOCKET_START_TOKEN));//发送欢迎信息
//				HANDLE hThread = CreateThread(NULL, 0, ThreadSend, (LPVOID)sockClt, CREATE_SUSPENDED, NULL);
//				ResumeThread(hThread);
//				WaitForSingleObject(hThread, INFINITE);
//				CloseHandle(hThread);
			}
		}
		close_client_socket(sockClt);
	}
	close_server_socket(sockSrv);

//	memset(global_client_buffer, 0, sizeof(global_client_buffer));
//	char* szTime;
//	szTime = get_download_time();
//	char buf[BUFSIZ] = { 0 };
//    printf("connected to server\n");
//    while(buf[0] == 0)
//    {
//        start_client_socket("192.168.5.7", ADB_TCP_SERVER_PORT, &global_client_sockfd);
//        send_socket_packs(SOCKET_STATR_TOKEN, strlen(SOCKET_STATR_TOKEN)+1, global_client_sockfd);
//        receive_socket_packs(buf, BUFSIZ, global_client_sockfd);
//        write_sys_log_text(buf);
//        if(buf[0]!='S' || buf[11]!='W')
//        {
//        	memset(buf, 0, BUFSIZ);
//        	close_client_socket(global_client_sockfd);
//        }
//        printf("%s\n", buf);
//    }
//
//	pthread_t pt_recv = 0;
//	void* pBuffer = NULL;
//	get_current_path(global_apk_path, MAXSIZE);
//	sprintf(global_apk_path, "%s%s", global_apk_path, APK_DIR_NAME);
//	while(1)
//	{
//		while(receive_buffer(global_client_sockfd, &pBuffer))
//		{
//		}
//		int bufferSize = *((int*)pBuffer);
//		if(bufferSize > 0)
//			pthread_create(&pt_recv, NULL, pthread_func_recv, pBuffer);
//	}
//    close_client_socket(global_client_sockfd);
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
