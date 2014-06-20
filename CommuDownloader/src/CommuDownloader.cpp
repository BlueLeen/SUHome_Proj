//============================================================================
// Name        : CommuDownloader.cpp
// Author      : leen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <limits.h>
#include <dirent.h>
#include "SocketSeal.h"
#include "LogFile.h"
#include "DeviceDetect.h"
//#include "SqliteManager.h"
#include "InterfaceFull.h"
using namespace std;

#define ADB_TCP_SERVER_PORT 7100
//#define APP_ROOT_PATH "/system/strongunion/"
//#define APP_DATABASE_NAME "sqcommudb"
#define APP_DATABASE_NAME "com"
#define APP_DBTABLE_APKINFO "apkinfo"
#define APK_ICONDIR_NAME "icon"
#define APK_DIR_NAME "dir"
#define APK_TEMP_PATH  "/data/local/tmp/strongunion/tmp"
#define SOCKET_STATR_TOKEN "OK"
#define SOCKET_RECVPACK_CONTENT	512
#define SOCKET_SENDPACK_LENGH	512
#define SOCKET_RECVPACK_LENGH	512
#define MAXSIZE  1024
#define ROWSIZE  200
#define MINSIZE  100
#define TINYSIZE 10
//#define SOCKET_UTF8_BUFFER 1024
//#define SOCKET_BUFFER	   512

#define SOCKET_CODE_UPGRADEBOX  			  	 88
#define SOCKET_CODE_REBOOTBOX  			  	 	 89
#define SOCKET_CODE_RESETBOX  			  	 	 90
#define SOCKET_CODE_STOPBOX  			  	 	 91
#define SOCKET_CODE_UNINSTALLAPK  			  	 92
#define SOCKET_CODE_INSTALLAPK  			  	 93
#define SOCKET_CODE_GETAPKLIST  			  	 94
#define SOCKET_CODE_CHNBOXAPK  			  	 	 95
#define SOCKET_CODE_DELBOXAPK  			  	 	 96
#define SOCKET_CODE_ADDBOXAPK  			  	 	 97
#define SOCKET_CODE_GETBOXINFO  			  	 98
#define SOCKET_CODE_GETWORKMODE  			  	 99
#define SOCKET_CODE_COMMUSTABLISHED  			 100

DeviceDetect global_detect;
SocketSeal global_sock_srv;
//SqliteManager global_sql_mgr;
int global_client_fd[10] = { 0 };

typedef struct _SOCKCLIENTBUF
{
	void* pBuf;
	int clientFd;
}SOCKCLIENTBUF;

extern char* get_current_path(char* szPath, int nLen);

static void* pthread_func_recv(void*);
static void* pthread_func_recv_parse(void*);
void parse_code(int code, char* szBuf, int cltFd);

//int code_convert(char *from_charset,char *to_charset,char *inbuf,unsigned long int inlen,char *outbuf,unsigned long int outlen)
//{
//	iconv_t cd;
//	char **pin = &inbuf;
//	char **pout = &outbuf;
//
//	cd = iconv_open(to_charset,from_charset);
//	if (cd==0) return -1;
//	memset(outbuf,0,outlen);
//	if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
//	iconv_close(cd);
//	return 0;
//}
//
//int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
//{
//	return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
//}

void send_all_client_packs(char* buf, int size)
{
	int i;
	for(i=0; i<10; i++)
	{
		if(global_client_fd[i] != 0)
			global_sock_srv.send_socket_packs(buf, size, global_client_fd[i]);
	}
}

void add_client_fd(int nClientFd)
{
	int i = 0;
	while(global_client_fd[i] != 0)
	{
		i++;
	}
	global_client_fd[i] = nClientFd;
}

void remove_client_fd(int nClientFd)
{
	int i = 0;
	while(global_client_fd[i] != nClientFd)
	{
		i++;
	}
	global_client_fd[i] = 0;
}

unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    FILE *fp;
    fp = fopen(path, "r");
    if(fp == NULL)
        return filesize;
    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    fclose(fp);
    return filesize;
}

bool is_file_exist(const char *path)
{
    if ( !access (path, F_OK) ) {
    	return true;
    }else{
         return false;
    }
}

