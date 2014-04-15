/*
 ============================================================================
 Name        : Hello.c
 Author      : leen
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>

#define DEV_ANDROID_FILE "/etc/udev/rules.d/50-android.rules"
#define ROWSIZE 200


_Bool venidIsExsit(FILE *fp, char *szVendor)
{
	//int i;
	char szRow[ROWSIZE] = {0};
	while(fgets(szRow, ROWSIZE, fp) != NULL && szRow[0] != '\n')
	{
		int i;
		//int nLen = strlen(szRow);
		//szRow[nLen - 1] = '\0';
		//printf("%s %d\n", szRow, nLen-1);
		char szVenTmp[10] = {0};
		for(i=0; i<4; i++)
		{
			//printf("%s\n", (char*)szRow+36+i);
			//szVenTmp[i] = *((char*)szRow+36+i);
			szVenTmp[i] = szRow[36+i];
			//printf("%c %c \n", *((char*)szRow+36+i), szRow[36+i]);
		}
		if(!strcmp(szVenTmp, szVendor))
		{
			return 1;
		}
	}
	return 0;
}

char* formOneRow(const char *szVendor, const char *szUser, char *szRow, int nCount)
{
	const char szForward[]="SUBSYSTEMS==\"usb\",ATTRS{idVendor}==\"";
	const char szImmediate[]="\",SYMLINK+=\"android_adb\",OWNER=\"";
	const char szBack[]="\"\n";
	sprintf(szRow, "%s%s%s%s%s", szForward, szVendor, szImmediate, szUser, szBack);
	//printf("%s\n", szRow);
	return szRow;
}

int main(int argc, char *argv[]) {
	FILE *fp;
	struct passwd *pwd;
	char szOneRow[ROWSIZE] = {0};
	//printf("%s\n", getpwuid(getuid())->pw_name);
	if(argc < 2)
	{
		printf("Arguments count fault!\n");
		return EXIT_FAILURE;
	}
	if((fp = fopen(DEV_ANDROID_FILE, "a+")) == NULL)
	{
		printf("Can not open file!\n");
	}
	//memcpy(szOneRow, 0, sizeof(szOneRow));
	if(!venidIsExsit(fp, argv[1]))
	{
		pwd = getpwuid(getuid());
		formOneRow(argv[1], pwd->pw_name, szOneRow, sizeof(szOneRow));
		//printf("%s\n", szOneRow);
		fputs(szOneRow, fp);
	}
	fclose(fp);
	//puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}
