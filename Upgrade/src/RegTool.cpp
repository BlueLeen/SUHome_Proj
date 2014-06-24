/*
 * RegTool.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: leen
 */

#include "RegTool.h"
#include <stdlib.h>
#include <string.h>

RegTool::RegTool() {
	// TODO Auto-generated constructor stub

}

RegTool::~RegTool() {
	// TODO Auto-generated destructor stub
}

bool RegTool::WritePrivateProfileInt(char* lpAppName, char* lpKeyName, int Value, char* lpFileName)
{
	return true;
}

bool RegTool::GetPrivateProfileInt(char* lpAppName, char* lpKeyName, int& Value, char* lpFileName, int ValueDefault)
{
	Value = GetIniKeyInt(lpAppName, lpKeyName, lpFileName);
	return true;
}

bool RegTool::WritePrivateProfileString(char* lpAppName, char* lpKeyName, char* Value, char* lpFileName)
{
	return true;
}

bool RegTool::GetPrivateProfileString(char* lpAppName, char* lpKeyName, char* Value, char* lpFileName, char* lpDefaultValue)
{
	char* szTmp = GetIniKeyString(lpAppName, lpKeyName, lpFileName);
	strcpy(Value, szTmp);
	return true;
}

//从INI文件读取字符串类型数据
char* RegTool::GetIniKeyString(char *title, char *key, char *filename) {
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
int RegTool::GetIniKeyInt(char *title, char *key, char *filename) {
	return atoi(GetIniKeyString(title, key, filename));
}

