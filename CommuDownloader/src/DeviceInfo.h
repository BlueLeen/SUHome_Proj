/*
 * DeviceInfo.h
 *
 *  Created on: May 29, 2014
 *      Author: leen
 */

#ifndef DEVICEINFO_H_
#define DEVICEINFO_H_

class DeviceInfo {
public:
	DeviceInfo();
	virtual ~DeviceInfo();
	void get_dev_info(char* buf);
	void get_dev_info(char* buf, const char* path);
	int m_nCode;//the value equal the asyn fuction the first argument value
	char m_szVid[10];//the vendor id
	char m_szPid[10];//the product id
	char m_szManFac[100];// the manufacturer
	char m_szProduct[100];// the product
	char m_szImei[100];//the imei number
	int m_nUsbNum;//the usb serial number
	int m_nState;//the state of the device(plug in/pull out)
	int m_nFormat;//the storage format(such as FAT32/NTFS/VFAT)
};

#endif /* DEVICEINFO_H_ */
