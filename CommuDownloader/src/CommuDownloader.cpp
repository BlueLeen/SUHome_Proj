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
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <pthread.h>
#include <limits.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
//#include <sys/shm.h>
#include <sys/mman.h>
#include "SocketSeal.h"
#include "LogFile.h"
#include "DeviceDetect.h"
//#include "SqliteManager.h"
#include "InterfaceFull.h"
#include "RegTool.h"
using namespace std;

#define ANDROID_SHELL "/system/bin/sh"
#define ADB_ADB_NAME "adb"
#define ADB_TCP_SERVER_PORT 7100
//#define APP_ROOT_PATH "/system/strongunion/"
//#define APP_DATABASE_NAME "sqcommudb"
#define APP_DATABASE_NAME "com"
#define APP_DBTABLE_APKINFO "apkinfo"
#define APK_ICONDIR_NAME "icon"
#define APK_DIR_NAME "dir"
#define ADB_USB_FILE "adb_usb.ini"
#define APK_TEMP_PATH  "/data/local/tmp/strongunion/tmp"
#define APK_DIR_PATH   "/data/local/tmp/strongunion/dir"
#define APK_KEY_DIR  "/data/.android"
#define APK_KEY_ROOT   "/root/.android"
#define APK_KEY_FILE_ADBKEY  	"adbkey"
#define APK_KEY_FILE_ADBKEYPUB  "adbkey.pub"
#define APK_KEY_FILE_ADBKEYUSB  "adb_usb.ini"
#define DEV_INTERNAL_SDCARD_PATH "/mnt/internal_sd"
#define UPGRADE					"Upgrade"
#define SUCCESS					"Success"
#define UPSETTINGPATH			"/data/local/tmp/"
#define UPUPDATEINI				"update.ini"
#define SOCKET_STATR_TOKEN "OK"
#define SOCKET_RECVPACK_CONTENT	512
#define SOCKET_SENDPACK_LENGH	512
#define SOCKET_RECVPACK_LENGH	512
#define MAXSIZE  1024
#define ROWSIZE  400
#define MINSIZE  100
#define TINYSIZE 10
#define READ_BUF_SIZE    50
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
#define SOCKET_CODE_HEARTBEAT  			         104


typedef struct _PHONEVID
{
	_PHONEVID():count(0)
	{
		memset(phoneVid, 0, sizeof(phoneVid));
	}
	unsigned int phoneVid[MAXSIZE];
	int count;
}PHONEVID;

DeviceDetect global_detect;
SocketSeal global_sock_srv;
//SqliteManager global_sql_mgr;
int global_client_fd[10] = { 0 };
PHONEVID global_phone_vidArr;

typedef struct _SOCKCLIENTBUF
{
	void* pBuf;
	int clientFd;
}SOCKCLIENTBUF;

SerialLine* global_ptrSl;
int* global_ptrDevNum;

extern char* get_current_path();
extern char* get_current_path(char* szPath, int nLen);

static void* pthread_func_recv(void*);
static void* pthread_func_recv_parse(void*);
void trim(char* str, char trimstr=' ');
void parse_code(int code, char* szBuf, int cltFd);
int grap_pack(void* buf, int nCode, const char* content);
int extract_pack(void* buf, unsigned int& code, char* szContent);
extern int systemdroid(const char * cmdstring);
//extern int GetStorageInfo(char *TotalCapacity, char *FreeCapacity);
//extern int GetStorageInfo(char * MountPoint, int *Capacity,  int type);
extern int GetStorageInfo(char *TotalCapacity, char *FreeCapacity, char* path);

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

bool SocketConnected(int sock) {
	if (sock <= 0)
		return 0;
	struct tcp_info info;
	int len = sizeof(info);
	getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) &len);
	if ((info.tcpi_state == TCP_ESTABLISHED)) {
		//myprintf("socket connected\n");
		return true;
	} else {
		//myprintf("socket disconnected\n");
		return false;
	}
}

void send_all_client_packs(char* buf, int size)
{
	int i;
	for(i=0; i<10; i++)
	{
		if(global_client_fd[i] != 0)
		{
			if(SocketConnected(global_client_fd[i]))
				global_sock_srv.send_socket_packs(buf, size, global_client_fd[i]);
			else
				global_client_fd[i] = 0;
		}
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
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "add the global client file descriptor,%d:%d!", i, global_client_fd[i]);
		LogFile::write_sys_log(szLog);
#endif
}

