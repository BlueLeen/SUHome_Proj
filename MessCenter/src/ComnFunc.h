#ifndef COMNFUNC_H_
#define COMNFUNC_H_
/*
 * The file contains common functions
 */
//#include <time.h>
#include <sys/time.h>

// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount()
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) != 0)
		return 0;
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

#endif
