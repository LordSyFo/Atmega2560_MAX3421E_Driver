/*
 * MAX3421E_Defs.h
 *
 * Created: 31/03/2020 11.45.51
 *  Author: hejmi
 */ 


#ifndef MAX3421E_DEFS_H_
#define MAX3421E_DEFS_H_

// Host result codes
#define hrSUCCES	0x00
#define hrBUSY		0x01
#define hrBADREQ	0x02
#define hrUNDEF		0x03
#define hrNAK		0x04
#define hrSTALL		0x05
#define hrTOGERR	0x06
#define hrWRONGPID	0x07
#define hrBADBC		0x08
#define hrPIDERR	0x09
#define hrPKTERR	0x0A
#define hrCRCERR	0x0B
#define hrKERR		0x0C
#define hrJERR		0x0D
#define hrTIMEOUT	0x0E
#define hrBABBLE	0x0F

#define hrRECIEVE_ERROR	0xf0
#define hrDATAERROR		0xf1
#define hrBUFFERFULL	0xf2

// HXFR Register bit settings for transfer types
#define hxfrSETUP	0x10
#define hxfrBULKOUT	0x02	// plus endpoint

// Host Registers
#define HXFR 30		// Used to transmit bytes in the fifos, should also be loaded with transmission type
#define HRSL 31
#define HIRQ 25
#define MODE 27
#define HIEN 26
#define HCTL 29
#define CPUCTL 16
#define PERADDR 28
#define RCVBC	6
#define USBCTL 15
#define USBIRQ 13
#define USBIEN 14

#define RCVFIFO 1	// Used to store recieved data
#define SNDFIFO 2	// Used to store tranmission data
#define SUDFIFO 4	// Used to store setup package data
#define SNDBC	7	// Used to store amount of bytes to be transferred

#define IOPINS1 20
#define IOPINS2 21

#define PINCTL 17

// Register bits
#define SNDTOG1		7
#define SNDTOG0		6
#define RCVTOG1		5
#define RCVTOG0		4
#define SIGRSM		3
#define SAMPLEBUS	2
#define FRMRST		1
#define BUSRST		0

#define HXFRDNIRQ	7
#define FRAMEIRQ	6
#define CONDETIRQ	5
#define SUSDNIRQ	4
#define SNDBAVIRQ	3
#define RCVDAVIRQ	2
#define RWUIRQ		1
#define BUSEVENTIRQ	0

#define HXFRDNIE	7
#define FRAMEIE		6
#define CONDETIE	5
#define SUSDNIE		4
#define SNDBAVIE	3
#define RCVDAVIE	2
#define RWUIE		1
#define BUSEVENTIE	0

#define FDUPSPI		4
#define INTLEVEL	3
#define POSINT		2
#define GPXB		1
#define GPXA		0

#define DPPULLDN	7
#define DMPULLDN	6
#define DELAYISO	5
#define SEPIRQ		4
#define SOFKAENAB	3
#define HUBPRE		2
#define LOWSPEED	1
#define HOST		0

#define CHIPRES		5
#define PWRDOWN		4

#define VBUSIRQ		6
#define NOVBUSIRQ	5
#define OSCOKIRQ	0

#define JSTATUS		7
#define KSTATUS		6
#define SNDTOGRD	5
#define RCVTOGRD	4
#define HRSLT3		3
#define HRSLT2		2
#define HRSLT1		1
#define HRSLT0		0

/* USB state machine states */
#define USB_DISCONNECTED									0x00
#define USB_ILLEGAL_STATE									0x01
#define USB_ERROR											0x02
#define USB_DEVICE_FOUND									0x03
#define USB_INITIALIZE										0x04
#define USB_SETTLE											0x05
#define USB_PERIPHERAL_RESET								0x06
#define USB_WAIT_RESET										0x07
#define USB_WAIT_SOF										0x08
#define USB_GET_DEV_DESCRIPTOR								0x09
#define USB_ADDRESSING										0x0a
#define USB_CONFIGURING										0x0c
#define USB_RUNNING											0x0b

#define USB_XFER_TIMEOUT    5000    //USB transfer timeout in milliseconds, per section 9.2.6.1 of USB 2.0 spec
#define USB_NAK_LIMIT       32000   //NAK limit for a transfer. o meand NAKs are not counted
#define USB_RETRY_LIMIT     3       //retry limit for a transfer
#define USB_SETTLE_DELAY    200     //settle delay in milliseconds
#define USB_NAK_NOWAIT      1       //used in Richard's PS2/Wiimote code

// Request types
#define GET_STATUS SetupPackage(0b10000000,0x00,0x00,0x00,0x02)
#define GET_DESCRIPTOR(desctype,descidx,lang,desclen) (SetupPackage(0b10000000,0x06,((uint16_t)desctype<<8|descidx),lang,desclen))	// Usually lang can be set to zero