void remove_client_fd(int nClientFd)
{
	int i = 0;
	while(global_client_fd[i] != nClientFd)
	{
		i++;
	}
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "remove the global client file descriptor,%d:%d!", i, global_client_fd[i]);
		LogFile::write_sys_log(szLog);
#endif
	global_client_fd[i] = 0;
}

static void* pthread_func_listen(void* pSockClt)
{
	pthread_detach(pthread_self());
	//pthread_t pt_recv_parse = 0;
	void* pBuffer = NULL;
	int sockClt = *(int*)pSockClt;
	static int ret = 0;
	while(true)
	{
		if(sockClt <= 0)
			break;
		//while((ret=global_sock_srv.receive_buffer(sockClt, &pBuffer)))
		while((ret=global_sock_srv.receive_buffer(sockClt, &pBuffer)))
		{
			//if(sockClt <= 0)
			if(ret == 2)
				break;
		}
		if(ret == 2)
		{
			global_sock_srv.close_client_socket(sockClt);
			remove_client_fd(sockClt);
	#ifdef DEBUG
			char szLog[MINSIZE] = { 0 };
			sprintf(szLog, "%s:%d!", "remove client socket", sockClt);
			LogFile::write_sys_log(szLog);
	#endif
			break;
		}
	}
	delete (int*)pSockClt;
	return NULL;
}

void listen_client_fd(int nClientFd)
{
	pthread_t pt_recv = 0;
	pthread_create(&pt_recv, NULL, pthread_func_listen, new int(nClientFd));
	pthread_join(pt_recv,NULL);
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

void set_timer()
{
	struct itimerval oldtv;
    struct itimerval itv;
    itv.it_interval.tv_sec = 60;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, &oldtv);
}

void signal_handler(int signo)
{
	switch (signo) {
	case SIGALRM:
	{
		char buf[MAXSIZE] = { 0 };
		int nLen = grap_pack(buf, SOCKET_CODE_HEARTBEAT, NULL);
		send_all_client_packs(buf, nLen);
#ifdef HEARTBEAT
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "send heartbeat instruction once minute!");
		LogFile::write_sys_log(szLog);
#endif
		break;
	}
//	case SIGVTALRM:
//	printf("Catch a signal -- SIGVTALRM\n");
//	break;
	}
}

bool set_home_env(const char *name, const char * value)
{
	char* tmp;
	tmp = getenv(name);
	if(tmp != NULL)
		return true;
	else
	{
		if(!setenv(name, value, 0))
			return true;
		else
			return false;
	}
}

void get_phone_vendorid()
{
	FILE *fpin;
	//static int nCount=0;
	char line[ROWSIZE] = { 0 };
	char szFile[PATH_MAX] = { 0 };
	sprintf(szFile, "%s%s", get_current_path(), ADB_USB_FILE);
	if(!is_file_exist(szFile))
		return;
	fpin = fopen(szFile, "r");
	while(fgets(line, ROWSIZE, fpin) != NULL)
	{
		if(!strcmp(line, "\n") || line[0]=='#') continue;
		trim(line);
		int len = strlen(line);
		if(line[len-1] == '\r')
			line[len-1] = '\0';
		//global_phone_vid[nCount++] = strtol(line, NULL, 16);
		global_phone_vidArr.phoneVid[global_phone_vidArr.count++] = strtol(line, NULL, 16);
	}
	fclose(fpin);
}

pid_t* find_pid_by_name( char* pidName, int& pidCount)
{
    DIR *dir;
    struct dirent *next;
    pid_t* pidList=NULL;
    int i=0;

    dir = opendir("/proc");


    while ((next = readdir(dir)) != NULL) {
        FILE *status;
        char filename[READ_BUF_SIZE];
        char buffer[READ_BUF_SIZE];
        char name[READ_BUF_SIZE];

        /* Must skip ".." since that is outside /proc */
        if (strcmp(next->d_name, "..") == 0)
            continue;

        /* If it isn't a number, we don't want it */
        if (!isdigit(*next->d_name))
            continue;

        sprintf(filename, "/proc/%s/status", next->d_name);
        if (! (status = fopen(filename, "r")) ) {
            continue;
        }
        if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL) {
            fclose(status);
            continue;
        }
        fclose(status);

        /* Buffer should contain a string like "Name:   binary_name" */
        sscanf(buffer, "%*s %s", name);

        if (strcmp(name, pidName) == 0) {
            pidList=(pid_t*)realloc( pidList, sizeof(pid_t) * (i+2));
            pidList[i++]=strtol(next->d_name, NULL, 0);
        }
    }
    pidCount = i;
    return pidList;
}

