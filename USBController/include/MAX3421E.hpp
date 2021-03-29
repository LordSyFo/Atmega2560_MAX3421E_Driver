/* 
* MAX3421E.h
*
* Created: 30/03/2020 13.42.38
* Author: hejmi
*/


#ifndef __MAX3421E_H__
#define __MAX3421E_H__

#include "SPISerial.hpp"
#include "usbdefs.hpp"
#include "max3421defs.h"

class MAX3421E
{

public:
	MAX3421E();
	~MAX3421E();
	
	/**
	*	Initializes the MAX3421E-chip and device records.
	*	@return	True if chip was initialized succesfully, false otherwise
	*/
	bool Initialize();
	
	/**
	*	Resets member devicerecords and member endpoint records.
	*/
	void InitializeRecords();
	
	/**
	*	Probes the USB bus to determine what is connected, and saves the state in busState_.
	*	@return	State of the bus
	*/
	uint8_t ProbeBus();
	
	/**
	*	Writes multiple bytes to a given register.
	*	@param bytes	Bytes to be transmitted.
	*	@param reg		Register to transmit bytes to.
	*	@param length	Length of bytes.
	*/
	void WriteMultipleToReg(uint8_t* bytes,uint8_t reg, uint16_t length);
	
	/**
	*	Writes a single byte to a given register.
	*	@param byte		Byte to be transmitted.
	*	@param reg		Register to transmit byte to.
	*/
	void WriteSingleToReg(uint8_t byte,uint8_t reg);
	
	/**
	*	Reads a single byte from given register.
	*	@param	reg		Register to read from.
	*	@return The read byte
	*/
	uint8_t ReadSingleFromReg(uint8_t reg);
	
	/**
	*	Reads multiple bytes from given register.
	*	@param datacontainer	Container to read bytes into.
	*	@param reg				Register to read from.
	*	@param len				Length of data to be read (size of datacontainer).
	*	@return Address to datacontainer.
	*/
	uint8_t* ReadMultipleFromReg(uint8_t* datacontainer, uint8_t reg, uint8_t len);
	
	/**
	*	Configures found device from enumeration in a device record and updates the device address.
	*	@return True if configuration was successful, false otherwise.
	*/
	bool ConfigureDevices();
	
	/**
	*	Performs chip reset and waits for internal oscillator to be stable again.
	*	@return True if reset was successful, false if timeout occurred.
	*/
	bool Reset();
	
	/**
	*	Changes state in enumeration state-machine.
	*	@param state	The new state.
	*/
	void SetUSBState(uint8_t state);
	
	/**
	*	Gets the current state in the enumeration state machine.
	*	@return The current state.
	*/
	uint8_t GetUSBState() {return usbState_;};
	
	/**
	*	Dispatches token to given endpoint and handles NAK, timeouts and stalls.
	*	@param token		Token to be dispatched (could be IN or OUT).
	*	@param ep			Endpoint address to dispatch token to
	*	@param naklimit		Amount of NAK's before giving up
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	uint8_t DispatchPacket(uint8_t token, uint8_t ep, uint8_t naklimit);
	
	/**
	*	Performs a control transfer (for numerous descriptors).
	*	Handles Setup-stage, Data-stage and Status-stage specified in the USB Standard.
	*	@param address			Address of device to request descriptor from.
	*	@param ep				Endpoint to request from. (should always be 0 specifying the default pipe)
	*	@param bmRequestType	Request type (determines direction, type of request and designated recipient)
	*	@param bRequest			The kind of request, for example what kind of descriptor.
	*	@param wValueLow		Lower byte of request parameter
	*	@param wValueHigh		Upper byte of request parameter
	*	@param wIdx				Parameter index or offset
	*	@param wLength			Used to specify how many bytes to be transferred if there is a data stage
	*	@param data				Address to datacontainer for data in datastage (set to NULL if theres no data stage)
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	uint8_t ControlRequest(uint8_t address, uint8_t ep, uint8_t bmRequestType, uint8_t bRequest,
	uint8_t wValueLow,uint8_t wValueHigh, uint16_t wIdx, uint16_t wLength, uint8_t* data);
	
	/**
	*	Performs a BULK-IN Transfer described in https://pdfserv.maximintegrated.com/en/an/AN3785.pdf
	*	@param pep				Pointer to endpoint to do InTransfer from.
	*	@param nbytesptr		Pointer to number of bytes to be read
	*	@param data				Pointer to datacontainer for read data
	*	@param bInterval		Interval for polling data transfers from specified endpoint.
	*	@param naklimit			Amount of NAK's before giving up
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	uint8_t InTransfer(EpInfo* pep,uint16_t* nbytesptr, uint8_t* data, uint8_t bInterval,uint8_t naklimit);
	
	/**
	*	Performs a BULK-OUT Transfer described in https://pdfserv.maximintegrated.com/en/an/AN3785.pdf
	*	@param pep				Pointer to endpoint to do OutTransfer to.
	*	@param nbytes			Number of bytes to be transferred
	*	@param data				Pointer to datacontainer for data to be transmitted
	*	@param naklimit			Amount of NAK's before giving up
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	uint8_t OutTransfer(EpInfo* pep, uint8_t nbytes, uint8_t* data,uint8_t naklimit);
	
	/**
	*	Loads a specified address into the PERADDR register used to determine where packets should be sent to.
	*	Also updates the MODE register to accommodate for speed of device.
	*	@param address		Address to load
	*/
	void SetAddress(uint8_t address);

	/**
	*	Gets the currently active device record, found during enumeration.
	*	@return	Pointer to the currently active device record.
	*/
	const DeviceRecord* GetActiveDevRecord() const;
	