int get_all_apk(char *path, char (*fileArr)[TINYSIZE])
{
	int 			 nCount = 0;
	DIR              *pDir ;
	struct dirent    *ent  ;
	char              childpath[512];

	pDir=opendir(path);
	memset(childpath,0,sizeof(childpath));

	while((ent=readdir(pDir))!=NULL)
	{
		if(ent->d_type & DT_DIR)
		{
			continue;
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
					continue;
			sprintf(childpath,"%s/%s",path,ent->d_name);
			printf("path:%s/n",childpath);
			//listDir(childpath);
		}
		else
		{
			//cout<<ent->d_name<<endl;
			strcpy(*fileArr, ent->d_name);
			//printf("Addr:%x\n", (*fileArr));
			//printf("String:%s\n", *fileArr);
			fileArr++;
			nCount++;
		}
	}
	return nCount;
}

int main() {
	//cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	char szPath[ROWSIZE] = { 0 };
	get_current_path(szPath, sizeof(szPath));
	sprintf(szPath, "%s%s", szPath, APP_DATABASE_NAME);
#ifdef DEBUG
	LogFile::write_sys_log(szPath);
#endif
	//global_sql_mgr.open_sqlite_db(szPath);
//	global_sql_mgr.create_sqlite_table(APP_DBTABLE_APKINFO, "apkid INTEGER, name VARCHAR(50),"
//			"mode INTEGER, vername VARCHAR(20), pkgname VARCHAR(50), filesize REAL,"
//			"iconpath VARCHAR(20)");
	//global_sql_mgr.create_sqlite_table(APP_DBTABLE_APKINFO, "apkid INTEGER, name VARCHAR(50)");
	global_detect.plug_dev_detect();
	global_sock_srv.start_server_socket(ADB_TCP_SERVER_PORT);
	//wait for the client to connect
	while(1)
	{
		int sockClt;
		char buf[BUFSIZ] = { 0 }; //数据传送的缓冲区
		int len = 0;
		*(int*)(buf+4) = htonl(SOCKET_CODE_COMMUSTABLISHED);
		//g2u(SOCKET_STATR_TOKEN, strlen(SOCKET_STATR_TOKEN), buf+8, len);
		len = sizeof(SOCKET_STATR_TOKEN);
		memcpy(buf+8, SOCKET_STATR_TOKEN, len);
		*(int*)buf = htonl(len+8);
		sockClt = global_sock_srv.accept_client_socket();
		if (sockClt != 0)
		{
			pthread_t pt_recv = 0;
			//void* pBuffer = NULL;
			global_sock_srv.send_socket_packs(buf, len+8, sockClt);
			add_client_fd(sockClt);
			//global_sock_srv.receive_socket_packs(buf, BUFSIZ, sockClt);
			pthread_create(&pt_recv, NULL, pthread_func_recv, &sockClt);
//#ifdef DEBUG
//			LogFile::write_sys_log(buf, APP_ROOT_PATH);
//#endif
		}
	}
	//global_sock_srv.close_server_socket();
	//global_sql_mgr.close_sqlite_db();
	return 0;
}

static void* pthread_func_recv(void* pSockClt)
{
	pthread_detach(pthread_self());
	pthread_t pt_recv_parse = 0;
	void* pBuffer = NULL;
	int sockClt = *(int*)pSockClt;
	int ret = 0;
	while((ret=global_sock_srv.receive_buffer(sockClt, &pBuffer)))
	{
	}
	if(ret == 2)
	{
		global_sock_srv.close_client_socket(sockClt);
		remove_client_fd(sockClt);
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "%s:%d", "close client socket!", sockClt);
		LogFile::write_sys_log(szLog);
#endif
		return NULL;
	}
	int bufferSize = ntohl((int)*(int*)pBuffer);
	SOCKCLIENTBUF* pSckCltBuf = new SOCKCLIENTBUF();
	pSckCltBuf->pBuf = pBuffer;
	pSckCltBuf->clientFd = sockClt;
	if(bufferSize > 0)
		//pthread_create(&pt_recv_parse, NULL, pthread_func_recv_parse, pBuffer);
		pthread_create(&pt_recv_parse, NULL, pthread_func_recv_parse, pSckCltBuf);

	return NULL;
}

