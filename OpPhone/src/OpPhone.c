/*
 ============================================================================
 Name        : OpPhone.c
 Author      : leen
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define MAXSIZE 1024
#define ROWSIZE 200
#define APK_DIR_NAME "ApkDir"
#define ADB_DIR_NAME "Adb"
#define CFG_DIR_NAME "cfg"
#define CFG_FILE_NAME "setting"
#define CFG_SEC_TIME "Time"

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

//从INI文件读取整类型数据
int GetIniKeyInt(char *title, char *key, char *filename) {
	return atoi(GetIniKeyString(title, key, filename));
}

int install_android_apk(char* szApk)
{
	char shellComm[MAXSIZE] = { 0 };
	char szPath[MAXSIZE] = { 0 };
	char szApkPath[MAXSIZE] = { 0 };
	char szAdbPath[MAXSIZE] = { 0 };
	get_current_path(szPath, MAXSIZE);
	sprintf(szApkPath, "%s%s/%s", szPath, APK_DIR_NAME, szApk);
	sprintf(szAdbPath, "%s%s/%s", szPath, ADB_DIR_NAME, "adb");
	sprintf(shellComm, "%s install %s", szAdbPath, szApkPath);
	system(shellComm);
	return 0;
}

char* get_download_time()
{
	char szPath[MAXSIZE] = { 0 };
	get_current_path(szPath, MAXSIZE);
	sprintf(szPath, "%s%s/%s", szPath, CFG_DIR_NAME, CFG_FILE_NAME);
	char* szTime;
	szTime= GetIniKeyString(CFG_SEC_TIME, "lastdownload", szPath);
	return GetIniKeyString(CFG_SEC_TIME, "lastdownload", szPath);
}


int main(int argc, char* argv[]) {
	//get_download_time();
	if(argc<2 || !strcmp(argv[1], "--help"))
	{
		printf("The arguments is fault.The command should be \"OpPhone -op apk\" \n");
	}
	if(!strcmp(argv[1], "-i") || !strcmp(argv[1], "--install"))
		install_android_apk(argv[2]);
	//if(!strcmp(argv[1], "-l") || !strcmp(argv[1], "--last"))

	return EXIT_SUCCESS;
}
