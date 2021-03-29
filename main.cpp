/*
 * PingPongRelease.cpp
 *
 * Created: 23/05/2020 11.44.37
 * Author : Nicklas Grunert (@github.com/LordSyFo)
 */ 

// Target CPU frequency
#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

extern "C"{
	#include "FreeRTOS.h"
	#include "task.h"
};

/* For some reason including this in main will work in every class */
#include "CPPOperators.h"

#include "Logger.hpp"

#include "TestCallback.hpp"

#include "USBHost.hpp"
#include "xboxdefs.hpp"

// Wrapper to use class method in task
void usbHostProcessWrapper(void* param)
{
	while(1){
		static_cast<USBHost*>(param)->Process();
	}
	vTaskDelete( NULL );
}

int main(void)
{
	USBHost usbHost;						// initializes in constructor
	
	TestCallback callbackClass(&usbHost);	// test callbacks from within class

	_delay_ms(1000);

	usbHost.AddCallback((CallbackFunction)&callbackClass.CallbackWrapper,&callbackClass);

	int retcode = xTaskCreate(usbHostProcessWrapper,(const signed char*)"USBHOSTTASK",512,&usbHost,tskIDLE_PRIORITY,NULL);

	vTaskStartScheduler();
	
	while(1);
	
	return 0;
}