/*
 * ISerial.h
 *
 * Created: 30/03/2020 10.28.16
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 


#ifndef ISERIAL_H_
#define ISERIAL_H_

#include <stdint.h>

class ISerial
{
	
	public:
		virtual uint8_t ReadByte() = 0;
		virtual uint8_t WriteByte(uint8_t byte) = 0;
		virtual uint16_t ReadBytes(uint8_t* bytes, uint16_t len) = 0;
		virtual uint16_t WriteBytes(uint8_t* bytes, uint16_t len) = 0;
};



#endif /* ISERIAL_H_ */