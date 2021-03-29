/* 
* MAX3421E.cpp
*
* Created: 30/03/2020 13.42.38
* Author: Nicklas Grunert (@github.com/LordSyFo)
*/

#define F_CPU 16000000

#include <util/delay.h>
#include "MAX3421E.hpp"
#include <string.h>
#include "max3421defs.h"
#include <avr/io.h>
#include "Logger.hpp"
#include "FreeRTOS.h"
#include "task.h"

#define MAX_RESET	PINL6		// used to be PINH4 changed after integration
#define GPX			PINH5
#define INT			PINH6

// default constructor
MAX3421E::MAX3421E()
{
	// Initialize SPI
	spi_ = new SPISerial();
	spi_->DeselectSlave();
	
	// Initialize endpoint
	ep_.maxPktSize = 8;
	
	// Initialize pins
	//DDRH	&= ~((1<<GPX) | (1<<INT));
	//DDRH	|= (1<<MAX_RESET);
	//PORTH	|= (1<<MAX_RESET);	// release from reset
	DDRL	|= (1<<MAX_RESET);
	PORTL	|= (1<<MAX_RESET);	// release from reset
	
	
	
	usb_ = new UsbDevice();
	usb_->lowspeed = false;		// is set to true if bus sample determines the device is lowspeed
	
	usbState_ = USB_DISCONNECTED;	// set up state machine
	busState_ = SE0;				// set up bus state to disconnected
}

bool MAX3421E::Initialize()
{
	/* Inspired by https://github.com/felis/USB_Host_Shield_2.0 */
	
	/* Setup full-duplex SPI */
	WriteSingleToReg(((1<<FDUPSPI)),PINCTL);
	
	if (Reset() == 0)	// timeout occured
	{
		LOG_ERROR("Couldn't reset MAX3421E.");
		return false;
	}
	
	_delay_ms(1000);
	
	WriteSingleToReg(((1<<DPPULLDN) | (1<<DMPULLDN) | (1<<HOST)),MODE);	// Pull d+ and d- low and set host mode
	
	LOG_DEBUG("Successfully initialized MAX3421E.");

	InitializeRecords();

	return true;	
}

void MAX3421E::SetUSBState(uint8_t state){
	//LOG_DEBUG("State: %d",state);
	usbState_ = state;
}

