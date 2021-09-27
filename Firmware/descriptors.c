//---------------------------------------------------------------------------//
// Project: Midi2Usb - MIDI to USB converter.                                //
// File:    descriptors.c - this file contains USB descriptor data tables.   //
// Date:    September 2021, May 2020                                         //
// Author:  Maximov K.M. (c) https://makbit.com                              //
// Info:    https://keil.com/pack/doc/mw/USB/html/_u_s_b__descriptors.html   //
//          midi10.pdf ("USB Device Class Definition  for  MIDI Devices")    //
//---------------------------------------------------------------------------//
#include "globals.h"
#include <endian.h>

//---------------------------------------------------------------------------//
//  Constants for MIDI Audio USB devices                                     //
//---------------------------------------------------------------------------//
#define USB_CLASS_AUDIO                    1
#define USB_AUDIO_AUDIOCONTROL             1
#define USB_AUDIO_MIDISTRIMING             3
#define USB_MIDI_INTERFACE_DESCSIZE        7
#define USB_IN_JACK_DESCSIZE               6
#define USB_OUT_JACK_DESCSIZE              9
#define USB_MIDI_CS_EP_DESCSIZE            5
#define USB_MIDI_CS_EP_DESCRIPTOR          0x25
#define USB_MIDI_CS_EP_MS_GENERAL          1
#define USB_AUDIO_EP_DESCSIZE              9
#define MIDI_CS_IF_HEADER                  1
#define MIDI_CS_IF_IN_JACK                 2
#define MIDI_CS_IF_OUT_JACK                3
#define MIDI_JACK_TYPE_EMB                 1
#define MIDI_JACK_TYPE_EXT                 2

//---------------------------------------------------------------------------//
// USB MIDI Device Descriptor                                                //
// Midi10.pdf: Appendix A, p.37                                              //
// VID/PID: https://habr.com/ru/post/255831/                                 //
// https://usb.org/sites/default/files/midi10.pdf                            //
//---------------------------------------------------------------------------//
SI_SEGMENT_VARIABLE
(usbDeviceDesc[], const USB_DeviceDescriptor_TypeDef, SI_SEG_CODE) =
{
	USB_DEVICE_DESCSIZE,               // bLength, 18 bytes
	USB_DEVICE_DESCRIPTOR,             // bDescriptorType, 1
	htole16(0x0110),                   // bcdUSB USB Ver, 1.10
	0x00,                              // bDeviceClass, 0 for Audio
	0x00,                              // bDeviceSubClass, 0 for Audio
	0x00,                              // bDeviceProtocol, 0 for Audio
	SLAB_USB_EP1IN_MAX_PACKET_SIZE,    // bMaxPacketSize0, 64 bytes
	htole16(0x1209),                   // idVendor, Free GPL (or SiLabs 0x10C4)
	htole16(0x7522),                   // idProduct, Makbit MIDI2USB
	htole16(0x0120),                   // bcdDevice, my ver. 1.20
	0x01,                              // iManufacturer string
	0x02,                              // iProduct string
	0x03,                              // iSerialNumber (no serial string)
	0x01                               // bNumConfigurations
};