void create_dir()
{
	if(!is_file_exist(APK_DIR_PATH))
		//mkdir(APK_DIR_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir(APK_DIR_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
	if(!is_file_exist(APK_TEMP_PATH))
		mkdir(APK_TEMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
}

void create_adbkey_file()
{
	char shellComm[MAXSIZE] = { 0 };
	char szFileSrc[PATH_MAX] = { 0 };
	char szFileDes[PATH_MAX] = { 0 };
	if(!is_file_exist(APK_KEY_DIR ))
		//mkdir(APK_DIR_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir(APK_KEY_DIR, S_IRWXU);
	if(!is_file_exist(APK_KEY_ROOT))
		mkdir(APK_KEY_ROOT, S_IRWXU);
	char* szCurDir = get_current_path();

	snprintf(szFileSrc, sizeof(szFileSrc), "%s%s", szCurDir, APK_KEY_FILE_ADBKEY);
	snprintf(szFileDes, sizeof(szFileDes), "%s/%s", APK_KEY_DIR, APK_KEY_FILE_ADBKEY);
	if(!is_file_exist(szFileDes))
	{
		snprintf(shellComm, sizeof(shellComm), "cp %s %s", szFileSrc, szFileDes);
		systemdroid(shellComm);
	}
	snprintf(szFileDes, sizeof(szFileDes), "%s/%s", APK_KEY_ROOT, APK_KEY_FILE_ADBKEY);
	if(!is_file_exist(szFileDes))
	{
		snprintf(shellComm, sizeof(shellComm), "cp %s %s", szFileSrc, szFileDes);
		systemdroid(shellComm);
	}

	snprintf(szFileSrc, sizeof(szFileSrc), "%s%s", szCurDir, APK_KEY_FILE_ADBKEYPUB);
	snprintf(szFileDes, sizeof(szFileDes), "%s/%s", APK_KEY_DIR, APK_KEY_FILE_ADBKEYPUB);
	if(!is_file_exist(szFileDes))
	{
		snprintf(shellComm, sizeof(shellComm), "cp %s %s", szFileSrc, szFileDes);
		systemdroid(shellComm);
	}
	snprintf(szFileDes, sizeof(szFileDes), "%s/%s", APK_KEY_ROOT, APK_KEY_FILE_ADBKEYPUB);
	if(!is_file_exist(szFileDes))
	{
		snprintf(shellComm, sizeof(shellComm), "cp %s %s", szFileSrc, szFileDes);
		systemdroid(shellComm);
	}

	snprintf(szFileSrc, sizeof(szFileSrc), "%s%s", szCurDir, APK_KEY_FILE_ADBKEYUSB);
	snprintf(szFileDes, sizeof(szFileDes), "%s/%s", APK_KEY_DIR, APK_KEY_FILE_ADBKEYUSB);
	if(!is_file_exist(szFileDes))
	{
		snprintf(shellComm, sizeof(shellComm), "cp %s %s", szFileSrc, szFileDes);
		systemdroid(shellComm);
	}
	snprintf(szFileDes, sizeof(szFileDes), "%s/%s", APK_KEY_ROOT, APK_KEY_FILE_ADBKEYUSB);
	if(!is_file_exist(szFileDes))
	{
		snprintf(shellComm, sizeof(shellComm), "cp %s %s", szFileSrc, szFileDes);
		systemdroid(shellComm);
	}
}

void create_bin()
{
//	mount -o remount,rw /
//	mkdir /bin
//	ln -s /system/bin/sh /bin/sh
	if(!is_file_exist("/bin/sh"))
	{
//#ifdef DEBUG
//		LogFile::write_sys_log("create link file: /bin/sh");
//#endif
		char shellComm[MAXSIZE] = { 0 };
		snprintf(shellComm, sizeof(shellComm), "mount -o remount,rw /");
		systemdroid(shellComm);
		snprintf(shellComm, sizeof(shellComm), "mkdir /bin");
		systemdroid(shellComm);
		snprintf(shellComm, sizeof(shellComm), "ln -s /system/bin/sh /bin/sh");
		systemdroid(shellComm);
	}
}

void KillProcess(char* szProcess)
{
	pid_t* pId;
	int nCount = 0;
	pId = find_pid_by_name(szProcess, nCount);
	for(int i=0; i<nCount; i++)
	{
		if(pId != NULL)
		{
		    if(*pId > 0)
		    {
		        kill(*pId, SIGKILL);
		    }
		}
	}
}

void alloc_shmem()
{
	int fd = shm_open("./myshm", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	ftruncate(fd, 1024);
	void *shm = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	//global_ptrShm = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	*((int*)shm) = 3;
	//printf("\nMemory attached at %X\n", *(int*)shm);
	global_ptrDevNum = (int*)shm;
	*global_ptrDevNum = 3;
	global_ptrSl = (SerialLine*)(shm + sizeof(int*));

//	key_t key;
//	int shmid;//共享内存标识符
//	void *shm = NULL;//分配的共享内存的原始首地址
//	key = ftok("vmmshmsu", 1); // 计算标识符
//	//创建共享内存
//	if ((shmid = shmget(key, 1024, IPC_CREAT | IPC_EXCL |0666)) < 0)
//		printf("shmget error");
//		//exit(1);
//	//将共享内存连接到当前进程的地址空间
//	shm = shmat(shmid, 0, 0);
//	if (shm == (void*) -1) {
//		fprintf(stderr, "shmat failed\n");
//		exit(EXIT_FAILURE);
//	}
//	memset(shm, 0, 1024);
//	global_ptrDevNum = (int*)shm;
//	*global_ptrDevNum = 3;
//	global_ptrSl = (SerialLine*)(shm + sizeof(int*));
	//printf("\nMemory attached at %X\n", (unsigned long) global_share_ptr);
}

void shmem_rw()
{
	struct stat stat;
	int fd = shm_open("./myshm", O_RDWR, S_IRUSR | S_IWUSR);
	fstat(fd, &stat);
	global_ptrDevNum = (int*) mmap(NULL, stat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	global_ptrSl = (SerialLine*)(global_ptrDevNum+sizeof(int*));
	close(fd);
}

int main() {
	//cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	int nCount = 0;
	find_pid_by_name("center", nCount);
	if(nCount > 1)
		exit(1);
	create_dir();
	create_bin();
	create_adbkey_file();
	alloc_shmem();
	printf("\nMemory attached at %X\n", *global_ptrDevNum);
	get_phone_vendorid();
	KillProcess("adb");
	if(!set_home_env("HOME", "/data"))
		LogFile::write_sys_log("create HOME=/data failed!");
	char szAdbPath[PATH_MAX] = { 0 };
	char shellComm[MAXSIZE] = { 0 };
	sprintf(szAdbPath, "%s%s", get_current_path(), ADB_ADB_NAME);
	sprintf(shellComm, "%s start-server", szAdbPath);
	systemdroid(shellComm);
//	char szPath[ROWSIZE] = { 0 };
//	get_current_path(szPath, sizeof(szPath));
//	sprintf(szPath, "%s%s", szPath, APP_DATABASE_NAME);
//#ifdef DEBUG
//	LogFile::write_sys_log(szPath);
//#endif
	set_timer();
	signal(SIGALRM, signal_handler);
	//global_sql_mgr.open_sqlite_db(szPath);
//	global_sql_mgr.create_sqlite_table(APP_DBTABLE_APKINFO, "apkid INTEGER, name VARCHAR(50),"
//			"mode INTEGER, vername VARCHAR(20), pkgname VARCHAR(50), filesize REAL,"
//			"iconpath VARCHAR(20)");
	//global_sql_mgr.create_sqlite_table(APP_DBTABLE_APKINFO, "apkid INTEGER, name VARCHAR(50)");
	global_detect.plug_dev_detect();
	int sockSrv = global_sock_srv.start_server_socket(ADB_TCP_SERVER_PORT);
	//wait for the client to connect
	pid_t pid;
	while(1)
	{
		int sockClt;
		char buf[BUFSIZ] = { 0 }; //数据传送的缓冲区
//		int len = 0;
//		*(int*)(buf+4) = htonl(SOCKET_CODE_COMMUSTABLISHED);
//		len = sizeof(SOCKET_STATR_TOKEN);
//		memcpy(buf+8, SOCKET_STATR_TOKEN, len);
//		*(int*)buf = htonl(len+8);
		int len = grap_pack(buf, SOCKET_CODE_COMMUSTABLISHED, SOCKET_STATR_TOKEN);
		sockClt = global_sock_srv.accept_client_socket();
		add_client_fd(sockClt);

#ifndef ONECLIENT
		if((pid = fork()) == 0)
#endif
		{
#ifndef ONECLIENT
			close(sockSrv);
#endif
			if (sockClt != 0)
			{
				pthread_t pt_recv = 0;
				//void* pBuffer = NULL;
				//global_sock_srv.send_socket_packs(buf, len+8, sockClt);
				global_sock_srv.send_socket_packs(buf, len, sockClt);
				global_detect.send_usb_info();
				//add_client_fd(sockClt);
				//global_sock_srv.receive_socket_packs(buf, BUFSIZ, sockClt);
				pthread_create(&pt_recv, NULL, pthread_func_recv, new int(sockClt));
				pthread_join(pt_recv,NULL);
	//#ifdef DEBUG
	//			LogFile::write_sys_log(buf, APP_ROOT_PATH);
	//#endif
			}
#ifndef ONECLIENT
			exit(0);
#endif
		}

		//listen_client_fd(sockClt);
		//close(sockClt);
	}
	global_sock_srv.close_server_socket();
	//global_sql_mgr.close_sqlite_db();
	return 0;
}

static void* pthread_func_recv(void* pSockClt)
{
	pthread_detach(pthread_self());
	pthread_t pt_recv_parse = 0;
	void* pBuffer = NULL;
	int sockClt = *(int*)pSockClt;
	static int ret = 0;
	while(true)
	{
		if(sockClt <= 0)
			break;
		//while((ret=global_sock_srv.receive_buffer(sockClt, &pBuffer)))
		while((ret=global_sock_srv.receive_buffer(sockClt, &pBuffer)))
		{
			//if(sockClt <= 0)
			if(ret == 2)
				break;
		}
		if(ret == 2)
		{
			global_sock_srv.close_client_socket(sockClt);
			//remove_client_fd(sockClt);
			shutdown(sockClt, SHUT_RDWR);
	#ifdef DEBUG
			char szLog[MINSIZE] = { 0 };
			sprintf(szLog, "%s:%d!", "close client socket", sockClt);
			LogFile::write_sys_log(szLog);
	#endif
			break;
		}
		int bufferSize = ntohl((int)*(int*)pBuffer);
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "%s:%d", "Buffer's first section is", bufferSize);
		LogFile::write_sys_log(szLog);
#endif
		SOCKCLIENTBUF* pSckCltBuf = new SOCKCLIENTBUF();
		pSckCltBuf->pBuf = pBuffer;
		pSckCltBuf->clientFd = sockClt;
		if(bufferSize > 0)
		{
			//pthread_create(&pt_recv_parse, NULL, pthread_func_recv_parse, pBuffer);
			pthread_create(&pt_recv_parse, NULL, pthread_func_recv_parse, pSckCltBuf);
			pthread_join(pt_recv_parse,NULL);
		}
	}
	delete (int*)pSockClt;
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
//	unsigned int len = ntohl(*(int*)pscb->pBuf);
//	unsigned int code = ntohl(*(int*)((unsigned char*)pscb->pBuf+4));
//	char szContent[SOCKET_RECVPACK_CONTENT] = { 0 };
//	memcpy(szContent, (unsigned char*)pscb->pBuf+8, len-8);
	unsigned int code;
	char szContent[SOCKET_RECVPACK_CONTENT] = { 0 };
	extract_pack(pscb->pBuf, code, szContent);
	parse_code(code, szContent, pscb->clientFd);
	free(pscb->pBuf);
	delete pscb;
	return NULL;
}

int extract_content_info(const char* content, char (*field)[MINSIZE], int count, char separator='_')
{
	int i;
	char* tmp = (char*)content;
	for(i=0; i<count-1; i++)
	{
		//const char* szSep = strstr(tmp, "_");
		char* szSep = strstr(tmp, "_");
		if(szSep == NULL)
			break;
		strncpy((char*)field, tmp, szSep-tmp);
		tmp = (char*)szSep + 1;
		field++;
	}
	strcpy((char*)field, tmp);
	return i+1;
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
		if(nCode != 104)
		{
			char szLog[MINSIZE] = { 0 };
			sprintf(szLog, "The send package is:%d %d", 8+nLen,
					nCode);
			LogFile::write_sys_log(szLog);
		}
#endif
		return 8;
	}
}

int extract_pack(void* buf, unsigned int& code, char* szContent)
{
	unsigned int len = ntohl(*(int*)buf);
	code = ntohl(*(int*)((unsigned char*)buf+4));
	if(len > 8)
	{
		memcpy(szContent, (unsigned char*)buf+8, len-8);
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The receive package is:%d %d %s", len,
				code, szContent);
		LogFile::write_sys_log(szLog);
#endif
	}
	else
	{
		strcpy(szContent, "");
#ifdef DEBUG
		char szLog[MINSIZE] = { 0 };
		sprintf(szLog, "The receive package is:%d %d,the content is:no!", len,
				code);
		LogFile::write_sys_log(szLog);
#endif
	}
	return len;
}

