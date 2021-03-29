/*
 * xboxdefs.h
 *
 * Created: 16/04/2020 11.50.50
 *  Author: hejmi
 */ 


#ifndef XBOXDEFS_H_
#define XBOXDEFS_H_

typedef struct XBOXInputRecord {
	uint8_t primaryKeys;
	uint8_t secondaryKeys;
} __attribute__((packed)) XBOXInputRecord;

/* Control key offsets */
#define SECONDARY_CONTROLKEYS_OFFSET	2
#define PRIMARY_CONTROLKEYS_OFFSET		3
#define LT_OFFSET						4
#define RT_OFFSET						5

#define LEFTX_LOW						6
#define LEFTX_HIGH						7

#define RIGHTX_LOW						8
#define RIGHTX_HIGH						9

/* Control key packet offsets for primary controlkeys*/
#define AKEY			16
#define BKEY			32
#define XKEY			64
#define YKEY			128
#define XBOX_CENTER_KEY 4
#define RBKEY			2
#define LBKEY			1

/* Control key packet offsets for secondary controlkeys*/
#define UPKEY				1
#define DOWNKEY				2
#define LEFTKEY				4
#define RIGHTKEY			8
#define ANALOG_DOWN_LEFT	64
#define ANALOG_DOWN_RIGHT	128
#define BACKKEY				32
#define STARTKEY			16

/* Control packet types */
#define RUMBLE_TYPE 0x00
#define LED_TYPE	0x01

/* Output request types */
#define REQUEST_INVALID 0x00
#define REQUEST_RUMBLE	0x01	// Takes two uint8_t parameters indicating left and right motor speeds respectively
#define REQUEST_LED		0x02	// Takes one parameter indicating the LED animation		

/* LED Animations - https://www.partsnotincluded.com/xbox-360-controller-led-animations-info/ */
#define LED_ONE_ON		0x06
#define LED_TWO_ON		0x07
#define LED_THREE_ON	0x08
#define LED_FOUR_ON		0x09
#define LED_ROTATING	0x0A

#endif /* XBOXDEFS_H_ */