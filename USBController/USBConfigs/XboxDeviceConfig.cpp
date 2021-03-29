/*
 * XboxDeviceConfig.cpp
 *
 * Created: 12/04/2020 08.59.07
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 


#define F_CPU 16000000

#include <util/delay.h>

#include "XboxDeviceConfig.hpp"
#include "Logger.hpp"
#include <stdlib.h>

#include "task.h"

#include "xboxdefs.hpp"

XboxDeviceConfig::XboxDeviceConfig(MAX3421E* max){
	
	max_ = max;
	nCallbackFunctions_ = 0;
	nCallbackContexts_ = 0;
	
	pid_ = 654;
	vid_ = 1118;

	rumble_ = false;
	vSemaphoreCreateBinary(rumbleSemaphore_);
	vSemaphoreCreateBinary(ledSemaphore_);
	
	/* Hardcode endpoints based on external analysis */
	inputEndpoint_.Interval = 4;
	inputEndpoint_.maxPktSize = 32;
	inputEndpoint_.epAddr = 1;
	inputEndpoint_.direction = 1;
	
	outputEndpoint_.Interval = 8;
	outputEndpoint_.maxPktSize = 32;
	outputEndpoint_.epAddr = 1;
	outputEndpoint_.direction = 0;

	/* Null initialize all callback functions */
	for (int i = 0; i < MAX_CALLBACK_FUNCTIONS; i++){
		callbackFunctions_[i] = NULL;
		callbackContexts_[i] = NULL;
	}
}

uint16_t XboxDeviceConfig::GetVid()
{
	return vid_;
}

uint16_t XboxDeviceConfig::GetPid()
{
	return pid_;
}

void XboxDeviceConfig::Process()
{
	PollInputs();
	PollRumbleRequest();
	PollLEDRequest();
}

void XboxDeviceConfig::OutputRequest(uint8_t requestType, void* params)
{
	/* Switch on request type */
	switch(requestType){
		case REQUEST_RUMBLE:
		{
			if (params == NULL) return;
			uint8_t* args = reinterpret_cast<uint8_t*>(params);
			RequestRumble(args[0],args[1]);
			break;
		}
		case REQUEST_LED:
		{
			if (params == NULL) return;
			uint8_t ledAnimation = *(reinterpret_cast<uint8_t*>(params));
			RequestLED(ledAnimation);
			break;
		}
		default:
			break;
	}
}

/* Checks if there has been any requests regarding LEDS */
void XboxDeviceConfig::PollLEDRequest()
{
	xSemaphoreTake(ledSemaphore_,portMAX_DELAY);
		
	if (led_){
		DoLEDAnimation();
			
		/* Reset activation variables */
		ledAnimation_	= 0;
		led_			= false;
	}
		
	xSemaphoreGive(ledSemaphore_);	
}

void XboxDeviceConfig::RequestLED(uint8_t ledAnimation)
{
	xSemaphoreTake(ledSemaphore_,portMAX_DELAY);
	ledAnimation_ = ledAnimation;
	led_ = true;
	xSemaphoreGive(ledSemaphore_);	
}

void XboxDeviceConfig::DoLEDAnimation()
{
	/* Parameters are used to determine the LED animation */
	uint8_t ledPacket[] = { LED_TYPE, 0x03, ledAnimation_};
		
	/* Transfer LED packet */
	uint8_t rcode = max_->OutTransfer(&outputEndpoint_,sizeof(ledPacket) / sizeof(ledPacket[0]),ledPacket,0);

	if (rcode){
		LOG_ERROR("Rcode: %d",rcode);
		return;
	}
	
}

/* Checks if there has been any requests regarding rumble */
void XboxDeviceConfig::PollRumbleRequest()
{
	xSemaphoreTake(rumbleSemaphore_,portMAX_DELAY);
	
	if (rumble_){
		DoRumbleController();
		
		/* Reset activation variables */
		rightRumble_	= 0;
		leftRumble_		= 0;
		rumble_			= false;
	}
	
	xSemaphoreGive(rumbleSemaphore_);
}

void XboxDeviceConfig::RequestRumble(uint8_t leftRumble, uint8_t rightRumble)
{
	xSemaphoreTake(rumbleSemaphore_,portMAX_DELAY);
	leftRumble_ = leftRumble;
	rightRumble_ = rightRumble;
	rumble_ = true;
	xSemaphoreGive(rumbleSemaphore_);
}