static pid_t* childpid = NULL; /* ptr to array allocated at run-time */
static int maxfd;  /* from our open_max(), {Prog openmax} */
long open_max(void)
{
	long openmax;
	struct rlimit r1;
	if((openmax = sysconf(_SC_OPEN_MAX))<0 || openmax == LONG_MAX)
	{
		if(getrlimit(RLIMIT_NOFILE, &r1) < 0)
			LogFile::write_sys_log("can't get file limit!");
		if(r1.rlim_max == RLIM64_INFINITY)
			openmax = 256;
		else
			openmax = r1.rlim_max;
	}
	return openmax;
}
FILE* popendroid(const char *cmdstring, const char *type)
{
    int     i, pfd[2];
    pid_t   pid;
    FILE    *fp;
    /* only allow "r" or "w" */
    if ((type[0] != 'r' && type[0] != 'w') || type[1] != 0) {
        errno = EINVAL;     /* required by POSIX.2 */
        return(NULL);
    }

    if (childpid == NULL) {     /* first time through */
        /* allocate zeroed out array for child pids */
        maxfd = open_max();
        if ( (childpid = (pid_t*)calloc(maxfd, sizeof(pid_t))) == NULL)
            return(NULL);
    }

    if (pipe(pfd) < 0)
        return(NULL);   /* errno set by pipe() */

    if ( (pid = fork()) < 0)
        return(NULL);   /* errno set by fork() */
    else if (pid == 0) {                            /* child */
        if (*type == 'r') {
            close(pfd[0]);
            if (pfd[1] != STDOUT_FILENO) {
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);
            }
        } else {
            close(pfd[1]);
            if (pfd[0] != STDIN_FILENO) {
                dup2(pfd[0], STDIN_FILENO);
                close(pfd[0]);
            }
        }
            /* close all descriptors in childpid[] */
        for (i = 0; i < maxfd; i++)
            if (childpid[ i ] > 0)
                close(i);

        execl(ANDROID_SHELL, "sh", "-c", cmdstring, (char *) 0);
        _exit(127);
    }
                                /* parent */
    if (*type == 'r') {
        close(pfd[1]);
        if ( (fp = fdopen(pfd[0], type)) == NULL)
            return(NULL);
    } else {
        close(pfd[0]);
        if ( (fp = fdopen(pfd[1], type)) == NULL)
            return(NULL);
    }
    childpid[fileno(fp)] = pid; /* remember child pid for this fd */
    return(fp);
}