	/**
	*	Enumerates USB to find connected devices. Uses a state-machine to perform enumeration.
	*/
	void Enumerate();

	/**
	*	Prints the VID and PID of the found device-descriptor.
	*/
	void PrintDeviceInfo();

	// Inline methods
	/**
	*	Gets the device descriptor for a specific device determined by address.
	*	@param addr		Device address to get device record from.
	*	@param ep		Should always be zero as device descriptors lay in the default pipeline.
	*	@param nbytes	Amount of bytes to read from the acquired device descriptor.
	*	@param dataPtr	Pointer to the data-container, where the device descriptor is to be loaded into.
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	inline uint8_t GetDeviceDescriptor(uint8_t addr, uint8_t ep, unsigned int nbytes, uint8_t* dataPtr){
		return (ControlRequest(addr,ep,bmREQ_GET_DESCR,USB_REQUEST_GET_DESCRIPTOR,0x00,USB_DESCRIPTOR_DEVICE,0x0000,nbytes,dataPtr));
	}

	/**
	*	Sets a unique address to the connected device.
	*	@param oldaddr		Device address to be updated.
	*	@param ep			Should always be zero to specify the default pipeline.
	*	@param newaddr		Unique address to be set.
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	inline uint8_t SetDeviceAddress(uint8_t oldaddr, uint8_t ep, uint8_t newaddr){
		return (ControlRequest(oldaddr,ep,bmREQ_SET,USB_REQUEST_SET_ADDRESS, newaddr, 0x00, 0x0000, 0x0000, 0));
	}

	/**
	*	Gets the configuration from a specified device.
	*	@param addr			Device address to get configuration from.
	*	@param ep			Should always be zero to specify the default pipeline.
	*	@param nbytes		Number of bytes to be read (should be 1 all the time).
	*	@param dataPtr		Pointer to store received data (0 value means device hasnt been configured and 1 means it has).
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	inline uint8_t GetConfiguration(uint8_t addr, uint8_t ep, unsigned int nbytes, uint8_t* dataPtr){
		return (ControlRequest(addr,ep,bmREQ_GET_DESCR,USB_REQUEST_GET_CONFIGURATION, 0x00, 0x00, 0x0000, nbytes, dataPtr));
	}

	/**
	*	Gets the configuration descriptor from a specified device (only gets the first since most usb devices only has one).
	*	@param addr			Device address to get configuration descriptor from.
	*	@param ep			Should always be zero to specify the default pipeline.
	*	@param nbytes		Number of bytes to be read.
	*	@param dataPtr		Pointer to store received configuration descriptor.
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	inline uint8_t GetConfigDescriptor(uint8_t addr, uint8_t ep, unsigned int nbytes, uint8_t* dataPtr){
		return (ControlRequest(addr,ep,bmREQ_GET_DESCR,USB_REQUEST_GET_DESCRIPTOR, 0x00,USB_DESCRIPTOR_CONFIGURATION, 0x0000, nbytes, dataPtr));
	}

	/**
	*	Gets an interface descriptor from a specified device with a specified interface number.
	*	@param addr			Device address to get interface descriptor from.
	*	@param ep			Should always be zero to specify the default pipeline.
	*	@param interface	Interface number to get descriptor from.
	*	@param nbytes		Number of bytes to be read.
	*	@param dataPtr		Pointer to store received interface descriptor.
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	inline uint8_t GetInterfaceDescriptor(uint8_t addr, uint8_t ep, uint8_t interface, unsigned int nbytes, uint8_t* dataPtr){
		return (ControlRequest(addr,ep,bmREQ_GET_DESCR,USB_REQUEST_GET_DESCRIPTOR, 0, USB_DESCRIPTOR_INTERFACE, 0x0000, nbytes, dataPtr));
	}

	/**
	*	Sets the configuration on specified device. Used to enable the device.
	*	@param addr			Device address to set configuration.
	*	@param ep			Should always be zero to specify the default pipeline.
	*	@param config		Configuration to enable (stored in bConfigurationValue from the Configuration descriptor).
	*	@return A host return code specified at * Host result codes * in max3421defs.h 
	*/
	inline uint8_t SetConfiguration(uint8_t addr, uint8_t ep, uint8_t config){
		return (ControlRequest(addr,ep,bmREQ_SET,USB_REQUEST_SET_CONFIGURATION, config, 0x00, 0x0000, 0, 0));
	}

private:
	SPISerial* spi_;

	uint8_t busState_;
	uint8_t usbState_;
	
	EpInfo ep_;
	UsbDevice* usb_;
	DeviceRecord devRecord_[2];
	
	// Constants
	static const uint16_t nakLimit_		= 100;	//TODO find more fitting values for these
	static const uint16_t retryLimit_	= 100;
	
	uint8_t devDescBuf_[sizeof(USB_DEVICE_DESCRIPTOR)];	// Buffer for device-descriptor
	
}; //MAX3421E

typedef struct SetupPackage{
	
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint8_t wValueLow;
	uint8_t wValueHigh;
	uint16_t wIndex;
	uint16_t wLength;
	
	SetupPackage(uint8_t reqtype = 0, uint8_t req = 0, uint8_t vallow = 0,uint8_t valhigh = 0, uint16_t idx = 0, uint16_t len = 0)
	{
		bmRequestType = reqtype;
		bRequest = req;
		wValueLow = vallow;
		wValueHigh = valhigh;
		wIndex = idx;
		wLength = len;
	}
	
} __attribute__((packed)) SetupPackage;

#endif //__MAX3421E_H__
