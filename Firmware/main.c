//---------------------------------------------------------------------------//
// Project: Midi2Usb - firmware that connects musical instruments to PC.     //
//                     Converter USB <=> MIDI (in, out).                     //
// File:    Main.c - Firmware entry point, initialization and main loop.     //
// Date:    November 2022, September 2021, May-August 2020, version 1.3      //
// Author:  Maximov K.M. (c) https://makbit.com                              //
// Source:  https://github.com/makbit/Midi2USB                               //
// License: Free for non-commercial use.                                     //
//---------------------------------------------------------------------------//
/*---------------------------------------------------------------------------//
 === Include special header for SiLabs MCU EFM8 ===
 [Options] -> [C/C++ Compiler] -> [Preprocessor]
 $PROJ_DIR$
 $PROJ_DIR$/EFM8/sdk/Device/shared/si8051Base
 $PROJ_DIR$/EFM8/sdk/Device/EFM8UB2/inc
 $PROJ_DIR$/EFM8/sdk/Device/EFM8UB2/peripheral_driver/inc
 $PROJ_DIR$/EFM8/sdk/Lib/efm8_usb/inc
 $PROJ_DIR$/EFM8/sdk/Lib/efm8_assert
-----------------------------------------------------------------------------*/
#include <stdint.h>
#include "globals.h"

// Global variables
volatile SI_SEG_IDATA uint8_t nUsbCount  = 0;      // Data bytes in USB->MIDI
volatile SI_SEG_IDATA uint8_t nMidiCount = 0;      // Data bytes in MIDI->USB
volatile SI_SEG_IDATA uint8_t nMidiRTMsg = 0;      // Real-Time Message (0-none)
SI_SEG_XDATA uint8_t aUsbBuffer [USB_BUF_SIZE];    // Buffer for USB->MIDI
SI_SEG_XDATA uint8_t aMidiBuffer[MIDI_BUF_SIZE];   // Buffer for MIDI->USB
SI_SEG_XDATA uint8_t aMidiRTMsg[sizeof(uint32_t)]; // MIDI_RTMsg->USB

//---------------------------------------------------------------------------//
//                                                                           //
//---------------------------------------------------------------------------//
int main( void )
{
	WDT_Init();                             // Disable WDTimer (not used)
	PORT_Init();                            // Initialize ports (UART, LEDs)
	SYSCLK_Init();                          // Set system clock to 48MHz
	UART1_Init();                           // Initialize UART @31250, 8-N-1
	USBD_Init( &usbInitStruct );            // Initialize USB, clock calibrate
	LED_IN  = true;                         // Blink LED (off after usb-cfg)
	LED_OUT = true;                         // Blink LED (off after usb-cfg)
	IE_EA   = true;                         // Global enable IRQ

	while(1)
	{
		//--- MIDI RTMsg => USB
		// System Real Time messages are given priority over other messages.
		// These single-byte messages may occure anywhere in the data stream.
		if( nMidiRTMsg )
		{
			IE_EA  = false;                 // Begin: Critical section
			aMidiRTMsg[0] = 0x0F;           // Cable=0, Code = 0xF
			aMidiRTMsg[1] = nMidiRTMsg;     // Real-Time Message
			aMidiRTMsg[2] = 0;              // not used
			aMidiRTMsg[3] = 0;              // not used
			if( USB_STATUS_OK==USBD_Write(EP1IN,aMidiRTMsg,sizeof(uint32_t),false) )
			{
				nMidiRTMsg = 0;             // Clear MIDI Real-Time Message
			}
			IE_EA  = true;                  // End of: Critical section
		}

		//--- MIDI => USB
		if( nMidiCount >= sizeof(uint32_t) )
		{
			IE_EA  = false;                 // Begin: Critical section
			if( USB_STATUS_OK==USBD_Write(EP1IN,aMidiBuffer,nMidiCount,false) )
			{
				nMidiCount = 0;             // Reset MIDI data byte counter
			}
			IE_EA  = true;                  // End of: Critical section
			LED_IN = false;                 // Turn off input LED
		}

		//--- USB => MIDI
		if( nUsbCount )
		{
			uint8_t i;
			LED_OUT = true;                 // Turn on Led for New packet
			for(i = 0; i < nUsbCount; i++)  // Process every data byte
			{
				USB2MIDI( aUsbBuffer[i] );  // Convert USB packet into MIDI
			}
			nUsbCount = 0;                  // Reset counter
			USBD_Read(EP2OUT, aUsbBuffer, sizeof(aUsbBuffer), true);
			LED_OUT = false;                // Turn off Led, when done
		}
	}
}

//---------------------------------------------------------------------------//
// USB API Callbacks                                                         //
//---------------------------------------------------------------------------//
#if SLAB_USB_STATE_CHANGE_CB
void USBD_DeviceStateChangeCb(USBD_State_TypeDef oldState,
                              USBD_State_TypeDef newState)
{
	// Entering suspend mode, power internal and external blocks down
	if (newState == USBD_STATE_SUSPENDED)
	{
	}
	// Exiting suspend mode, power internal and external blocks up
	if (oldState == USBD_STATE_SUSPENDED)
	{
	}
	// Start reading, when USB is configured and ready
	if (newState == USBD_STATE_CONFIGURED)
	{
		LED_IN  = 0;                        // Turn off LED
		LED_OUT = 0;                        // Turn off LED
		USBD_Read(EP2OUT, aUsbBuffer, sizeof(aUsbBuffer), true);
	}
}
#endif // SLAB_USB_STATE_CHANGE_CB

//---------------------------------------------------------------------------//
//                                                                           //
//---------------------------------------------------------------------------//
uint16_t USBD_XferCompleteCb(uint8_t epAddr,
                             USB_Status_TypeDef status,
                             uint16_t xferred,
                             uint16_t remaining)
{
	UNREFERENCED_ARGUMENT(epAddr);
	UNREFERENCED_ARGUMENT(status);
	UNREFERENCED_ARGUMENT(remaining);

	if( epAddr==EP2OUT && status==USB_STATUS_OK )
	{
		nUsbCount = xferred;
	}
	return 0;
}

//---------------------------------------------------------------------------//
#ifndef NDEBUG
void slab_Assert()
{
  while ( 1 );
}
#endif

