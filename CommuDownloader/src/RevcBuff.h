/*
 * RevcBuff.h
 *
 *  Created on: 2014年12月12日
 *      Author: su
 */

#ifndef REVCBUFF_H_
#define REVCBUFF_H_

#include "CLock.h"

class RevcBuff {
public:
	RevcBuff();
	virtual ~RevcBuff();
	int putData(const char* buf, int len);
	bool getData(char* buf, int& len);
	void resetData();
private:
	int cursize;
	CLock bufLock;
	static const int BUFFERSIZE = 8196;
	char buffer[BUFFERSIZE];
};

#endif /* REVCBUFF_H_ */
