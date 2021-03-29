/*
 * Logger.cpp
 *
 * Created: 01/04/2020 11.01.40
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 

#include "Logger.hpp"

extern "C"{
	#include "uart.h"
};

#include <stdarg.h>
#include <stdio.h>

static const char LOG_LEVEL_STRINGS[3][6] = {
	{"INFO"},
	{"DEBUG"},
	{"ERROR"}
};

Logger::Logger()
{
	
	bufferSize_ = 255;
	
	// Initialize UART0
	InitUART(UART0,115200,8,'N');
	
	// Just in case initialize other uarts
	InitUART(UART1,57600,8,'N');
	InitUART(UART2,115200,8,'N');
}

void Logger::Log(const char* msg)
{
	SendString(UART0,(char*)msg);
}

void Logger::Log(LogLevel level,const char* module,const char* msg, ...)
{
	// Safety checks
	if (level < 0 || level > 3)
		return;
	
	// Populate a va_list
	va_list args;
	va_start(args, msg);
	
	// Delegate formatted message to sprintf
	char msg_buffer[255];
	bufferLength_ = vsprintf(msg_buffer, msg, args);
	
	// Format log message
	bufferLength_ = snprintf(buffer_,bufferSize_,"%s: [%s] %s\n",LOG_LEVEL_STRINGS[level],module,msg_buffer);
	buffer_[bufferLength_] = '\0';
	
	// Output log message to UART0
	SendString(UART0,buffer_);
}

Logger::~Logger(){}