void MAX3421E::Enumerate()
{
	/* Inspired by https://github.com/felis/USB_Host_Shield_2.0 */
	
	uint8_t rcode;

	static uint32_t delay = 0;

	// Switch on busState should've been set after initialization
	switch(busState_)
	{
		case NA_STATE:	// N/A State
			SetUSBState(USB_ILLEGAL_STATE);
			break;
		case SE0:
			SetUSBState(USB_DISCONNECTED);
			break;
		case LSHOST:
			usb_->lowspeed = true;	// device is lowspeed
		case FSHOST:
			/* If device is connecting we don't want to override usbstate */
			if (usbState_ < USB_DEVICE_FOUND){
				delay = USB_SETTLE_DELAY;
				SetUSBState(USB_SETTLE);	
			}
			break;
	}
	
	switch (usbState_){
		case USB_ILLEGAL_STATE:
		case USB_DISCONNECTED:
			/* Probe bus to see if any device has connected */
			ProbeBus();
			break;
		case USB_SETTLE:
			/* Let usb device settle if it has just been connected */
			vTaskDelay(delay/portTICK_RATE_MS);
			SetUSBState(USB_DEVICE_FOUND);
			break;
		case USB_DEVICE_FOUND:
			SetUSBState(USB_INITIALIZE);
			break;
		case USB_INITIALIZE:
			InitializeRecords();
			SetUSBState(USB_PERIPHERAL_RESET);	// issue bus reset to set device into default unconfigured state
			break;
		case USB_PERIPHERAL_RESET:
			WriteSingleToReg((1<<BUSRST),HCTL);	// bus reset
			SetUSBState(USB_WAIT_RESET);
			break;
		case USB_WAIT_RESET:
			/* Wait for bus reset to be completed */
			if ((ReadSingleFromReg(HCTL) & (1<<BUSRST)) == 0)
			{
				uint8_t modeReg = ReadSingleFromReg(MODE);
				WriteSingleToReg(modeReg | (1<<SOFKAENAB),MODE);	// Enable auto gen of FS SOF packets or LS keep-alive pulses / frame markers
				SetUSBState(USB_WAIT_SOF);
			}
			break;
		case USB_WAIT_SOF:
			/* Wait for at least one frame marker */
			if (ReadSingleFromReg(HIRQ) & (1<<FRAMEIRQ)){
				delay = 20;
				vTaskDelay(delay/portTICK_RATE_MS);	// wait to get it going
				SetUSBState(USB_GET_DEV_DESCRIPTOR);
			}
			break;
		case USB_GET_DEV_DESCRIPTOR:
			devRecord_[0].epInfo->maxPktSize = 8;
			rcode = GetDeviceDescriptor(0,0,sizeof(USB_DEVICE_DESCRIPTOR),(uint8_t*)&devDescBuf_);
			if (rcode == hrSUCCES){
				
				// Save information in template device record
				devRecord_[0].devDescriptor = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(devDescBuf_);
				devRecord_[0].epInfo->maxPktSize = devRecord_[0].devDescriptor->bMaxPacketSize0;
				
				SetUSBState(USB_ADDRESSING);
				
			} else {
				//LOG_ERROR("Failed to get device descriptor %d", rcode);
				SetUSBState(USB_ERROR);
			}
			break;
		case USB_ADDRESSING:
			/* Configure connected device */
			if (ConfigureDevices())
			{
				SetUSBState(USB_CONFIGURING);
			} else {
				SetUSBState(USB_ERROR);
			}
			break;
		case USB_CONFIGURING:
			break;
		case USB_RUNNING:
			break;
		case USB_ERROR:
			LOG_ERROR("Error occurred while enumerating usb!");
			break;
	}
	
}

const DeviceRecord* MAX3421E::GetActiveDevRecord() const
{
	// If record has been configured.
	if (devRecord_[1].devDescriptor != NULL)
		return &devRecord_[1];
	else
		return NULL;
}

bool MAX3421E::ConfigureDevices()
{
	uint8_t rcode;
	
	for (int i = 1; i < sizeof(devRecord_)/sizeof(devRecord_[0]); i++)
	{
		/* If record isn't initialized with an endpoint info */
		if (devRecord_[i].epInfo == NULL){
			
			// Copy from template obtained under enumeration
			devRecord_[i].epInfo = devRecord_[0].epInfo;
			devRecord_[i].devDescriptor = devRecord_[0].devDescriptor;	
			
			// Set the device address
			rcode = SetDeviceAddress(0,0,i);
			
			if (rcode == hrSUCCES){
				// If device address was accepted
				devRecord_[i].devAddress = i;
				
			} else {
				LOG_ERROR("Couldn't set device address. %d",rcode);
				return false;
			}
		}
	}
	
	return true;
}

void MAX3421E::InitializeRecords()
{
	for (int i = 0; i < sizeof(devRecord_)/sizeof(devRecord_[0]); i++)
	{
		devRecord_[i].epInfo = NULL;
		devRecord_[i].devDescriptor = NULL;
	}
		
	devRecord_[0].epInfo = &ep_;
		
	// Initialize endpoint
	ep_.bmSndToggle = SNDTOG0;   //set DATA0/1 toggles to 0
	ep_.bmRcvToggle = RCVTOG0;
	ep_.epAddr		= 0x00;
	
}

void MAX3421E::PrintDeviceInfo()
{
	if (devRecord_[1].devDescriptor != NULL)
		LOG_INFO("VID: %d\nPID: %d",devRecord_[1].devDescriptor->idVendor,devRecord_[1].devDescriptor->idProduct);
}