static void* pthread_func_recv_parse(void* pSckCltBuf)
{
	pthread_detach(pthread_self());
	SOCKCLIENTBUF* pscb = (SOCKCLIENTBUF*)pSckCltBuf;
//	unsigned int len = ntohl(*(int*)pBuf);
//	unsigned int code = ntohl(*(int*)(pBuf+4));
//	char szContent[SOCKET_RECVPACK_CONTENT] = { 0 };
//	memcpy(szContent, pBuf+8, len-8);
	unsigned int len = ntohl(*(int*)pscb->pBuf);
	unsigned int code = ntohl(*(int*)((unsigned char*)pscb->pBuf+4));
	char szContent[SOCKET_RECVPACK_CONTENT] = { 0 };
	memcpy(szContent, (unsigned char*)pscb->pBuf+8, len-8);
	parse_code(code, szContent, pscb->clientFd);
	free(pscb->pBuf);
	delete pscb;
	return NULL;
}

void extract_content_info(const char* content, char (*field)[MINSIZE], int count, char separator='_')
{
	int i;
	char* tmp = (char*)content;
	for(i=0; i<count-1; i++)
	{
		const char* szSep = strstr(tmp, "_");
		if(szSep == NULL)
			break;
		strncpy((char*)field, tmp, szSep-tmp);
		tmp = (char*)szSep + 1;
		field++;
	}
	strcpy((char*)field, tmp);
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
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The send package is:%d %d %s", 8+nLen,
				nCode, content);
		LogFile::write_sys_log(szLog);
#endif
		return nLen+8;
	}
	else
	{
		*(int*)buf = htonl(8);
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The send package is:%d %d", 8+nLen,
				nCode);
		LogFile::write_sys_log(szLog);
#endif
		return 8;
	}
}

int execstream(const char *cmdstring, char *buf, int size)
{
	FILE* stream;
	stream = popen(cmdstring, "r");
	if(NULL == stream)
	{
		LogFile::write_sys_log("execute adb command failed!");
		strcpy(buf, "failed");
		return 1;
	}
	else
	{
		while(NULL != fgets(buf, size, stream))
		{
		}
		pclose(stream);
		return 0;
	}
}

void trim(char* str, char trimstr=' ')
{
	char* szTmp = str;
	while(*szTmp == ' ')
		szTmp++;
	strcpy(str, szTmp);
	int len = strlen(str);
	szTmp = str + len -1;
	while(*szTmp==' ' || *szTmp=='\n')
		szTmp--;
	*(szTmp+1) = '\0';
}

void parse_code(int code, char* szBuf, int cltFd)
{
	char buf[BUFSIZ] = { 0 }; //数据传送的缓冲区
	char szPath[PATH_MAX] = { 0 };
	if(code == SOCKET_CODE_UPGRADEBOX)
	{
	}
	else if(code == SOCKET_CODE_REBOOTBOX)
	{
		char szCmdString[MINSIZE] = { 0 };
		char szCmdResult[MINSIZE] = { 0 };
		strcpy(szCmdString, "reboot");
		int len = grap_pack(buf, SOCKET_CODE_REBOOTBOX, NULL);
		global_sock_srv.send_socket_packs(buf, len, cltFd);
		execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
	}
	else if(code == SOCKET_CODE_RESETBOX)
	{
	}
	else if(code == SOCKET_CODE_STOPBOX)
	{
		char szCmdString[MINSIZE] = { 0 };
		char szCmdResult[MINSIZE] = { 0 };
		strcpy(szCmdString, "reboot -p");
		int len = grap_pack(buf, SOCKET_CODE_REBOOTBOX, NULL);
		global_sock_srv.send_socket_packs(buf, len, cltFd);
		execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
	}
	else if(code == SOCKET_CODE_UNINSTALLAPK)//delete the instruction
	{
	}
	else if(code == SOCKET_CODE_INSTALLAPK)
	{
		char coninfo[3][MINSIZE];
		extract_content_info(szBuf, coninfo, 1);
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The content's %d fields value is:%s", 1,
				coninfo[0]);
		LogFile::write_sys_log(szLog);
#endif
		char szApkPath[PATH_MAX] = { 0 };
		get_current_path(szPath, sizeof(szPath));
		sprintf(szApkPath, "%s%s/%s", szPath, APK_DIR_NAME, coninfo[0]);
		int result = InterfaceFull::install_android_apk(szApkPath);
		char szContent[ROWSIZE] = { 0 };
		sprintf(szContent, "%s_%d", coninfo[0], result);
		int len = grap_pack(buf, SOCKET_CODE_INSTALLAPK, szContent);
		global_sock_srv.send_socket_packs(buf, len, cltFd);
	}
	else if(code == SOCKET_CODE_GETAPKLIST)//get apk list
	{
		char szApkBuf[MAXSIZE];
		char apklist[200][TINYSIZE];
		int count = get_all_apk(APK_TEMP_PATH, apklist);
		memset(szApkBuf, 0, sizeof(szApkBuf));
		strcpy(szApkBuf, apklist[0]);
		int i;
		for(i=1; i<count; i++)
		{
			strcat(szApkBuf, "_");
			strcat(szApkBuf, apklist[i]);
		}
		int len = grap_pack(buf, SOCKET_CODE_GETAPKLIST, szApkBuf);
		global_sock_srv.send_socket_packs(buf, len, cltFd);
	}
	else if(code == SOCKET_CODE_DELBOXAPK)
	{
		char coninfo[3][MINSIZE];
		extract_content_info(szBuf, coninfo, 3);
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The code is:%d,The content's %d fields value is:%s %s %s", SOCKET_CODE_DELBOXAPK, 3,
				coninfo[0], coninfo[1], coninfo[2]);
		LogFile::write_sys_log(szLog);
