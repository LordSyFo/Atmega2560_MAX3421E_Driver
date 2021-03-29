/*
 * SPISerial.cpp
 *
 * Created: 30/03/2020 10.32.29
 *  Author: LordSyFo
 */ 
#define F_CPU 1600000

#include "SPISerial.hpp"
#include <avr/io.h>
#include <util/delay.h>

#define MOSI	PINB2
#define MISO	PINB3
#define SCK		PINB1
#define SS		PINL7	// used to be PB4 changed to PL7
#define SSOLD	PINB4

SPISerial::SPISerial()
{
		
		initSPI();			// Initialize SPI on AtMega2560
		DeselectSlave();	// Deselect slave ie. set ss high
		
}

uint8_t SPISerial::ReadByte()
{	
	// Send dummy byte and return data register
	return WriteByte(0x00);
}

/* Very unsafe and vulnerable to overflow */
uint16_t SPISerial::ReadBytes(uint8_t* bytes, uint16_t len)
{
	for (int i = 0; i < len; i++)
		bytes[i] = ReadByte();
	
	return len;
}

uint8_t SPISerial::WriteByte(uint8_t byte)
{
	
	SPDR = byte;				// Load data register with byte
	while(!(SPSR & (1<<SPIF)));	// Wait till transmission in completed
	
	return SPDR;
}

// If length is set to 0 we expect the bytes to be null-terminated
uint16_t SPISerial::WriteBytes(uint8_t* bytes, uint16_t len)
{
	if (len == 0){
		for (uint8_t* ch = bytes; *ch!='\0'; ch++)
			WriteByte(*ch);
	}
	else if (len > 0){
		for (int i = 0; i < len; i++)
			WriteByte(bytes[i]);
	}
	return len;
}

void SPISerial::initSPI()
{

	// Initialize pins
	DDRB = 0;
	DDRB &= ~(1<<MISO);											// Set MISO to input
	DDRB |= ((1<<MOSI) | (1<<SCK) | (1<<PINB0));				// Set MOSI,SCK and SS to outputs
	DDRL |= (1<<SS);
	
	// Setup SPI Registers
	SPCR |= (1<<SPE) | (1<<MSTR);			// Enable spi and set Master mode
	//SPCR |= (1<<SPR1) | (1<<SPR0);		// fosc / 128	= 125kHz
	//SPCR |= (1<<SPR0);					// fosc / 16	= 1MHz
	SPSR |= (1<<SPI2X);
	SPCR &= ~((1<<SPR0)|(1<<SPR1));			// fosc / 4	= 4MHz or fosc /2 = 8MHz with SPI2X
	
	
	// Clear SPI Interrupt
	uint8_t tmp = SPDR;
	tmp = SPSR;
	
}

void SPISerial::SelectSlave()
{
	PORTL &= ~(1<<SS);
}

void SPISerial::DeselectSlave()
{
	PORTL |= (1<<SS);
}

SPISerial::~SPISerial()
{
	
	
}