//---------------------------------------------------------------------------//
// USB Configuration Descriptor                                              //
//---------------------------------------------------------------------------//
SI_SEGMENT_VARIABLE
(usbConfigDesc[], const uint8_t, SI_SEG_CODE) =
{
	//--- Configuration Descriptor header, p.37
	USB_CONFIG_DESCSIZE,               // bLength, 9 bytes
	USB_CONFIG_DESCRIPTOR,             // bDescriptorType, 2
	101,                               // wTotalLength(LSB), 101 bytes
	0x00,                              // wTotalLength(MSB)
	0x02,                              // bNumInterfaces
	0x01,                              // bConfigurationValue
	0x00,                              // iConfiguration (no string)
	0x80,                              // bmAttributes (Bus-powered)
	CONFIG_DESC_MAXPOWER_mA(100),      // bMaxPower (100mA)

	//--- #0 Standard Audio Control (AC) Interface Descriptor, p.38
	USB_INTERFACE_DESCSIZE,            // bLength, 9 bytes
	USB_INTERFACE_DESCRIPTOR,          // bDescriptorType, 4
	0,                                 // bInterfaceNumber, #0
	0,                                 // bAlternateSetting, 0
	0,                                 // bNumEndpoints, 0
	USB_CLASS_AUDIO,                   // bInterfaceClass, 1
	USB_AUDIO_AUDIOCONTROL,            // bInterfaceSubClass, 1
	0,                                 // bInterfaceProtocol, unused
	0,                                 // iInterface, unused

	//--- Class-Specific Audio Control (AC) Interface Header Descriptor, p.39
	USB_INTERFACE_DESCSIZE,            // bLength, 9 bytes
	USB_CS_INTERFACE_DESCRIPTOR,       // bDescriptorType, 0x24
	MIDI_CS_IF_HEADER,                 // bDescriptorSubtype, 0x01
	0x00,                              // bcdADC(LSB)
	0x01,                              // bcdADC(MSB), 0x0100
	0x09,                              // wTotalLength(LSB), 9 bytes
	0x00,                              // wTotalLength(MSB)
	0x01,                              // bInCollection, num of streaming IF
	0x01,                              // baInterfaceNr, IF #1 belongs to this

	//--- #1 Standard MIDI Streaming (MS) Interface Descriptor, p.39
	USB_INTERFACE_DESCSIZE,            // bLength, 9 bytes
	USB_INTERFACE_DESCRIPTOR,          // bDescriptorType, 4
	1,                                 // bInterfaceNumber, #1
	0,                                 // bAlternateSetting, 0
	2,                                 // bNumEndpoints, 2
	USB_CLASS_AUDIO,                   // bInterfaceClass, 1
	USB_AUDIO_MIDISTRIMING,            // bInterfaceSubClass, 3
	0,                                 // bInterfaceProtocol, unused
	0,                                 // iInterface, unused

	// EMB:  IN Jack #1 <-----> EXT: OUT Jack #4
	// EMB: OUT Jack #3 <-----> EXT:  IN Jack #2

	//--- Class-Specific MS Interface Header Descriptor, p.40
	USB_MIDI_INTERFACE_DESCSIZE,       // bLength, 7 bytes
	USB_CS_INTERFACE_DESCRIPTOR,       // bDescriptorType, 0x24
	MIDI_CS_IF_HEADER,                 // bDescriptorSubtype, 0x01
	0x00,                              // bcdADC(LSB)
	0x01,                              // bcdADC(MSB), 0x0100 (version)
	0x41,                              // wTotalLength(LSB), 65 bytes
	0x00,                              // wTotalLength(MSB)

	//--- MIDI IN JACK EMB(it connects to the USB OUT Endpoint), p.40
	USB_IN_JACK_DESCSIZE,              // bLength, 6 bytes
	USB_CS_INTERFACE_DESCRIPTOR,       // bDescriptorType, 0x24
	MIDI_CS_IF_IN_JACK,                // bDescriptorSubtype, 0x02
	MIDI_JACK_TYPE_EMB,                // bJackType, 0x01 (embedded)
	1,                                 // bJackID, #1
	0,                                 // Jack string descriptor, unused
	//--- MIDI IN JACK EXT, p.40
	USB_IN_JACK_DESCSIZE,              // bLength, 6 bytes
	USB_CS_INTERFACE_DESCRIPTOR,       // bDescriptorType, 0x24
	MIDI_CS_IF_IN_JACK,                // bDescriptorSubtype, 0x02
	MIDI_JACK_TYPE_EXT,                // bJackType, 0x02 (external)
	2,                                 // bJackID, #2
	0,                                 // Jack string descriptor, unused

	//--- MIDI OUT JACK EMB (connects to IN Endpoint), p.41
	USB_OUT_JACK_DESCSIZE,             // bLength, 9 bytes
	USB_CS_INTERFACE_DESCRIPTOR,       // bDescriptorType, 0x24
	MIDI_CS_IF_OUT_JACK,               // bDescriptorSubtype, 0x03
	MIDI_JACK_TYPE_EMB,                // bJackType, 0x01
	3,                                 // bJackID
	1,                                 // bNrInputPins
	2,                                 // baSourceID, this <=> Jack #2
	1,                                 // baSourcePin
	0,                                 // iJack, unused
	//--- MIDI OUT JACK EXT, p.41
	USB_OUT_JACK_DESCSIZE,             // bLength, 9 bytes
	USB_CS_INTERFACE_DESCRIPTOR,       // bDescriptorType, 0x24
	MIDI_CS_IF_OUT_JACK,               // bDescriptorSubtype, 0x03
	MIDI_JACK_TYPE_EXT,                // bJackType, 0x02
	4,                                 // bJackID
	1,                                 // bNrInputPins
	1,                                 // baSourceID, this <=> Jack #1
	1,                                 // baSourcePin
	0,                                 // iJack, unused

	//  IN Jack Emb #1 <=====> OUT EP 0x02
	// OUT Jack Emb #3 <=====>  IN EP 0x81

	//--- Standard BULK IN Endpoint Descriptor
	USB_AUDIO_EP_DESCSIZE,             // bLength, 9 bytes
	USB_ENDPOINT_DESCRIPTOR,           // bDescriptorType, 0x05
	USB_EP_DIR_IN | 0x01,              // bEndpointAddress, IN EP #1 (0x81)
	USB_EPTYPE_BULK,                   // bmAttributes, 0x02 (bulk)
	SLAB_USB_EP1IN_MAX_PACKET_SIZE,    // wMaxPacketSize(LSB), 64
	0,                                 // wMaxPacketSize(LSB), 0
	0,                                 // bInterval, unused
	0,                                 // bRefresh, unused
	0,                                 // bSynchAddress, unused
	//--- Class-specific MIDI Stream BULK OUT Endpoint Descriptor
	USB_MIDI_CS_EP_DESCSIZE,           // bLength, 5 bytes
	USB_MIDI_CS_EP_DESCRIPTOR,         // bDescriptorType, 0x25
	USB_MIDI_CS_EP_MS_GENERAL,         // bDescriptorSubtype, 0x01
	1,                                 // bNumEmbMIDIJack
	3,                                 // baAssocJackID, OUT Jack Emb #3

	//--- Standard BULK OUT Endpoint Descriptor
	USB_AUDIO_EP_DESCSIZE,             // bLength, 9 bytes
	USB_ENDPOINT_DESCRIPTOR,           // bDescriptorType, 0x05
	USB_EP_DIR_OUT | 0x02,             // bEndpointAddress, OUT EP #2 (0x02)
	USB_EPTYPE_BULK,                   // bmAttributes, 0x02 (bulk)
	SLAB_USB_EP2OUT_MAX_PACKET_SIZE,   // wMaxPacketSize(LSB), 64
	0,                                 // wMaxPacketSize(LSB), 0
	0,                                 // bInterval, unused
	0,                                 // bRefresh, unused
	0,                                 // bSynchAddress, unused
	//--- Class-specific MIDI Stream BULK IN Endpoint Descriptor
	USB_MIDI_CS_EP_DESCSIZE,           // bLength, 5 bytes
	USB_MIDI_CS_EP_DESCRIPTOR,         // bDescriptorType, 0x25
	USB_MIDI_CS_EP_MS_GENERAL,         // bDescriptorSubtype, 0x01
	1,                                 // bNumEmbMIDIJack
	1                                  // baAssocJackID, IN Jack Emb #1
};

