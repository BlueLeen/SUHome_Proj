#ifndef _DEVICESTATE_FILE__
#define _DEVICESTATE_FILE__

typedef struct _plgdevicestate
{
	int state;//1:plug in 2:pull out
	int devcode;//1:phone
}plgdevicestate;

unsigned long plug_dev_detect();





#endif
