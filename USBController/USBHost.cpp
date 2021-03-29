/*
 * USBHost.cpp
 *
 * Created: 08/04/2020 19.40.35
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 

#include "USBHost.hpp"
#include <stddef.h>
#include "usbhostdefs.hpp"
#include <assert.h>
#include "Logger.hpp"
#include "XboxDeviceConfig.hpp"

// default constructor
USBHost::USBHost()
{
	Initialize();
	state_ = HOST_DISCONNECTED;	// setup state machine
}

void USBHost::OutputRequest(uint8_t requestType, void* params)
{
	if (activeConfig_ != NULL){
		activeConfig_->OutputRequest(requestType,params);
	}
}

void USBHost::AddCallback(CallbackFunction callback, void* context)
{
	/* Add to callback functions queue */
	if (nCallbackFunctionsQueue_ < MAX_CALLBACK_FUNCTIONS){
		callbackFunctionsQueue_[nCallbackFunctionsQueue_] = *callback;
		contextQueue_[nCallbackFunctionsQueue_++] = context;
	}
}

void USBHost::AddCallbacksToConfig(){
	assert(activeConfig_ != NULL);	// should never happen as this function is called after activeConfig is updated.

	for (int i = 0; i < MAX_CALLBACK_FUNCTIONS; i++)
		if (callbackFunctionsQueue_[i] != NULL){
			activeConfig_->AddCallback(callbackFunctionsQueue_[i],contextQueue_[i]);
		}
}

bool USBHost::Initialize()
{
	/* Initialize all device configs to NULL */
	for (int i = 0; i < MAX_DEVICE_CFGS; i++)
		deviceConfigs_[i] = NULL;
	
	/* Initialize MAXDevice */
	max_.Initialize();
	
	/* Add supported USB devices */
	AddDeviceConfig(new XboxDeviceConfig(&max_));
	
	/* Null initialize all callback functions in queue */
	nCallbackFunctionsQueue_ = 0;
	for (int i = 0; i < MAX_CALLBACK_FUNCTIONS; i++){
		callbackFunctionsQueue_[i] = NULL;
		contextQueue_[i] = NULL;
	}
	
	activeConfig_ = NULL;
	
	return true;
}

void USBHost::Process()
{
	switch (state_){
		
		case(HOST_DISCONNECTED):
		{
			/* Enumerate until we find a matching device */
			max_.Enumerate();
					
			/* If device has been found, find matching config */
			if (max_.GetUSBState() == USB_CONFIGURING){
				state_ = HOST_DEVICE_FOUND;		// Device has been found and needs to be configured!	
			}
			break;
		}
		case(HOST_DEVICE_FOUND):
		{
			currentRecord_  = max_.GetActiveDevRecord();
			
			if (currentRecord_ == NULL) return;
			if (currentRecord_->devDescriptor->bNumConfigurations <= 0) return;	// cant configure device with no configuration
							
			IDeviceConfig* cfg = FindMatchingCfg(currentRecord_->devDescriptor->idVendor, currentRecord_->devDescriptor->idProduct);
							
			if (cfg == NULL) return;
							
			activeConfig_ = cfg;
			state_ = HOST_CONFIG_FOUND;	// Found matching config -> go configure device
			
			break;
		}
		case(HOST_CONFIG_FOUND):
		{
			assert(activeConfig_ != NULL);
			
			/* Attempt to configure device */
			if (activeConfig_->Configure(currentRecord_)){
				state_ = HOST_DEVICE_CONFIGURED;
			}
			break;
		}
		case(HOST_DEVICE_CONFIGURED):
		{
			// Start running device
			state_ = HOST_DEVICE_RUNNING;
			max_.SetUSBState(USB_RUNNING);
			AddCallbacksToConfig();	// add queued callback functions to config
			// no break here so we can start running right away
		}
		case(HOST_DEVICE_RUNNING):
		{
			assert(activeConfig_ != NULL);
			
			activeConfig_->Process();
			// TODO: Should enumerate here to make sure the device hasn't disconnected
			break;
		}
	}
	 
}

IDeviceConfig* USBHost::FindMatchingCfg(uint16_t vid, uint16_t pid)
{
	/* Loop through added devices */
	for (int i = 0; i < MAX_DEVICE_CFGS; i++){
		if (deviceConfigs_[i] == NULL) continue;
		
		if (deviceConfigs_[i]->GetPid() == pid && deviceConfigs_[i]->GetVid() == vid)
			return deviceConfigs_[i];
	}
	
	return NULL;
}

void USBHost::AddDeviceConfig(IDeviceConfig* config)
{
	for (int i = 0; i < MAX_DEVICE_CFGS; i++){
		if (deviceConfigs_[i] == NULL){
			deviceConfigs_[i] = config;
			return;
		}
	}
}

USBHost::~USBHost(){
	
}