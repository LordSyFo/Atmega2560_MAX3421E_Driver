/*
 * SPISerial.cpp
 *
 * Created: 30/03/2020 10.30.26
 *  Author: LordSyFo
 */ 
#include "ISerial.hpp"

#ifndef SPISerial_H_
#define SPISerial_H_

class SPISerial : public ISerial 
{
	
	public:
		SPISerial();
		virtual ~SPISerial();
		
		// Public methods
		
		/**
		*	Reads a byte from the SPI data register by writing a 
			dummy byte and returning the leftover data.
		*	@return The read byte.
		*/
		uint8_t ReadByte();
		
		/**
		*	Transmits a specified byte to the SPI.
		*	@param byte		Specified byte to write over SPI.
		*	@return Byte leftover in the data register after transmission.
		*/
		uint8_t WriteByte(uint8_t byte);
		
		/**
		*	Reads a specified length of bytes into a byte array passed as argument.
		*	@param bytes	Specified byte buffer to fill read data into.
		*	@param len		Number of bytes to read.
		*	@return Length of read data.
		*/
		uint16_t ReadBytes(uint8_t* bytes, uint16_t len);
		
		/**
		*	Transmits specified bytes to SPI.
		*	@param bytes	Specified bytes to transmit.
		*	@param len		Number of bytes to transmit.
		*	@return Length of transmitted bytes.
		*/
		uint16_t WriteBytes(uint8_t* bytes, uint16_t len);
		
		/**
		*	Selects slave on pin SS (by setting SS low)
		*/
		void SelectSlave();
		
		/**
		*	Deselects slave on pin SS (by setting SS high)
		*/
		void DeselectSlave();

	private:
		// Private methods
		/**
		* Initializes the SPI with defined MOSI,MISO,SS and SCK
		*/
		void initSPI();
		
		// Private attributes
		uint8_t _rxBuffer[16];
		uint8_t _txBuffer[16];
		
};

#endif /* SPISerial_H_ */