uint8_t MAX3421E::ProbeBus()
{
	/* Inspired by https://github.com/felis/USB_Host_Shield_2.0 */
	/* and https://pdfserv.maximintegrated.com/en/an/AN3785.pdf */
	
	/* Sample bus to determine wether a device is connected */
	WriteSingleToReg((1<<SAMPLEBUS),HCTL);	// sample d+ and d-
		
	while(!(ReadSingleFromReg(HCTL) & (1<<SAMPLEBUS)));	// wait for sampling to be done (sie clears samplebus bit)
	
	// Read the sample from HRSL
	uint8_t busSample = ReadSingleFromReg(HRSL);
	
	// Only examine J and K bits
	busSample &= ((1<<JSTATUS) | (1<<KSTATUS));
	
	uint8_t lowspeed = ReadSingleFromReg(MODE) & (1<<LOWSPEED);	// meaning of j and k state depends on LOWSPEED bit
	
	/* Switch on KSTATUS and JSTATUS bits to determine mode */
	switch(busSample)
	{
		case ((1<<JSTATUS)):
			if (lowspeed)
			{
				/* D+ Low and D- High => Lowspeed device */
				WriteSingleToReg(MODE_LS_HOST,MODE);	// Start low-speed host mode
				busState_ = LSHOST;					// save state
				
			} else {
				/* D+ High and D- Low => Fullspeed device */
				WriteSingleToReg(MODE_FS_HOST,MODE);	// Start full-speed host mode
				busState_ = FSHOST;					// save state
			}
			break;
		case ((1<<KSTATUS)):
			if (lowspeed)
			{
				/* D+ High and D- Low => Fullspeed device */
				WriteSingleToReg(MODE_FS_HOST,MODE);	// Start full-speed host mode
				busState_ = FSHOST;					// save state
							
				} else {
				/* D+ Low and D- High => Lowspeed device */
				WriteSingleToReg(MODE_LS_HOST,MODE);	// Start low-speed host mode
				busState_ = LSHOST;					// save state
			}
			break;
		case (NA_STATE):
			/* Not defined state */
			LOG_ERROR("Undefined bus state encountered.");
			busState_ = NA_STATE;
			break;
			
		case (SE0_STATE):
			/* Disconnected state */
			busState_ = SE0;
			WriteSingleToReg((1<<DPPULLDN) | (1<<DMPULLDN) | (1<<HOST) ,MODE);	// Make sure MODE is correctly setup
			break;
	}
	
	return busState_;
}

void MAX3421E::WriteMultipleToReg(uint8_t* bytes,uint8_t reg, uint16_t length)
{
	uint8_t command = ((reg<<3) | (1<<1));	// Shift register to REG0-REG4 and set WR-bit
	
	spi_->SelectSlave();
	spi_->WriteByte(command);
	spi_->WriteBytes(bytes,length);
	spi_->DeselectSlave();
}

void MAX3421E::WriteSingleToReg(uint8_t byte,uint8_t reg)
{
	uint8_t command = ((reg<<3) | 0x02);	// Shift register to REG0-REG4 and set WR-bit (1)
	
	spi_->SelectSlave();
	spi_->WriteByte(command);
	spi_->WriteByte(byte);
	spi_->DeselectSlave();
}

uint8_t MAX3421E::ReadSingleFromReg(uint8_t reg)
{
	spi_->SelectSlave();
	uint8_t command = ((reg<<3));			// Shift register to REG0-REG4 and set R-bit (0)
	spi_->WriteByte(command);				// Send read command
	uint8_t result = spi_->ReadByte();		// Sends empty byte and read
	spi_->DeselectSlave();
	return result;
}

