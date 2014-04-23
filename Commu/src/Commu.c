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

#define SMALLSIZE 100
#define RCVSIZE   512
#define MAXSIZE   1024
#define CFG_DIR_NAME "cfg"
#define CFG_FILE_NAME "setting"
#define CFG_SEC_TIME "Time"
#define SOCKET_STATR_TOKEN "Start-The socket created by leen.Email:blueleen@163.com"
#define APK_DIR_NAME "ApkDir"

int global_client_sockfd = 0;

static void* pthread_func_recv(void*);



//从INI文件读取字符串类型数据
char *GetIniKeyString(char *title, char *key, char *filename) {
	FILE *fp;
	char szLine[1024] = { 0 };
	static char tmpstr[1024];
	int rtnval;
	int i = 0;
	int flag = 0;
	char *tmp;
	if ((fp = fopen(filename, "r")) == NULL) {
		printf("have   no   such   file \n");
		return "";
	}
	while (!feof(fp)) {
		rtnval = fgetc(fp);
		if (rtnval == EOF) {
			break;
		} else {
			szLine[i++] = rtnval;
		}
		if (rtnval == '\n') {
//#ifndef WIN32
//			i--;
//#endif
			szLine[--i] = '\0';
			i = 0;
			tmp = strchr(szLine, '=');
			if ((tmp != NULL) && (flag == 1)) {
				if (strstr(szLine, key) != NULL) {
//注释行
					if ('#' == szLine[0]) {
					} else if ('/' == szLine[0] && '/' == szLine[1]) {
					} else {
//找打key对应变量
						strcpy(tmpstr, tmp + 1);
						fclose(fp);
						return tmpstr;
					}
				}
			} else {
				strcpy(tmpstr, "[");
				strcat(tmpstr, title);
				strcat(tmpstr, "]");
				if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0) {
//找到title
					flag = 1;
				}
			}
		}
	}
	fclose(fp);
	return "";
}

char* get_current_path(char* szPath, int nLen)
{
	int cnt = readlink("/proc/self/exe", szPath, nLen);
	if(cnt<0 || cnt>=nLen)
	{
		printf("Error:Get current directory!\n");
		return NULL;
	}
	int i;
	for(i=cnt; i>=0; --i)
	{
		if(szPath[i] == '/')
		{
			szPath[i+1] = '\0';
			break;
		}
	}
	//printf("Current absolute path:%s\n", szPath);
	return szPath;
}

//从INI文件读取整类型数据
int GetIniKeyInt(char *title, char *key, char *filename) {
	return atoi(GetIniKeyString(title, key, filename));
}

char* get_download_time()
{
	char szPath[MAXSIZE] = { 0 };
	get_current_path(szPath, MAXSIZE);
	sprintf(szPath, "%s%s/%s", szPath, CFG_DIR_NAME, CFG_FILE_NAME);
//	char* szTime;
//	szTime= GetIniKeyString(CFG_SEC_TIME, "lastdownload", szPath);
	return GetIniKeyString(CFG_SEC_TIME, "lastdownload", szPath);
}

