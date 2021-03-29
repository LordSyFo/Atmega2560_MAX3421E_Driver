/*
 * USBHost.h
 *
 * Created: 08/04/2020 19.32.10
 *  Author: hejmi
 */ 


#ifndef USBHOST_H_
#define USBHOST_H_

#include "MAX3421E.hpp"
#include "IDeviceConfig.hpp"

#define MAX_DEVICE_CFGS 1

class USBHost {
	
public:
	USBHost();
	~USBHost();
	
	/**
	*	Adds supported device configs, initializes the max3421 and resets member attributes.
	*	@return True if initialization was successful, false otherwise
	*/
	bool Initialize();
	
	/**
	*	Process to be run continuously. Enumerates devices.
	*/
	void Process();
	
	/**
	*	Adds a supported device configuration to member-list of supported devices.
	*	@param config	Pointer to configuration to be added
	*/
	void AddDeviceConfig(IDeviceConfig* config);
	
	/**
	*	Searches list of supported devices to find matching PID and VID.
	*	@param vid	VID to search for
	*	@param pid	PID to search for
	*	@return		Pointer to matching config, NULL if no match was found.
	*/
	IDeviceConfig* FindMatchingCfg(uint16_t vid, uint16_t pid);
	
	/**
	*	Adds a callback function to queue. Callback functions are queued so that they can be attached to
	*	the correct device config when they are found.
	*	@param	callback	Callback-function to be added
	*	@param	context		Context to be passed to the callback (useful when using class methods as callbacks)
	*/
	void AddCallback(CallbackFunction callback, void* context);
	
	/**
	*	Do output request to active device configuration.
	*	@param	requestType	Type of request (See macros under *Output request types* in active device config)
	*	@param	params		Array of parameters if any should be used in the request.
	*/
	void OutputRequest(uint8_t requestType, void* params);
	
	/**
	*	Gets the MAX3421 instance used.
	*	@return	Pointer to saved MAX3421 instance.
	*/
	MAX3421E* GetMax() {return &max_;}
	
private:
	MAX3421E max_;
	
	IDeviceConfig* deviceConfigs_[MAX_DEVICE_CFGS];
	
	uint8_t state_;
	IDeviceConfig* activeConfig_;
	
	const DeviceRecord* currentRecord_;
	
	CallbackFunction callbackFunctionsQueue_[MAX_CALLBACK_FUNCTIONS];
	void* contextQueue_[MAX_CALLBACK_FUNCTIONS];
	uint8_t nCallbackFunctionsQueue_;
	void AddCallbacksToConfig();
	
};


#endif /* USBHOST_H_ */