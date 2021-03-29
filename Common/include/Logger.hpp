/*
 * Logger.h
 *
 * Created: 01/04/2020 10.58.46
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>

typedef enum LogLevel {
	INFO_LEVEL,
	DEBUG_LEVEL,
	ERROR_LEVEL
} LogLevel;

class Logger
{
	
	public:

		Logger();
		~Logger();
		
		void Log(LogLevel level,const char* module,const char* msg, ...);
		void Log(const char* msg);
	
	private:
		char buffer_[255];
		uint16_t bufferSize_;
		uint16_t bufferLength_;
};

/* Macros */
static Logger uart_logger_g;

#define LOG_INFO(...) (uart_logger_g.Log(INFO_LEVEL,__FILE__,__VA_ARGS__))

#ifdef DEBUG_MODE
	#define LOG_DEBUG(...) (uart_logger_g.Log(DEBUG_LEVEL,__FILE__,__VA_ARGS__))
#else
	#define LOG_DEBUG(...) void(0)
#endif

#define LOG_ERROR(...) (uart_logger_g.Log(ERROR_LEVEL,__FILE__,__VA_ARGS__))
#define LOG_NO_FORMAT(msg) (uart_logger_g.Log(msg))

#endif /* LOGGER_H_ */