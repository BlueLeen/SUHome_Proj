/*
 * LogFile.h
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#ifndef LOGFILE_H_
#define LOGFILE_H_

class LogFile {
public:
	LogFile();
	virtual ~LogFile();

//	static void write_sys_log(char* szWriteString, char* szFile);
//	static void write_sys_log(int nWrite, char* szFile);
	static void write_sys_log(char* szWriteString);
	static void write_sys_log(int nWrite);
private:
	static const int SIZE=100;
};

#endif /* LOGFILE_H_ */