//---------------------------------------------------------------------------//
// USB String Descriptors                                                    //
//---------------------------------------------------------------------------//
#define LANG_STRING          htole16( SLAB_USB_LANGUAGE )
#define MFR_STRING           'M','a','x','i','m','o','v',' ','K','.','M','.','\0'
#define MFR_SIZE             13
#define PROD_STRING          'M','I','D','I','2','U','S','B','\0'
#define PROD_SIZE            9
#define SER_STRING           '1','.','2','\0'
#define SER_SIZE             4

LANGID_STATIC_CONST_STRING_DESC(langDesc[], LANG_STRING);
UTF16LE_PACKED_STATIC_CONST_STRING_DESC( mfrDesc[],  MFR_STRING,  MFR_SIZE );
UTF16LE_PACKED_STATIC_CONST_STRING_DESC( prodDesc[], PROD_STRING, PROD_SIZE );
UTF16LE_PACKED_STATIC_CONST_STRING_DESC( serDesc[],  SER_STRING,  SER_SIZE );

//-----------------------------------------------------------------------------
const USB_StringTable_TypeDef usbStringTable[] =
{
	(SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_CODE))langDesc,
	(SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_CODE))mfrDesc,
	(SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_CODE))prodDesc,
	(SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_CODE))serDesc,
};

//-----------------------------------------------------------------------------
const USBD_Init_TypeDef usbInitStruct =
{
	(SI_VARIABLE_SEGMENT_POINTER(, void, SI_SEG_GENERIC))usbDeviceDesc,
	(SI_VARIABLE_SEGMENT_POINTER(, void, SI_SEG_GENERIC))usbConfigDesc,
	(SI_VARIABLE_SEGMENT_POINTER(, void, SI_SEG_GENERIC))usbStringTable,
	sizeof(usbStringTable) / sizeof(usbStringTable[0])
};