int pclosedroid(FILE *fp)
{
    int     fd, stat;
    pid_t   pid;
    if (childpid == NULL)
        return(-1);     /* popen() has never been called */
    fd = fileno(fp);
    if ( (pid = childpid[fd]) == 0)
        return(-1);     /* fp wasn't opened by popen() */
    childpid[fd] = 0;
    if (fclose(fp) == EOF)
        return(-1);
    while (waitpid(pid, &stat, 0) < 0)
        if (errno != EINTR)
            return(-1); /* error other than EINTR from waitpid() */

    return(stat);   /* return child's termination status */
}

int execstream(const char *cmdstring, char *buf, int size)
{
	FILE* stream;
	stream = popen(cmdstring, "r");
	//stream = popendroid(cmdstring, "r");
	if(NULL == stream)
	{
		LogFile::write_sys_log("execute adb command failed!");
		strcpy(buf, "failed");
		return 1;
	}
	else
	{
		while(NULL != fgets(buf, size, stream))
		//while(NULL != fread(buf, sizeof(char), size, stream))
		{
		}
//#ifdef DEBUG
//		char szLog[MINSIZE] = { 0 };
//		sprintf(szLog, "popen command is:%s", buf);
//		LogFile::write_sys_log(szLog);
//#endif
		pclose(stream);
		//pclosedroid(stream);
		return 0;
	}
}

