/*
 * CLock.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: leen
 */

#include "CLock.h"

CLock::CLock() {
	// TODO Auto-generated constructor stub
	 pthread_mutex_init(&m_Mutex, NULL);
}

CLock::~CLock() {
	// TODO Auto-generated destructor stub
	pthread_mutex_destroy(&m_Mutex);
}

int CLock::Lock()
{
	int nRetCode = pthread_mutex_lock(&m_Mutex);
	return (nRetCode == 0);
}

int CLock::Unlock()
{
	 int nRetCode = pthread_mutex_unlock(&m_Mutex);
	 return (nRetCode == 0);
}
