/* *
 *@file             inifile.h
 *@cpright        (C)2007 GEC
 *@auther        dengyangjun
 *@email        dyj057@gmail.com
 *@version        0.1
 *@create         2007-1-14
 *@modify        2007-1-14
 *@brief            declare ini file operation
 *@note
 *@history
 */

#ifndef INI_FILE_H_
#define  INI_FILE_H_

//#ifdef __cplusplus
//extern "C" {
//#endif

int read_profile_string(const char * section, const char * key, char * value,
		int size, const char * file);
int read_profile_int(const char * section, const char * key, int default_value,
		const char * file);

int write_profile_string(const char * section, const char * key,
		const char * value, const char * file);

//#ifdef __cplusplus
//}
//;
//// end of extern "C" {
//#endif

#endif   // end of INI_FILE_H_