uint8_t* MAX3421E::ReadMultipleFromReg(uint8_t* datacontainer, uint8_t reg, uint8_t len)
{	
	// Send register read command
	spi_->SelectSlave();
	uint8_t command = ((reg<<3));			// Shift register to REG0-REG4 and set R-bit (0)
	spi_->WriteByte(command);				// Send read command
	
	// Start reading bytes
	while(len--)
		*datacontainer++ = spi_->ReadByte();
		
	spi_->DeselectSlave();
	
	return datacontainer;
}

uint8_t MAX3421E::DispatchPacket(uint8_t token, uint8_t ep, uint8_t naklimit)
 { 	 	
	/* Inspired by https://github.com/felis/USB_Host_Shield_2.0 */
	
	uint8_t timeoutDispatch = 1;
	uint8_t timeout = 1;
	uint8_t rcode = hrSUCCES;
	uint8_t temp;
	
	uint16_t nakCount	= 0;
	uint16_t retryCount	= 0;
	
	while(++timeoutDispatch){
		
		WriteSingleToReg((token|ep),HXFR);	// Launch transfer
		
		rcode = 0xFF;
		
		while(++timeout){
			 
			temp = ReadSingleFromReg(HIRQ);
	 	
			if (temp & (1<<HXFRDNIRQ))
			{
				// Reset interrupt bit
	 			WriteSingleToReg((1<<HXFRDNIRQ),HIRQ);
				rcode = hrSUCCES;
	 			break;
			}
		}
	
		if (rcode != hrSUCCES){	// timeout occured return the rcode
			//LOG_ERROR("DispatchPacket - Timeout occured");
			return rcode;
		}
	
		// Analyze the return code (could be NAK, timeout on USB or bad request)
		rcode = (ReadSingleFromReg(HRSL) & 0x0f);
		
		switch (rcode){
			case hrNAK:
				nakCount++;
				if (nakCount > naklimit){
					//LOG_ERROR("Hit NAK limit.");
					return rcode;
				}
				break;
			case hrTIMEOUT:
				retryCount++;
				if (retryCount > retryLimit_){
					//LOG_ERROR("Hit retry limit.");
					return rcode;
				}
				break;
			case hrSTALL:
				/* This usually means that the request is bad so theres actually no reason to retry, but we do it anyways */
				retryCount++;
				if (retryCount > retryLimit_){
					//LOG_ERROR("Hit retry limit.");
					return rcode;
				}
				break;
			default:
				return rcode;
		} 
	}
	
	return rcode;
 }

uint8_t MAX3421E::ControlRequest(uint8_t address, uint8_t ep, uint8_t bmRequestType, uint8_t bRequest, uint8_t wValueLow,uint8_t wValueHigh, uint16_t wIdx, uint16_t wLength, uint8_t* data)
{
	/* Inspired by https://github.com/felis/USB_Host_Shield_2.0 */
	/* and https://pdfserv.maximintegrated.com/en/an/AN3785.pdf */
	
	bool direction = false;		// In or out
	uint8_t rcode;
	SetupPackage setupPkg;
	
	// Set address
	SetAddress(address);
	
	direction = (bmRequestType & 0x80);	// in-transfers has bit 7 set.
	
	/* fill in setup packet */
	setupPkg.bmRequestType	= bmRequestType;
	setupPkg.bRequest		= bRequest;
	setupPkg.wValueLow		= wValueLow;
	setupPkg.wValueHigh		= wValueHigh;
	setupPkg.wIndex			= wIdx;
	setupPkg.wLength		= wLength;
	
	// Load into SUDFIFO
	WriteMultipleToReg((uint8_t*)&setupPkg, SUDFIFO, 8);	// we can pass SetupPackage directly because the struct is packed
	
	rcode = DispatchPacket(SETUP_TOKEN,ep,nakLimit_);	// Dispatch the setup package
	
	if (rcode){
		LOG_ERROR("Dispatch setup packet rcode: %d.",rcode);
		return rcode;
	}
	
	// If data stage is required
	if (data != NULL){
		
		// If IN-Transfer
		if (direction){
			
			// Determine toggle
			ep_.bmRcvToggle = (ReadSingleFromReg(HRSL) & (1<<SNDTOGRD) ? 0 : 1);
			
			// Do InTransfer
			uint16_t nBytesPtr = wLength;
			rcode = InTransfer(&ep_,&nBytesPtr,data,1,nakLimit_);	// Toggle errors are handled in InTransfer

			if (rcode){
				LOG_ERROR("Data stage failed %d", rcode);
				return rcode;
			}
			
		} else	// if OUT-transfer
		{
			//TODO: Implement this stage
		}
		if (rcode){
			LOG_ERROR("Failed to make control request rcode: %d",rcode);
			return rcode;
		}
	}
	
	// Status stage - do specific handshake depending on direction
	return DispatchPacket((direction) ? OUT_HANDSHAKE_TOKEN : IN_HANDSHAKE_TOKEN, ep,nakLimit_);
}

