/*
 * TestCallback.h
 *
 * Created: 18/05/2020 12.11.15
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 


#ifndef TESTCALLBACK_H_
#define TESTCALLBACK_H_

#include "usbdefs.hpp"
#include "USBHost.hpp"

class TestCallback {
	
	public:
		TestCallback(USBHost* usb);
		
		static CallbackFunction CallbackWrapper(void* input, void* context)
		{
			TestCallback* self = reinterpret_cast<TestCallback*>(context);
			self->Callback(input);
		}
		
		void Callback(void* input);
	
	private:
		USBHost* usb_;
};


#endif /* TESTCALLBACK_H_ */