#endif
		get_current_path(szPath, sizeof(szPath));
		sprintf(szPath, "%s%s/%s", szPath, APK_DIR_NAME, coninfo[0]);
		int nLen = 0;
		if(!remove(szPath))
			nLen = grap_pack(buf, SOCKET_CODE_DELBOXAPK, "1");
		else
			nLen = grap_pack(buf, SOCKET_CODE_DELBOXAPK, "2");
		global_sock_srv.send_socket_packs(buf, nLen, cltFd);
	}
	else if(code == SOCKET_CODE_ADDBOXAPK || code == SOCKET_CODE_CHNBOXAPK)
	{
		//char sqltext[ROWSIZE] = { 0 };
		//char coninfo[4][MINSIZE];
		//extract_content_info(szBuf, coninfo, 4);
		char coninfo[3][MINSIZE];
		extract_content_info(szBuf, coninfo, 3);
		//sprintf(sqltext, "%s, '%s'", coninfo[0], coninfo[1]);
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The code is:%d,The content's %d fields value is:%s %s %s", SOCKET_CODE_ADDBOXAPK, 3,
				coninfo[0], coninfo[1], coninfo[2]);
		LogFile::write_sys_log(szLog);
#endif
		char szApkPath[PATH_MAX] = { 0 };
		sprintf(szApkPath, "%s/%s", APK_TEMP_PATH, coninfo[0]);
		if(!is_file_exist(szApkPath))
		{
//			*(int*)buf = htonl(9);
//			*(int*)(buf+4) = htonl(SOCKET_CODE_ADDBOXAPK);
//			*(char*)(buf+8) = '2';
//			global_sock_srv.send_socket_packs(buf, 9, cltFd);
			int nLen = grap_pack(buf, SOCKET_CODE_ADDBOXAPK, "2");
			global_sock_srv.send_socket_packs(buf, nLen, cltFd);
			return;
		}
		char szApkFullPath[PATH_MAX] = { 0 };
		get_current_path(szPath, sizeof(szPath));
		sprintf(szApkFullPath, "%s%s/%s", szPath, APK_DIR_NAME, coninfo[1]);
		rename(szApkPath, szApkFullPath);
		//if(!global_sql_mgr.insert_sqlite_table(APP_DBTABLE_APKINFO, sqltext))
		if(false)
		{
			remove(szApkFullPath);
//			*(int*)buf = htonl(9);
//			*(int*)(buf+4) = htonl(SOCKET_CODE_ADDBOXAPK);
//			*(char*)(buf+8) = '2';
//			global_sock_srv.send_socket_packs(buf, 9, cltFd);
			int nLen = grap_pack(buf, SOCKET_CODE_ADDBOXAPK, "2");
			global_sock_srv.send_socket_packs(buf, nLen, cltFd);
		}
		else
		{
//			*(int*)buf = htonl(9);
//			*(int*)(buf+4) = htonl(SOCKET_CODE_ADDBOXAPK);
//			*(char*)(buf+8) = '1';
//			global_sock_srv.send_socket_packs(buf, 9, cltFd);
			int nLen = grap_pack(buf, SOCKET_CODE_ADDBOXAPK, "1");
			global_sock_srv.send_socket_packs(buf, nLen, cltFd);
		}
	}
	else if(code == SOCKET_CODE_GETBOXINFO)
	{
		char szCmdString[MINSIZE] = { 0 };
		char szCmdResult[MINSIZE] = { 0 };
		char szContent[ROWSIZE] = { 0 };
		char szTemp[ROWSIZE] = { 0 };
		char* szTmp = NULL;
		int len = 0;
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The code is:%d,The content's:", SOCKET_CODE_GETBOXINFO);
		LogFile::write_sys_log(szLog);
#endif
		//get the sdk version
		strcpy(szCmdString, "cat /system/build.prop | grep \"ro.build.version.sdk\"");
		execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
		if(szCmdResult[0] != '\0')
		{
			szTmp = strstr(szCmdResult, "=");
			strcpy(szCmdResult, szTmp+1);
			len = strlen(szCmdResult);
			if(szCmdResult[len-1] == '\n')
				szCmdResult[len-1] = '_';
			else
			{
				szCmdResult[len] = '_';
				len++;
			}
			strcat(szContent, szCmdResult);
		}
		else
		{
			strcat(szContent, " _");
		}
		//get the rom version
		memset(szCmdResult, 0, sizeof(szCmdResult));
		strcpy(szCmdString, "cat /system/build.prop | grep \"ro.build.version.release\"");
		execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
		if(szCmdResult[0] != '\0')
		{
			szTmp = strstr(szCmdResult, "=");
			strcpy(szTemp, szTmp+1);
			strcpy(szCmdResult, szTemp);
			len = strlen(szCmdResult);
			if(szCmdResult[len-1] == '\n')
				szCmdResult[len-1] = '_';
			else
			{
				szCmdResult[len] = '_';
				len++;
			}
			strcat(szContent, szCmdResult);
		}
		else
		{
			strcat(szContent, " _");
		}
		//get the memory space
		memset(szCmdResult, 0, sizeof(szCmdResult));
		strcpy(szCmdString, "cat /proc/meminfo | grep \"MemTotal\"");
		execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
		if(szCmdResult[0] != '\0')
		{
			szTmp = strstr(szCmdResult, ":");
			strcpy(szTemp, szTmp+1);
			strcpy(szCmdResult, szTemp);
			trim(szCmdResult);
			szTmp = strstr(szCmdResult, " ");
			*szTmp = '\0';
			unsigned long space = atol(szCmdResult) / 1000;
			sprintf(szContent, "%s%ld_", szContent, space);
		}
		else
		{
			strcat(szContent, " _");
		}
		//get the cpu frequence
		memset(szCmdResult, 0, sizeof(szCmdResult));
		strcpy(szCmdString, "cat /proc/cpuinfo | grep \"BogoMIPS\"");
		execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
		if(szCmdResult[0] != '\0')
		{
			szTmp = strstr(szCmdResult, ":");
			strcpy(szTemp, szTmp+1);
			strcpy(szCmdResult, szTemp);
			trim(szCmdResult);
			szTmp = strstr(szCmdResult, " ");
			*szTmp = '\0';
			float speed = atol(szCmdResult) / 1000;
			sprintf(szContent, "%s%.3f", szContent, speed);
		}
		else
		{
			sprintf(szContent, "%s%s", szContent, " ");
		}

		int nLen = grap_pack(buf, SOCKET_CODE_GETBOXINFO, szContent);
		global_sock_srv.send_socket_packs(buf, nLen, cltFd);
	}
	else if(code == SOCKET_CODE_GETWORKMODE)//delete the instruction
	{
	}
}