void trim(char* str, char trimstr)
{
	char* szTmp = str;
	while(*szTmp == ' ')
		szTmp++;
	strcpy(str, szTmp);
	int len = strlen(str);
	szTmp = str + len -1;
	char* szEnd = szTmp;
	while(*szTmp==' ' || *szTmp=='\n')
		szTmp--;
	if(szTmp != szEnd)
		*(szTmp+1) = '\0';
}

int add_pri(const char *pathname)
{
    struct stat statbuf;
    /* turn on set-group-ID and turn off group-execute */
    if (stat(pathname, &statbuf) < 0)
    	return 1;
    if (chmod(pathname, statbuf.st_mode|S_IXUSR|S_IXGRP|S_IXOTH|S_IREAD|S_IRGRP|S_IROTH
    		|S_IWUSR|S_IWGRP|S_IWOTH) < 0)
    	return 1;
    return 0;
}

int get_upgrade_result()
{
	char szPath[PATH_MAX] = { 0 };
	int nSuccess=0;
	sprintf(szPath, "%s%s", UPSETTINGPATH, UPUPDATEINI);
	RegTool::GetPrivateProfileInt(UPGRADE, SUCCESS, nSuccess, szPath, 0);
	return nSuccess;
}

void parse_code(int code, char* szBuf, int cltFd)
{
	try
	{
		char buf[BUFSIZ] = { 0 }; //数据传送的缓冲区
		char szPath[PATH_MAX] = { 0 };
		if(code == 50)
		{
//			char szTotalStorage[10] = { 0 };
//			char szFreeStorage[10] = { 0 };
//			GetStorageInfo(szTotalStorage, szFreeStorage);
		}
		else if(code == SOCKET_CODE_UPGRADEBOX)
		{
			get_current_path(szPath, sizeof(szPath));
			strcat(szPath, "up");
			systemdroid(szPath);
			//exit(0);
			int result = get_upgrade_result();
			if(result == 1)
			{
				int len = grap_pack(buf, code, "1");
				global_sock_srv.send_socket_packs(buf, len, cltFd);
			}
			else
			{
				int len = grap_pack(buf, code, "2");
				global_sock_srv.send_socket_packs(buf, len, cltFd);
			}
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
			int len = grap_pack(buf, code, NULL);
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
			char szApkPath[PATH_MAX] = { 0 };
			get_current_path(szPath, sizeof(szPath));
			sprintf(szApkPath, "%s%s", szPath, APK_DIR_NAME);
			int count = get_all_apk(szApkPath, apklist);
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
			char coninfo[100][MINSIZE];
			int nCount = extract_content_info(szBuf, coninfo, 100);
			//sprintf(sqltext, "%s, '%s'", coninfo[0], coninfo[1]);
	#ifdef DEBUG
			char szLog[MINSIZE] = { 0 };
			sprintf(szLog, "The code is:%d,The content's %d fields value is:", SOCKET_CODE_ADDBOXAPK, nCount);
			for(int i=0; i<nCount; i++)
				sprintf(szLog, "%s %s", szLog, coninfo[i]);
			LogFile::write_sys_log(szLog);
	#endif
			char szApkPath[PATH_MAX] = { 0 };
			char szContent[ROWSIZE] = { 0 };
			for(int i=0; i<nCount; i++)
			{
				sprintf(szApkPath, "%s/%s", APK_TEMP_PATH, coninfo[i]);
				if(!is_file_exist(szApkPath) || add_pri(szApkPath)!=0)
				{
		//			*(int*)buf = htonl(9);
		//			*(int*)(buf+4) = htonl(SOCKET_CODE_ADDBOXAPK);
		//			*(char*)(buf+8) = '2';
		//			global_sock_srv.send_socket_packs(buf, 9, cltFd);
					if(i == 0)
						sprintf(szContent, "%s_%s", coninfo[i], "2");
					else
						sprintf(szContent, "%s_%s_%s", szContent, coninfo[i], "2");
					continue;
	//				int nLen = grap_pack(buf, code, "2");
	//				global_sock_srv.send_socket_packs(buf, nLen, cltFd);
    //				return;
				}
				char szApkFullPath[PATH_MAX] = { 0 };
				get_current_path(szPath, sizeof(szPath));
				sprintf(szApkFullPath, "%s%s/%s", szPath, APK_DIR_NAME, coninfo[i]);
				//if(!global_sql_mgr.insert_sqlite_table(APP_DBTABLE_APKINFO, sqltext))
				if(rename(szApkPath, szApkFullPath) != 0)
				{
					remove(szApkPath);
		//			*(int*)buf = htonl(9);
		//			*(int*)(buf+4) = htonl(SOCKET_CODE_ADDBOXAPK);
		//			*(char*)(buf+8) = '2';
		//			global_sock_srv.send_socket_packs(buf, 9, cltFd);
					if(i == 0)
						sprintf(szContent, "%s_%s", coninfo[i], "2");
					else
						sprintf(szContent, "%s_%s_%s", szContent, coninfo[i], "2");
//					int nLen = grap_pack(buf, code, "2");
//					global_sock_srv.send_socket_packs(buf, nLen, cltFd);
				}
				else
				{
		//			*(int*)buf = htonl(9);
		//			*(int*)(buf+4) = htonl(SOCKET_CODE_ADDBOXAPK);
		//			*(char*)(buf+8) = '1';
		//			global_sock_srv.send_socket_packs(buf, 9, cltFd);
					if(i == 0)
						sprintf(szContent, "%s_%s", coninfo[i], "1");
					else
						sprintf(szContent, "%s_%s_%s", szContent, coninfo[i], "1");
//					int nLen = grap_pack(buf, code, "1");
//					global_sock_srv.send_socket_packs(buf, nLen, cltFd);
				}
			}
			int nLen = grap_pack(buf, code, szContent);
			global_sock_srv.send_socket_packs(buf, nLen, cltFd);
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
	#ifdef DEBUG
			LogFile::write_sys_log(szContent);
	#endif
			//get the rom version
			strcat(szContent, "2.0_");
//			memset(szCmdResult, 0, sizeof(szCmdResult));
//			strcpy(szCmdString, "cat /system/build.prop | grep \"ro.build.version.release\"");
//			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
//			if(szCmdResult[0] != '\0')
//			{
//				szTmp = strstr(szCmdResult, "=");
//				strcpy(szTemp, szTmp+1);
//				strcpy(szCmdResult, szTemp);
//				len = strlen(szCmdResult);
//				if(szCmdResult[len-1] == '\n')
//					szCmdResult[len-1] = '_';
//				else
//				{
//					szCmdResult[len] = '_';
//					len++;
//				}
//				strcat(szContent, szCmdResult);
//			}
//			else
//			{
//				strcat(szContent, " _");
//			}
	#ifdef DEBUG
			LogFile::write_sys_log(szContent);
	#endif
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
				if(szTmp != NULL)
					*szTmp = '\0';
				unsigned long space = atol(szCmdResult) / 1000;
				sprintf(szContent, "%s%ld_", szContent, space);
			}
			else
			{
				strcat(szContent, " _");
			}
	#ifdef DEBUG
			LogFile::write_sys_log(szContent);
	#endif
			//get the cpu frequence
			memset(szCmdResult, 0, sizeof(szCmdResult));
			strcpy(szCmdString, "cat /proc/cpuinfo | grep \"BogoMIPS\"");
			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
			//strcpy(szCmdResult, "BogoMIPS        : 119.10");
			if(szCmdResult[0] != '\0')
			{
				szTmp = strstr(szCmdResult, ":");
				strcpy(szTemp, szTmp+1);
				strcpy(szCmdResult, szTemp);
				trim(szCmdResult);
				szTmp = strstr(szCmdResult, " ");
				if(szTmp != NULL)
					*szTmp = '\0';
				float speed = atof(szCmdResult) / 1000;
				sprintf(szContent, "%s%.3f_", szContent, speed);
			}
			else
			{
				strcat(szContent, " _");
			}

	#ifdef DEBUG
			LogFile::write_sys_log(szContent);
	#endif

			//get the free memory space
//			int cap;
//			GetStorageInfo("/mnt/sdcard", &cap,  3);
//			sprintf(szContent, "%s%d", szContent, cap);

			char total[TINYSIZE] = { 0 };
			char free[TINYSIZE] = { 0 };
			GetStorageInfo(total, free, "/data");
			sprintf(szContent, "%s%s", szContent, free);
//			memset(szCmdResult, 0, sizeof(szCmdResult));
//			strcpy(szCmdString, "cat /proc/meminfo | grep \"MemFree\"");
//			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
//			//strcpy(szCmdResult, "BogoMIPS        : 119.10");
//			if(szCmdResult[0] != '\0')
//			{
//				szTmp = strstr(szCmdResult, ":");
//				strcpy(szTemp, szTmp+1);
//				strcpy(szCmdResult, szTemp);
//				trim(szCmdResult);
//				szTmp = strstr(szCmdResult, " ");
//				if(szTmp != NULL)
//					*szTmp = '\0';
//				unsigned long space = atol(szCmdResult) / 1000;
//				sprintf(szContent, "%s%ld", szContent, space);
//			}
//			else
//			{
//				sprintf(szContent, "%s%s", szContent, " ");
//			}

	#ifdef DEBUG
			LogFile::write_sys_log(szContent);
	#endif

			int nLen = grap_pack(buf, SOCKET_CODE_GETBOXINFO, szContent);
			global_sock_srv.send_socket_packs(buf, nLen, cltFd);
		}
		else if(code == SOCKET_CODE_GETWORKMODE)//delete the instruction
		{
		}
	}
	catch(...)
	{
		LogFile::write_sys_log("The exception happened!");
	}
}
