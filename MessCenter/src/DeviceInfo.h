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
	int m_nCode;//the value equal the asyn fuction the first argument value
	char m_szVid[5];//the vendor id
	char m_szPid[5];//the product id
	char m_szManFac[30];// the manufacturer
	char m_szImei[10];//the imei number
	int m_nUsbNum;//the usb serial number
	int m_nState;//the state of the device(plug in/pull out)
	int m_nFormat;//the storage format(such as FAT32/NTFS/VFAT)
};

#endif /* DEVICEINFO_H_ */