#define HS_IN	0x80	// Handshake in		- terminates a control WRITE
#define HS_OUT	0xA0	// Handshake out	- terminates a control READ

// Special VBUS return types
#define SE0     0
#define SE1     1
#define FSHOST  2
#define LSHOST  3

#define SE0_STATE 0
#define NA_STATE 0xc0

// Host modes
#define MODE_FS_HOST	((1<<DPPULLDN) | (1<<DMPULLDN) | (1<<HOST) | (1<<SOFKAENAB))
#define MODE_LS_HOST	((1<<DPPULLDN) | (1<<DMPULLDN) | (1<<HOST) | (1<<SOFKAENAB) | (1<<LOWSPEED))

// Tokens
#define SETUP_TOKEN			0x10
#define IN_TOKEN			0x00
#define OUT_TOKEN			0x20
#define OUT_HANDSHAKE_TOKEN	0xA0
#define IN_HANDSHAKE_TOKEN	0x80

#define USB_NUMDEVICES 16

/* Setup Data Constants */

#define USB_SETUP_HOST_TO_DEVICE                0x00    // Device Request bmRequestType transfer direction - host to device transfer
#define USB_SETUP_DEVICE_TO_HOST                0x80    // Device Request bmRequestType transfer direction - device to host transfer
#define USB_SETUP_TYPE_STANDARD                 0x00    // Device Request bmRequestType type - standard
#define USB_SETUP_TYPE_CLASS                    0x20    // Device Request bmRequestType type - class
#define USB_SETUP_TYPE_VENDOR                   0x40    // Device Request bmRequestType type - vendor
#define USB_SETUP_RECIPIENT_DEVICE              0x00    // Device Request bmRequestType recipient - device
#define USB_SETUP_RECIPIENT_INTERFACE           0x01    // Device Request bmRequestType recipient - interface
#define USB_SETUP_RECIPIENT_ENDPOINT            0x02    // Device Request bmRequestType recipient - endpoint
#define USB_SETUP_RECIPIENT_OTHER               0x03    // Device Request bmRequestType recipient - other

/* Standard Device Requests */
#define USB_REQUEST_GET_STATUS                  0       // Standard Device Request - GET STATUS
#define USB_REQUEST_CLEAR_FEATURE               1       // Standard Device Request - CLEAR FEATURE
#define USB_REQUEST_SET_FEATURE                 3       // Standard Device Request - SET FEATURE
#define USB_REQUEST_SET_ADDRESS                 5       // Standard Device Request - SET ADDRESS
#define USB_REQUEST_GET_DESCRIPTOR              6       // Standard Device Request - GET DESCRIPTOR
#define USB_REQUEST_SET_DESCRIPTOR              7       // Standard Device Request - SET DESCRIPTOR
#define USB_REQUEST_GET_CONFIGURATION           8       // Standard Device Request - GET CONFIGURATION
#define USB_REQUEST_SET_CONFIGURATION           9       // Standard Device Request - SET CONFIGURATION
#define USB_REQUEST_GET_INTERFACE               10      // Standard Device Request - GET INTERFACE
#define USB_REQUEST_SET_INTERFACE               11      // Standard Device Request - SET INTERFACE
#define USB_REQUEST_SYNCH_FRAME                 12      // Standard Device Request - SYNCH FRAME

/* USB descriptors  */
#define USB_DESCRIPTOR_DEVICE                   0x01    // bDescriptorType for a Device Descriptor.
#define USB_DESCRIPTOR_CONFIGURATION            0x02    // bDescriptorType for a Configuration Descriptor.
#define USB_DESCRIPTOR_STRING                   0x03    // bDescriptorType for a String Descriptor.
#define USB_DESCRIPTOR_INTERFACE                0x04    // bDescriptorType for an Interface Descriptor.
#define USB_DESCRIPTOR_ENDPOINT                 0x05    // bDescriptorType for an Endpoint Descriptor.
#define USB_DESCRIPTOR_DEVICE_QUALIFIER         0x06    // bDescriptorType for a Device Qualifier.
#define USB_DESCRIPTOR_OTHER_SPEED              0x07    // bDescriptorType for a Other Speed Configuration.
#define USB_DESCRIPTOR_INTERFACE_POWER          0x08    // bDescriptorType for Interface Power.
#define USB_DESCRIPTOR_OTG                      0x09    // bDescriptorType for an OTG Descriptor.

// Control requests
#define bmREQ_GET_DESCR     USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_DEVICE     //get descriptor request type
#define bmREQ_SET           USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_DEVICE     //set request type for all but 'set feature' and 'set interface'
#define bmREQ_CL_GET_INTF   USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE     //get interface request type
#endif /* MAX3421E_DEFS_H_ */