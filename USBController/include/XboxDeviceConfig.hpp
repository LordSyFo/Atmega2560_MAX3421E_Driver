/*
 * XboxDeviceConfig.h
 *
 * Created: 08/04/2020 20.35.26
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 


#ifndef XBOXDEVICECONFIG_H_
#define XBOXDEVICECONFIG_H_

#include "MAX3421E.hpp"
#include "IDeviceConfig.hpp"
#include "xboxdefs.hpp"

#include "FreeRTOS.h"
#include "semphr.h"

class XboxDeviceConfig : public IDeviceConfig{
	
public:
	XboxDeviceConfig(MAX3421E* max);
	virtual ~XboxDeviceConfig();

	/**
	* Flushes the input buffer in xbox-controller, by reading what ever is in there on startup.
	*/
	void FlushInput();

	/**
	*	Polls data from input endpoint responsible for keypresses.
		If theres new data available it reads this and passes it to callback functions,
		saved in callbackFunctions_.
	*/
	void PollInputs();
	
	/**
	*	Polls requests from USBHost to rumble controller.
	*/
	void PollRumbleRequest();
	
	/**
	*	Polls requests from USBHost to do LED animation.
	*/
	void PollLEDRequest();
	
	/**
	*	Request a rumble with given motorspeeds
	*	@param leftRumble	Motorspeed for left motor
	*	@param rightRumble	Motorspeed for right motor
	*/
	void RequestRumble(uint8_t leftRumble, uint8_t rightRumble);
	
	/**
	*	Rumble the controller with saved motor speeds in leftRumble_ and rightRumble_
	*/
	void DoRumbleController();

	/**
	*	Request a specific animation for the LEDs
	*	@param ledAnimation		Animation to be uploaded to the LEDs
	*/
	void RequestLED(uint8_t ledAnimation);
	
	/**
	*	Upload the LED animation found in ledAnimation_
	*/
	void DoLEDAnimation();
	
	/**
	*	Get the VID specific for Xbox-360 Controllers
	*	@return		VID for Xbox-360 controllers
	*/
	virtual uint16_t GetVid();
	
	/**
	*	Get the PID specific for Xbox-360 Controllers
	*	@return		PID for Xbox-360 controllers
	*/
	virtual uint16_t GetPid();

	/**
	*	Process to be run continously after configuration.
		Polls inputs and requests.
	*/
	virtual void Process();
	
	/**
	*	Configures the Xbox-360 Controller and enables it.
	*	@param	record	Device record passed from USBHost obtained under enumeration
	*	@return	True if device was configures succesfully, false otherwise
	*/
	virtual bool Configure(const DeviceRecord* record);
	
	/**
	*	Adds a callback function to be called for changes in the input endpoint (keypresses).
	*	@param	callback	Callback-function to be added
	*	@param	context		Context to be passed to the callback (useful when using class methods as callbacks)
	*/
	virtual void AddCallback(CallbackFunction callback,void* context);
	
	/**
	*	Request an output action, could be rumble or led.
	*	@param	requestType	Type of request (See macros under *Output request types* in xboxdefs.hpp)
	*	@param	params		Array of parameters if any should be used in the request.
	*/
	virtual void OutputRequest(uint8_t requestType, void* params);
	
private:
	MAX3421E* max_;
	
	int pid_;
	int vid_;
	
	EpInfo inputEndpoint_;
	EpInfo outputEndpoint_;
	
	XBOXInputRecord inputRecord_;
	
	CallbackFunction callbackFunctions_[2];
	uint8_t nCallbackFunctions_;
	void* callbackContexts_[2];
	uint8_t nCallbackContexts_;
	
	xSemaphoreHandle rumbleSemaphore_;
	bool rumble_;			// used to activate rumble
	uint8_t leftRumble_;
	uint8_t rightRumble_;
	
	xSemaphoreHandle ledSemaphore_;
	uint8_t ledAnimation_;
	bool led_;				// used to activate led animation
	
};


#endif /* XBOXDEVICECONFIG_H_ */