uint8_t MAX3421E::OutTransfer(EpInfo* pep, uint8_t nbytes, uint8_t* data,uint8_t naklimit)
{
	uint8_t rcode = 0;
	
	/* Only supports up to 64 bytes of data for the time being (TODO: should be increased)*/
	if (nbytes > 64)
		return hrDATAERROR;
	
	// Set toggle value
	if (pep->bmRcvToggle) {
		WriteSingleToReg((1<<RCVTOG1),HCTL);
		} else {
		WriteSingleToReg((1<<RCVTOG0),HCTL);
	}
	
	// If sendbuffer is available
	if (ReadSingleFromReg(HIRQ) & (1<<SNDBAVIRQ)){
		
		// Buffer is available load data into sndfifo
		WriteMultipleToReg(data, SNDFIFO, nbytes);
		
		// Load number of bytes into SNDBC
		WriteSingleToReg(nbytes,SNDBC);
		
		while(1){
			// Dispatch OUT token to endpoint
			rcode = DispatchPacket(OUT_TOKEN, pep->epAddr,naklimit);	// Dispatch IN TOKEN to endpoint
		
			// If there was a datatoggle issue
			if (rcode == hrTOGERR){
					
				/* TOGERR indicates error on the toggle therefor we check the toggle again */
				pep->bmRcvToggle = (ReadSingleFromReg(HRSL) & (1<<RCVTOGRD) ? 0 : 1);
					
				if (pep->bmRcvToggle){
					WriteSingleToReg(RCVTOG1,HCTL);
					} else {
					WriteSingleToReg(RCVTOG0,HCTL);
				}
				continue;
			}
		
			if (rcode != hrSUCCES)
			{
				// Something went wrong data might need to be resent!
				// Examine hrslt bits
				uint8_t hrslt = ReadSingleFromReg(HRSL) & 0x0F;	// Only examine hrslt bits
				LOG_ERROR("OutTransfer ERROR: %d",hrslt);
				return hrDATAERROR;
			}
			
			break;
		}
		
		return rcode;
	}
	
	return hrBUFFERFULL;
}

