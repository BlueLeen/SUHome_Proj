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

#define MAXSIZE 1024
#define CFG_DIR_NAME "cfg"
#define CFG_FILE_NAME "setting"
#define CFG_SEC_TIME "Time"

int global_client_sockfd = 0;

int start_client_socket(const char* szRemoteSrvAddr, int nConnPort)
{
	struct sockaddr_in remote_addr; //服务器端网络地址结构体
	memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零
	remote_addr.sin_family=AF_INET;
	remote_addr.sin_addr.s_addr=inet_addr(szRemoteSrvAddr);
	remote_addr.sin_port=htons(nConnPort);

	/*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
    if((global_client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
        return 0;
    }

    /*将套接字绑定到服务器的网络地址上*/
    if(connect(global_client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
    {
        perror("connect");
        return 0;
    }
    return global_client_sockfd;
}

int send_socket_packs(char* szBuf, int nSize)
{
	int len = send(global_client_sockfd, szBuf, nSize,0);
	return len;
}

int receive_socket_packs(char* szBuf, int nSize)
{
	int len=recv(global_client_sockfd, szBuf, nSize, 0);
	szBuf[len] = '\0';
	return len;
}
void close_client_socket()
{
	if(!global_client_sockfd)
	{
		close(global_client_sockfd);
	}
}

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
	char* szTime;
	szTime = get_download_time();
	char buf[BUFSIZ] = { 0 };
    printf("connected to server\n");
    start_client_socket("192.168.5.7", 8000);
//    len=recv(client_sockfd,buf,BUFSIZ,0);//接收服务器端信息
//         buf[len]='\0';
//    printf("%s",buf); //打印服务器端信息
    strcpy(buf, szTime);
    send_socket_packs(buf, strlen(szTime));
    receive_socket_packs(buf, BUFSIZ);
    printf("%s\n", buf);

    /*循环的发送接收信息并打印接收信息--recv返回接收到的字节数，send返回发送的字节数*/
    while(1)
    {
        printf("Enter string to send:");
        scanf("%s",buf);
        if(!strcmp(buf,"quit"))
            break;
//        len=send(client_sockfd,buf,strlen(buf),0);
//        len=recv(client_sockfd,buf,BUFSIZ,0);
//        buf[len]='\0';
        send_socket_packs(buf, strlen(buf));
        receive_socket_packs(buf, BUFSIZ);
        printf("received:%s\n",buf);
    }
//    close(client_sockfd);//关闭套接字
    close_client_socket();
	return EXIT_SUCCESS;
}
