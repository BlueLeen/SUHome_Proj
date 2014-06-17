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

#define ROWSIZE 200

DeviceInfo::DeviceInfo()
:m_nCode(-1),m_nUsbNum(-1),m_nState(0),m_nFormat(0)
{
	// TODO Auto-generated constructor stub
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