uint8_t MAX3421E::InTransfer(EpInfo* pep, uint16_t* nbytesptr, uint8_t* data,uint8_t bInterval,uint8_t naklimit)
{
	uint8_t rcode = 0;
	uint8_t nRecieved;
	
	uint16_t nBytes = *nbytesptr;
	
	*nbytesptr = 0;
	
	// Set toggle value
	if (pep->bmRcvToggle) {
		WriteSingleToReg((1<<RCVTOG1),HCTL);
		} else {
		WriteSingleToReg((1<<RCVTOG0),HCTL);
	}
	
	// Only exits on break
	while(1)
	{
		rcode = DispatchPacket(IN_TOKEN, pep->epAddr,naklimit);	// Dispatch IN TOKEN to endpoint
		
		// If there was a datatoggle issue
		if (rcode == hrTOGERR){
			
			/* TOGERR indicates error on the toggle therefor we check the toggle again */
			pep->bmRcvToggle = (ReadSingleFromReg(HRSL) & (1<<RCVTOGRD) ? 0 : 1);
			
			if (pep->bmRcvToggle){
				WriteSingleToReg(RCVTOG1,HCTL);
				} else {
				WriteSingleToReg(RCVTOG0,HCTL);
			}
			continue;
		}
		
		if (rcode)
		{
			// Try again later as this is most likely a NAK.
			// If rcode isn't a NAK it could be a STALL but either way it should be handled in the calling function
			// so its fine when we just return the failed rcode.
			break;
		}
		
		// RCVDAVIRQ is asserted if data was recieved without errors
		if ((ReadSingleFromReg(HIRQ) & (1<<RCVDAVIRQ)) == 0)
		{
			//LOG_ERROR("NO RCVDAVIRQ! %d",rcode);
			rcode = hrRECIEVE_ERROR;	// recieve error
			break;
		}
		
		nRecieved = ReadSingleFromReg(RCVBC);		// Number of recieved bytes
		
		// Check if we have recieved more than we can store in data
		if (nRecieved <= nBytes - *nbytesptr)
		{
			ReadMultipleFromReg(data+*nbytesptr,RCVFIFO,nRecieved);		// read full package size
		} else {
			uint16_t rest = nBytes - *nbytesptr;	// calculate rest (amount of data we can read)
			
			// Should never be negative, but check anyways.
			if (rest > 0)
				ReadMultipleFromReg(data+*nbytesptr,RCVFIFO,rest);		// fill rest of data pointer with whatever we can read
		}
		
		WriteSingleToReg((1<<RCVDAVIRQ),HIRQ);		// Clear the IRQ & free the buffer
		*nbytesptr += nRecieved;					// add this packet's byte count to total transfer length
		
		// * If we recieve a package that is less than the max package size this has to be the last transmission (also works if the amount of recieved bytes are 0)
		// * If we have recieved all the bytes specified we're also done.
		if ((nRecieved < pep->maxPktSize) || (*nbytesptr >= nBytes))
		{
			// Save toggle value
			pep->bmRcvToggle = (ReadSingleFromReg(HRSL) & (1<<RCVTOGRD) ? 1 : 0);
			rcode = hrSUCCES;
			
			break;
		} else if (bInterval > 0) {
			/* If the device has a certain polling interval we must adhere to this */
			while (0 < bInterval){
				_delay_ms(1);
				bInterval--;
			}
		}
		
	}
	return rcode;
}

void MAX3421E::SetAddress(uint8_t address)
{
		
	WriteSingleToReg(address,PERADDR);			// Load address in PERADDR register
	uint8_t mode = ReadSingleFromReg(MODE);		// Read current mode
	
	// Set bmLOWSPEED and bmHUBPRE in case of low-speed device, reset them otherwise
	WriteSingleToReg((usb_->lowspeed) ? mode | (1<<LOWSPEED) | (1<<HUBPRE) : mode & ~((1<<LOWSPEED) | (1<<HUBPRE)),MODE);

}

bool MAX3421E::Reset()
{
	uint8_t timeout = 1;
	
	// do chip reset
	WriteSingleToReg((1<<CHIPRES),USBCTL);		// Chip reset
	WriteSingleToReg(0x00,USBCTL);				// Clear register (chip reset and powerdown)
	
	while(timeout++){
		
		/* wait for internal oscillator to be stable */
		if(ReadSingleFromReg(USBIRQ) & (1<<OSCOKIRQ))
			break;
		
		_delay_ms(1);	// wait 255 ms before asserting timeout
	}
	
	return (bool)timeout;
}

// default destructor
MAX3421E::~MAX3421E()
{
	delete spi_;
	
	for (int i = 0; i < sizeof(devRecord_)/sizeof(devRecord_[0]); i++)
		delete[] devRecord_[0].devDescriptor;
		
} //~MAX3421E
