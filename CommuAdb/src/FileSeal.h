#ifndef _FILE_SEAL__0X20140424100450
#define _FILE_SEAL__0X20140424100450

#include <stdio.h>

#define MAXSIZE   1024

//获取当前目录路径
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

//写字符串到当前目录下的的指定文件
void write_sys_log(char* szWriteString, char* szFile)
{
	char szFilePath[MAXSIZE];
	FILE* fp;
	char szTime[50];
	time_t ti;
	get_current_path(szFilePath, MAXSIZE);
	sprintf(szFilePath, "%s%s", szFilePath, szFile);
	fp = fopen(szFilePath, "a+");
	time(&ti);
	struct tm* ptm = localtime(&ti);
	sprintf(szTime, "当前时间为:%d/%d/%d %d:%d:%d", ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(fp,"%s>>----- %s\n", szTime, szWriteString);
	fclose(fp);
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

#endif