void write_sys_log(char* szWriteString)
{
	char szFilePath[MAXSIZE];
	FILE* fp;
	char szTime[50];
	time_t ti;
	get_current_path(szFilePath, MAXSIZE);
	sprintf(szFilePath, "%s%s", szFilePath, "CommuLog");
	fp = fopen(szFilePath, "a+");
	time(&ti);
	struct tm* ptm = localtime(&ti);
	sprintf(szTime, "当前时间为:%d/%d/%d %d:%d:%d", ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(fp,"%s>>----- %s\n", szTime, szWriteString);
	fclose(fp);
}

int main(int argc, char* argv[]) {
//    int client_sockfd;
//    int len;
//    struct sockaddr_in remote_addr; //服务器端网络地址结构体
//    char buf[BUFSIZ];  //数据传送的缓冲区
//    memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零
//    remote_addr.sin_family=AF_INET; //设置为IP通信
//    remote_addr.sin_addr.s_addr=inet_addr("192.168.5.7");//服务器IP地址
//    remote_addr.sin_port=htons(8000); //服务器端口号
//
//    /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
//    if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
//    {
//        perror("socket");
//        return 1;
//    }
//
//    /*将套接字绑定到服务器的网络地址上*/
//    if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
//    {
//        perror("connect");
//        return 1;
//    }
//	int client_sockfd;
//	int len;
	memset(global_client_buffer, 0, sizeof(global_client_buffer));
	char* szTime;
	szTime = get_download_time();
	char buf[BUFSIZ] = { 0 };
    printf("connected to server\n");
    while(buf[0] == 0)
    {
        start_client_socket("192.168.5.7", 8000, &global_client_sockfd);
    //    len=recv(client_sockfd,buf,BUFSIZ,0);//接收服务器端信息
    //         buf[len]='\0';
    //    printf("%s",buf); //打印服务器端信息
        send_socket_packs(SOCKET_STATR_TOKEN, strlen(SOCKET_STATR_TOKEN)+1, global_client_sockfd);
        receive_socket_packs(buf, BUFSIZ, global_client_sockfd);
        if(buf[0]!='S' || buf[11]!='W')
        {
        	memset(buf, 0, BUFSIZ);
        	close_client_socket(global_client_sockfd);
        }
        printf("%s\n", buf);
    }
    strcpy(buf, szTime);
    send_socket_packs(buf, strlen(szTime)+1, global_client_sockfd);
    /*循环的发送接收信息并打印接收信息--recv返回接收到的字节数，send返回发送的字节数*/
    memset(buf, 0, sizeof(buf));
    receive_socket_packs(buf, BUFSIZ, global_client_sockfd);
    if(buf[1]=='u' && buf[2]=='p')
    {
    	//printf("%s\n", buf+2);
    	send_socket_packs(buf, strlen(buf), global_client_sockfd);
        while(receive_socket_packs(buf, BUFSIZ, global_client_sockfd) && buf[0] == 's')
        {
        	char szFile[MAXSIZE] = { 0 };
        	char szFileName[SMALLSIZE] = { 0 };
        	int len = 0;
        	get_current_path(szFile, MAXSIZE);
        	strcpy(szFileName, buf+2);
        	sprintf(szFile, "%s%s/%s.apk", szFile,APK_DIR_NAME,szFileName);
        	memset(buf, 0, sizeof(buf));
        	//len=receive_socket_packs(buf, BUFSIZ);
        	pthread_t pt_recv = 0;
        	int recvSize = 0;
        	/*int ret = */pthread_create(&pt_recv, NULL, pthread_func_recv, (void*)szFile);
        	while((len=receive_socket_packs((char*)global_client_buffer+recvSize, 30, global_client_sockfd)) )
        	//while(1)
        	{
//        		if(len < SMALLSIZE)
//        		{
//        			char szEnd[SMALLSIZE] = { 0 };
//        			sprintf(szEnd, "e %s", szFileName);
//        			if(!strcmp(buf, szEnd))
//        			{
//        				break;
//        			}
//        		}
        		if(len < 0)
        		{
        			printf("Recieve Data From Server failed!\n");
        		}
        		//len = read(global_client_sockfd, buf, RCVSIZE);
        		//if(len == 0) break;

        		recvSize += len;

        		memcpy(buf, &len, 4);
        		send_socket_packs(buf, 4, global_client_sockfd);
        		//bzero(buf, BUFSIZ);
        		//printf("socket client descriptor:%d\n", global_client_sockfd);
        	}
        	int nEnd = -1;
        	memcpy(global_client_buffer+recvSize, &nEnd, 4);

//       	sprintf(buf, "%s %s", "end", szFileName);
//       	send_socket_packs(buf, strlen(buf)+1);
//        	if(buf[0] == 's')
//        	{
//        		//while()
//        	}
//            printf("Enter string to send:");
//            scanf("%s",buf);
//            if(!strcmp(buf,"quit"))
//                break;
//            send_socket_packs(buf, strlen(buf));
//            receive_socket_packs(buf, BUFSIZ);
//            printf("received:%s\n",buf);
        }
    }
//    while(1)
//    {
//        printf("Enter string to send:");
//        scanf("%s",buf);
//        if(!strcmp(buf,"quit"))
//            break;
////        len=send(client_sockfd,buf,strlen(buf),0);
////        len=recv(client_sockfd,buf,BUFSIZ,0);
////        buf[len]='\0';
//        send_socket_packs(buf, strlen(buf));
//        receive_socket_packs(buf, BUFSIZ);
//        printf("received:%s\n",buf);
//    }
//    close(client_sockfd);//关闭套接字
    close_client_socket(global_client_sockfd);
	return EXIT_SUCCESS;
}

static void* pthread_func_recv(void* data)
{
	FILE* fp = fopen((char*)data, "a+b");
	int segSize = (int)(*global_client_buffer);
	unsigned char *szTemp = global_client_buffer;
	fseek(fp, 0, SEEK_SET);
	sleep(2);
	while(segSize >= 0 )
	{
		if(segSize == 0)
		{
			segSize = (int)(*szTemp);
			continue;
		}
		char szLen[30] = { 0 };
		sprintf(szLen, "Segment:%d", segSize);
		write_sys_log(szLen);
		szTemp += 4;
		int write_len = fwrite(szTemp, 1, segSize, fp);
		if(write_len < segSize)
		{
			printf("File:Write Failed\n");
			break;
		}
		bzero(szTemp-4, segSize+4);
		segSize = (int)(*(szTemp+=segSize));
		sleep(2000);
	}
	fclose(fp);
	return NULL;
}
