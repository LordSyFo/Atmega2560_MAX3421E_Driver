/*
 * TestCallback.cpp
 *
 * Created: 18/05/2020 12.11.44
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 
#include "TestCallback.hpp"
#include "Logger.hpp"
#include "xboxdefs.hpp"

TestCallback::TestCallback(USBHost* usb)
{
	usb_ = usb;
}

void TestCallback::Callback(void* input){
	
	XBOXInputRecord* keys = reinterpret_cast<XBOXInputRecord*>(input);
		
	if (keys->primaryKeys & XKEY)
		LOG_INFO("X was pressed!");
	if (keys->primaryKeys & BKEY)
		LOG_INFO("B was pressed!");
	if (keys->secondaryKeys & LEFTKEY)
		LOG_INFO("LEFT was pressed!");
	if (keys->secondaryKeys & RIGHTKEY)
		LOG_INFO("RIGHT was pressed!");
	
	if (keys->primaryKeys & AKEY){
		uint8_t rumbleStrengths[] = {255,255};
		usb_->OutputRequest(REQUEST_RUMBLE,rumbleStrengths);
	}
	
	if (keys->primaryKeys & YKEY){
		uint8_t ledAnimation[] = {LED_ROTATING};
		usb_->OutputRequest(REQUEST_LED,ledAnimation);
	}
	
}
