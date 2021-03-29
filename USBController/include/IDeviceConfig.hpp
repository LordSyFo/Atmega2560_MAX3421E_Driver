/*
 * IDeviceConfig.h
 *
 * Created: 08/04/2020 19.23.32
 *  Author: hejmi
 */ 


#ifndef IDEVICECONFIG_H_
#define IDEVICECONFIG_H_

#include "usbdefs.hpp"

#define MAX_CALLBACK_FUNCTIONS 2

class IDeviceConfig {

public:
	virtual uint16_t GetVid() = 0;
	virtual uint16_t GetPid() = 0;

	virtual void Process() = 0;
	virtual bool Configure(const DeviceRecord* record) = 0;

	virtual void AddCallback(CallbackFunction callback, void* context) = 0;
	virtual void OutputRequest(uint8_t requestType, void* params) = 0;
	
};


#endif /* IDEVICECONFIG_H_ */