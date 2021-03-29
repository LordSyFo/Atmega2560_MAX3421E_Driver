/************************************************
* "uart.c":                                     *
* Implementation file for Mega2560 UART driver. *
* Using UART0, UART1, UART2 or UART3            *
*                                               *
* Henning Hargaard, 2/4 2019                    *
*************************************************/
#include <avr/io.h>
#include <stdlib.h>
#include "uart.h"

// Target CPU frequency
#define XTAL 16000000

// Memory mapping the UART registers
// "UART_adr" is parameter for the functions
#define UCSRnA _SFR_MEM8(UART_adr)
#define UCSRnB _SFR_MEM8(UART_adr+1)
#define UCSRnC _SFR_MEM8(UART_adr+2)
#define UBRRn  _SFR_MEM16(UART_adr+4)
#define UDRn   _SFR_MEM8(UART_adr+6)

/*************************************************************************
USART initialization.
    Asynchronous mode.
	RX and TX enabled.
	No interrupts enabled.
	Number of Stop Bits = 1.
	No Parity.
	Baud rate = Parameter.
	Data bits = Parameter.
Parameters:
	BaudRate: Wanted Baud Rate (300-115200).
	Databits: Wanted number of Data Bits (5-8).
	Parity: 'E' (Even parity), 'O' (Odd parity), otherwise No Parity.
*************************************************************************/
void InitUART(unsigned int UART_adr, unsigned long BaudRate, unsigned char DataBit, char Parity)
{
  if ((BaudRate >= 300) && (BaudRate <= 115200) && (DataBit >=5) && (DataBit <= 8))
  { 
    // "Normal" clock, no multiprocessor mode (= default)
    UCSRnA = 0b00100000;
    // No interrupts enabled
    // Receiver enabled
    // Transmitter enabled
    // No 9 bit operation
    UCSRnB = 0b00011000;	
    // Asynchronous operation, 1 stop bit
    // Bit 2 and bit 1 controls the number of data bits
    UCSRnC = (DataBit-5)<<1;
	// Set parity bits (if parity used)
	if (Parity == 'E')
      UCSRnC |= 0b00100000;
    else if (Parity == 'O')	  
      UCSRnC |= 0b00110000;	
    // Set Baud Rate according to the parameter BaudRate:
    // Adding (8*Baudrate) ensures correct rounding (up/down)
    UBRRn = (XTAL+(8*BaudRate))/(16*BaudRate) - 1;
  }  
}

/*************************************************************************
  Returns 0 (FALSE), if the UART has NOT received a new character.
  Returns value <> 0 (TRUE), if the UART HAS received a new character.
*************************************************************************/
unsigned char CharReady(unsigned int UART_adr)
{
   return UCSRnA & (1<<7);
}

/*************************************************************************
Awaits new character received.
Then this character is returned.
*************************************************************************/
char ReadChar(unsigned int UART_adr)
{
  // Wait for new character received
  while ( (UCSRnA & (1<<7)) == 0 )
  {}                        
  // Then return it
  return UDRn;
}

/*************************************************************************
Awaits transmitter register ready.
Then send the character.
Parameter :
	Tegn : Character for sending. 
*************************************************************************/
void SendChar(unsigned int UART_adr, char Tegn)
{
  // Wait for transmitter register empty (ready for new character)
  while ( (UCSRnA & (1<<5)) == 0 )
  {}
  // Then send the character
  UDRn = Tegn;
}

/*************************************************************************
Sends 0 terminated string.
Parameter:
   Streng: Pointer to the string. 
*************************************************************************/
void SendString(unsigned int UART_adr, char* Streng)
{
  // Repeat until zero-termination
  while (*Streng != 0)
  {
    // Send the character pointed to by "Streng"
    SendChar(UART_adr, *Streng);
    // Advance the pointer one step
    Streng++;
  }
}

/*************************************************************************
Converts the integer "Tal" to an ASCII string - and then sends this string
using the USART.
Makes use of the C standard library <stdlib.h>.
Parameter:
    Tal: The integer to be converted and sent. 
*************************************************************************/
void SendInteger(unsigned int UART_adr, int Tal)
{
char array[7];
  // Convert the integer to an ASCII string (array), radix = 10 
  itoa(Tal, array, 10);
  // - then send the string
  SendString(UART_adr, array);
}

void SendIntegerBinary(unsigned int UART_adr, uint8_t Tal)
{
	char array[8];
	// Convert the integer to an ASCII string (array), radix = 10
	itoa(Tal, array, 2);
	// - then send the string
	SendString(UART_adr, array);
}

/************************************************************************/