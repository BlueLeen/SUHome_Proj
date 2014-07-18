/*
 * DeviceInfo.cpp
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#include "DeviceInfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define ROWSIZE 400

extern bool is_file_exist(const char *path);
extern int execstream(const char *cmdstring, char *buf, int size);
extern void trim(char* str, char trimstr=' ');

int replacestr(char *sSrc, char *sMatchStr, char *sReplaceStr)
{
        int  StringLen;
        char caNewString[ROWSIZE];

        char *FindPos = strstr(sSrc, sMatchStr);
        if( (!FindPos) || (!sMatchStr) )
                return -1;

        while( FindPos )
        {
                memset(caNewString, 0, sizeof(caNewString));
                StringLen = FindPos - sSrc;
                strncpy(caNewString, sSrc, StringLen);
                strcat(caNewString, sReplaceStr);
                strcat(caNewString, FindPos + strlen(sMatchStr));
                strcpy(sSrc, caNewString);

                FindPos = strstr(sSrc, sMatchStr);
        }

        return 0;
}

DeviceInfo::DeviceInfo()
:m_nCode(-1),m_nUsbNum(-1),m_nState(0),m_nFormat(0)
{
	// TODO Auto-generated constructor stub
	m_szVid[0] = ' ';
	m_szPid[0] = ' ';
	m_szManFac[0] = ' ';
	m_szProduct[0] = ' ';
	m_szImei[0] = ' ';
}

DeviceInfo::~DeviceInfo() {
	// TODO Auto-generated destructor stub
}

void DeviceInfo::get_dev_info(char* buf)
{
	char szInfo[ROWSIZE] = { 0 };
	int nPos = 0;
	//if(buf[0] == '\0') return;
	while(buf[nPos] != '\0')
	{
		int nRowPos = 0;
		memset(szInfo, 0, sizeof(szInfo));
		while(buf[nPos] != '\n')
		{
			szInfo[nRowPos] = buf[nPos];
			nRowPos++;
			nPos++;
		}
		if(szInfo[0]!='\0' && szInfo[0]!='\n' && szInfo[0]!=' ')
		{
			char* szTmp;
			if(szInfo[0]=='T' && (szTmp=strstr(szInfo,"Port"))!=NULL)
			{
				char* szTmpVal = szTmp + strlen("Port") + 1;
				char szRealVal[10] = { 0 };
				int i = 0;
				int j = 0;
				while(*(szTmpVal+i) == ' ')
				{
					i++;
					continue;
				}
				while(*(szTmpVal+i) != ' ')
				{
					szRealVal[j++] = *(szTmpVal+i);
					i++;
				}
				m_nUsbNum = atoi(szRealVal);
			}
			else if(szInfo[0]=='P' && (szTmp=strstr(szInfo,"Vendor"))!=NULL)
			{
				char* szTmpVenVal = szTmp + strlen("Vendor") + 1;
				char* szTmpProSec = strstr(szInfo,"ProdID");
				char* szTmpProVal = szTmpProSec + strlen("ProdID") + 1;
				char szRealVal[10] = { 0 };
				int i = 0;
				int j = 0;
				while(*(szTmpVenVal+i) == ' ')
				{
					i++;
					continue;
				}
				while(*(szTmpVenVal+i) != ' ')
				{
					szRealVal[j++] = *(szTmpVenVal+i);
					i++;
				}
				strcpy(m_szVid, szRealVal);
				memset(szRealVal, 0, sizeof(szRealVal));
				int k = 0;
				int h = 0;
				while(*(szTmpProVal+k) == ' ')
				{
					k++;
					continue;
				}
				while(*(szTmpProVal+k) != ' ')
				{
					szRealVal[h++] = *(szTmpProVal+k);
					k++;
				}
				strcpy(m_szPid, szRealVal);
			}
			else if(szInfo[0]=='S' && (szTmp=strstr(szInfo,"Manufacturer"))!=NULL)
			{
				char* szTmpManVal = szTmp + strlen("Manufacturer") + 1;
				char szRealVal[10] = { 0 };
				int i = 0;
				int j = 0;
				while(*(szTmpManVal+i) == ' ')
				{
					i++;
					continue;
				}
				while(*(szTmpManVal+i) != ' ')
				{
					szRealVal[j++] = *(szTmpManVal+i);
					if(szRealVal[j] == '_')
						szRealVal[j] = '-';
					i++;
				}
				strcpy(m_szManFac, szRealVal);
			}
			else if(szInfo[0]=='S' && (szTmp=strstr(szInfo,"Product"))!=NULL)
			{
				char* szTmpProVal = szTmp + strlen("Product") + 1;
				char szRealVal[10] = { 0 };
				int i = 0;
				int j = 0;
				while(*(szTmpProVal+i) == ' ')
				{
					i++;
					continue;
				}
				while(*(szTmpProVal+i) != ' ')
				{
					szRealVal[j++] = *(szTmpProVal+i);
					if(szRealVal[j] == '_')
						szRealVal[j] = '-';
					i++;
				}
				strcpy(m_szProduct, szRealVal);
			}
			else if(szInfo[0]=='S' && (szTmp=strstr(szInfo,"SerialNumber"))!=NULL)
			{
				char* szTmpSerVal = szTmp + strlen("SerialNumber") + 1;
				char szRealVal[10] = { 0 };
				int i = 0;
				int j = 0;
				while(*(szTmpSerVal+i) == ' ')
				{
					i++;
					continue;
				}
				while(*(szTmpSerVal+i) != ' ')
				{
					szRealVal[j++] = *(szTmpSerVal+i);
					if(szRealVal[j] == '_')
						szRealVal[j] = '-';
					i++;
				}
				strcpy(m_szImei, szRealVal);
			}
		}
		nPos++;
	}
}

void DeviceInfo::get_dev_info(char* buf, const char* path)
{
	char szPath[PATH_MAX] = { 0 };
	if(!is_file_exist(path))
		return;
	else
	{
		char szCmdString[ROWSIZE] = { 0 };
		char szCmdResult[ROWSIZE] = { 0 };
		//get the idVendor
		snprintf(szPath, sizeof(szPath), "%s/idVendor", path);
		snprintf(szCmdString,  sizeof(szCmdString), "cat %s", szPath);
		if(is_file_exist(szPath))
		{
			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
			trim(szCmdResult);
			strncpy(m_szVid, szCmdResult, sizeof(m_szVid));
		}
		//get the idProduct
		snprintf(szPath, sizeof(szPath), "%s/idProduct", path);
		snprintf(szCmdString,  sizeof(szCmdString), "cat %s", szPath);
		if(is_file_exist(szPath))
		{
			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
			trim(szCmdResult);
			strncpy(m_szPid, szCmdResult, sizeof(m_szPid));
		}
		//get the manufacturer
		snprintf(szPath, sizeof(szPath), "%s/manufacturer", path);
		snprintf(szCmdString,  sizeof(szCmdString), "cat %s", szPath);
		if(is_file_exist(szPath))
		{
			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
			trim(szCmdResult);
			replacestr(szCmdResult, "_", "###");
			strncpy(m_szManFac, szCmdResult, sizeof(m_szManFac));
		}
		//get the product
		snprintf(szPath, sizeof(szPath), "%s/product", path);
		snprintf(szCmdString,  sizeof(szCmdString), "cat %s", szPath);
		if(is_file_exist(szPath))
		{
			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
			trim(szCmdResult);
			replacestr(szCmdResult, "_", "###");
			strncpy(m_szProduct, szCmdResult, sizeof(m_szProduct));
		}
		//get the imei
		snprintf(szPath, sizeof(szPath), "%s/serial", path);
		snprintf(szCmdString,  sizeof(szCmdString), "cat %s", szPath);
		if(is_file_exist(szPath))
		{
			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
			trim(szCmdResult);
			replacestr(szCmdResult, "_", "###");
			strncpy(m_szImei, szCmdResult, sizeof(m_szImei));
		}
//		//get the imei
//		sprintf(szPath, "%s/serial", path);
//		sprintf(szCmdString, "cat %s", szPath);
//		if(is_file_exist(szPath))
//		{
//			execstream(szCmdString, szCmdResult, sizeof(szCmdResult));
//			trim(szCmdResult);
//			strcpy(m_szImei, szCmdResult);
//		}
	}
}

