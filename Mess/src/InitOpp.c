#include <stdio.h>
#include "InitOpp.h"
#include "DeviceState.h"

extern void write_sys_log(char* szWriteString);
typedef void (*pGlobalCallBackFuc)(int code, void *pStruc);
pGlobalCallBackFuc global_callback_func;

int Init(void *pfunc)
{
	global_callback_func = (pGlobalCallBackFuc)pfunc;
	if(global_callback_func == NULL)
	{
		write_sys_log("Init failed!");
		return 1;
	}
	else
	{
		plug_dev_detect();
		return 0;
	}
}

