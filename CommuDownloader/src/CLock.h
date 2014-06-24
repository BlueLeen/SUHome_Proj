/*
 * CLock.h
 *
 *  Created on: Jun 24, 2014
 *      Author: leen
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <pthread.h>

class CLock {
public:
	CLock();
	virtual ~CLock();
	int Lock();
	int Unlock();
private:
    pthread_mutex_t m_Mutex;
};

#endif /* CLOCK_H_ */
