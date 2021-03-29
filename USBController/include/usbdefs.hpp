/*
 * USBDeviceConfig.h
 *
 * Created: 03/04/2020 13.28.34
 *  Author: hejmi
 */ 


#ifndef USBDEVICECONFIG_H_
#define USBDEVICECONFIG_H_
#include <stdint.h>

typedef void (*CallbackFunction)(void*,void*);

/* Most of the names corresponds to names in https://www.beyondlogic.org/usbnutshell/usb1.shtml */

typedef struct EpInfo {
	
	uint8_t epAddr;			// Endpoint address
	uint8_t maxPktSize;
	uint8_t direction;		// Endpoint transfer type IN = 0 | OUT = 1.
	uint8_t Interval;		// Polling interval in frames.
	
	uint8_t bmSndToggle;	// Send data toggle
	uint8_t bmRcvToggle;	// Recieve data toggle
	uint8_t bmNakPower;
	
} __attribute__((packed)) EpInfo;

// TODO: This will probably removed as i will be using DeviceRecord structure instead
typedef struct UsbDevice {
	EpInfo *epinfo; // endpoint info pointer
	uint8_t address;
	uint8_t epcount; // number of endpoints
	bool lowspeed; // indicates if a device is the low speed one
	//      uint8_t devclass; // device class
} UsbDevice;

/* Interface descriptor structure */
typedef struct USB_INTERFACE_DESCRIPTOR {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __attribute__((packed)) USB_INTERFACE_DESCRIPTOR;

/* Configuration descriptor structure */
typedef struct USB_CONFIGURATION_DESCRIPTOR{
	uint8_t bLength;				// Size of Descriptor in Bytes
	uint8_t bDescriptorType;		// Configuration Descriptor (0x02)
	uint16_t wTotalLength;			// Total length in bytes of data returned
	uint8_t bNumInterfaces;			// Number of Interfaces
	uint8_t bConfigurationValue;	// Value to use as an argument to select this configuration
	uint8_t iConfiguration;			// Index of String Descriptor describing this configuration
	uint8_t bmAttributes;			// D7 Reserved, set to 1. (USB 1.0 Bus Powered)D6 Self PoweredD5 Remote WakeupD4..0 Reserved, set to 0.
	uint8_t bMaxPower;				// Maximum Power Consumption in 2mA units
} __attribute__((packed)) USB_CONFIGURATION_DESCRIPTOR;

/* Device descriptor structure */
typedef struct USB_DEVICE_DESCRIPTOR{
	uint8_t bLength;			// Length of this descriptor.
	uint8_t bDescriptorType;	// DEVICE descriptor type (USB_DESCRIPTOR_DEVICE).
	uint16_t bcdUSB;			// USB Spec Release Number (BCD).
	uint8_t bDeviceClass;		// Class code (assigned by the USB-IF). 0xFF-Vendor specific.
	uint8_t bDeviceSubClass;	// Subclass code (assigned by the USB-IF).
	uint8_t bDeviceProtocol;	// Protocol code (assigned by the USB-IF). 0xFF-Vendor specific.
	uint8_t bMaxPacketSize0;	// Maximum packet size for endpoint 0.
	uint16_t idVendor;			// Vendor ID (assigned by the USB-IF).
	uint16_t idProduct;			// Product ID (assigned by the manufacturer).
	uint16_t bcdDevice;			// Device release number (BCD).
	uint8_t iManufacturer;		// Index of String Descriptor describing the manufacturer.
	uint8_t iProduct;			// Index of String Descriptor describing the product.
	uint8_t iSerialNumber;		// Index of String Descriptor with the device's serial number.
	uint8_t bNumConfigurations; // Number of possible configurations.
} __attribute__((packed)) USB_DEVICE_DESCRIPTOR;

typedef struct DeviceRecord {
	EpInfo* epInfo;
	uint8_t devAddress;
	USB_DEVICE_DESCRIPTOR* devDescriptor;
} DeviceRecord;

#endif /* USBDEVICECONFIG_H_ */