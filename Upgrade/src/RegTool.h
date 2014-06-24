/*
 * RegTool.h
 *
 *  Created on: Jun 23, 2014
 *      Author: leen
 */

#ifndef REGTOOL_H_
#define REGTOOL_H_

#include <stdio.h>

class RegTool {
public:
	RegTool();
	virtual ~RegTool();
	static bool WritePrivateProfileInt(char* lpAppName, char* lpKeyName, int Value, char* lpFileName);
	static bool GetPrivateProfileInt(char* lpAppName, char* lpKeyName, int& Value, char* lpFileName, int ValueDefault=0);
	static bool WritePrivateProfileString(char* lpAppName, char* lpKeyName, char* Value, char* lpFileName);
	static bool GetPrivateProfileString(char* lpAppName, char* lpKeyName, char* Value, char* lpFileName, char* lpDefaultValue=NULL);
private:
	static int GetIniKeyInt(char *title, char *key, char *filename);
	static char *GetIniKeyString(char *title, char *key, char *filename);
};

#endif /* REGTOOL_H_ */
