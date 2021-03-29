/****************************************
* "uart.h":                             *
* Header file for Mega2560 UART driver. *
* Using UART0, UART1, UART2 or UART3    *
*                                       *
* Henning Hargaard, 2/4 2019            *
*****************************************/ 

#ifndef __UART_H__
#define __UART_H__

// Memory mapping the base address of the UART registers
#define UART0 0xC0
#define UART1 0xC8
#define UART2 0xD0
#define UART3 0x130

void InitUART(unsigned int UART_adr, unsigned long BaudRate, unsigned char DataBit, char Parity);
unsigned char CharReady(unsigned int UART_adr);
char ReadChar(unsigned int UART_adr);
void SendChar(unsigned int UART_adr, char Tegn);
void SendString(unsigned int UART_adr, char* Streng);
void SendInteger(unsigned int UART_adr, int Tal);
void SendIntegerBinary(unsigned int UART_adr, uint8_t Tal);
/****************************************/

#endif //__UART_H__