void XboxDeviceConfig::DoRumbleController()
{
	/* Parameters are used to determine the motorspeed for the two rumble motors in the controller */
	uint8_t rumblePacket[] = { RUMBLE_TYPE, 0x08, 0x00, leftRumble_, rightRumble_, 0x00, 0x00, 0x00 };
		
	/* Transfer rumble packet */
	uint8_t rcode = max_->OutTransfer(&outputEndpoint_,sizeof(rumblePacket) / sizeof(rumblePacket[0]),rumblePacket,0);

	if (rcode){
		LOG_ERROR("Rcode: %d",rcode);
		return;
	}
	
	/* Transfer reset rumble packet */
	vTaskDelay(200/portTICK_RATE_MS);
	uint8_t resetRumblePacket[] = { RUMBLE_TYPE, 0x08, 0x00, 0, 0, 0x00, 0x00, 0x00 };
	rcode = max_->OutTransfer(&outputEndpoint_,sizeof(resetRumblePacket) / sizeof(resetRumblePacket[0]),resetRumblePacket,0);

	if (rcode){
		LOG_ERROR("Rcode: %d",rcode);
		return;
	}
		
}

void XboxDeviceConfig::PollInputs()
{
	uint8_t fullPacket[64];	// Always transfers two packets so store both to save calling this method two times
	uint16_t nbytes = 64;
	
	/* Always transfers two packets */
	uint8_t rcode = max_->InTransfer(&inputEndpoint_,&nbytes,(uint8_t*)&fullPacket,inputEndpoint_.Interval,1);
	
	if (rcode == hrNAK)
	{
		/* No interrupts pending = no new information */
		return;
	}
	
	if (rcode==hrSUCCES)
	{
		/* Ignore packets that doesnt concern control key changes */
		if (fullPacket[PRIMARY_CONTROLKEYS_OFFSET] == inputRecord_.primaryKeys 
			&& fullPacket[SECONDARY_CONTROLKEYS_OFFSET] == inputRecord_.secondaryKeys)
			return;
		
		
		/* Update input structure - using only the first packet (we are not concerned about the keys being released) */
		inputRecord_.primaryKeys	= fullPacket[PRIMARY_CONTROLKEYS_OFFSET];
		inputRecord_.secondaryKeys	= fullPacket[SECONDARY_CONTROLKEYS_OFFSET];
		
		// Call callback functions
		for (int i = 0; i < MAX_CALLBACK_FUNCTIONS; i++){
			if (callbackFunctions_[i] != NULL){
				callbackFunctions_[i](&inputRecord_,callbackContexts_[i]);
			}
		}
		
		/*if (inputRecord_.primaryKeys & AKEY){
			uint8_t rumbles[] = {255,255};
			OutputRequest(REQUEST_RUMBLE,rumbles);
		}*/
	}

}

void XboxDeviceConfig::FlushInput()
{
	/* Read 3x64 byte packets (seems to be enough to clear the buffer) */
	for (int i = 0; i < 3; i++){
		uint8_t buffer[64];
		uint16_t nbytes = 64;
		uint8_t rcode = max_->InTransfer(&inputEndpoint_,&nbytes,(uint8_t*)&buffer,inputEndpoint_.Interval,1);
	}
}

bool XboxDeviceConfig::Configure(const DeviceRecord* record)
{
	/* Check if device is already configured */
	uint8_t byte = 0xff;
	uint8_t rcode = max_->GetConfiguration(record->devAddress,0,1,&byte);
	
	if (rcode == hrSUCCES){
		if (byte != 0)
			return true;
	}
	
	/* Get configuration descriptor - we need the configValue to enable the device*/
	uint8_t config_desc[sizeof(USB_CONFIGURATION_DESCRIPTOR)];
	USB_CONFIGURATION_DESCRIPTOR* configPtr = reinterpret_cast<USB_CONFIGURATION_DESCRIPTOR*>(config_desc);
	
	rcode = max_->GetConfigDescriptor(record->devAddress, 0, sizeof(USB_CONFIGURATION_DESCRIPTOR), (uint8_t*)&config_desc);

	if (rcode != hrSUCCES) return false;
	
	/* Enable configuration */
	LOG_DEBUG("Enabling configuration.");
	
	rcode = max_->SetConfiguration(record->devAddress,0,configPtr->bConfigurationValue);
	
	if (rcode != hrSUCCES) return false;
	
	vTaskDelay(100/portTICK_RATE_MS);
	
	FlushInput();	// Flush input
	
	LOG_DEBUG("Succesfully configured device!");
	
	return true;
}

void XboxDeviceConfig::AddCallback(CallbackFunction callback, void* context)
{
	if (nCallbackFunctions_ < MAX_CALLBACK_FUNCTIONS)
	{
		callbackFunctions_[nCallbackFunctions_++] = *callback;
		callbackContexts_[nCallbackContexts_++] = context;
	}
}

XboxDeviceConfig::~XboxDeviceConfig(){
	
}
