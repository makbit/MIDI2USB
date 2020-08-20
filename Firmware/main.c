//---------------------------------------------------------------------------//
// Project: Midi2Usb - firmware that connects musical instruments to PC.     //
//                     Converter USB <=> MIDI (in, out).                     //
// File:    Main.c - Firmware entry point, initialization and main loop.     //
// Date:    May-August 2020, version 0.2                                     //
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
volatile SI_SEG_IDATA uint8_t nUsbCount  = 0;    // Data bytes in USB->MIDI
volatile SI_SEG_IDATA uint8_t nMidiCount = 0;    // Data bytes in MIDI->USB
SI_SEG_XDATA uint8_t aUsbBuffer [USB_BUF_SIZE];  // Buffer for USB->MIDI
SI_SEG_XDATA uint8_t aMidiBuffer[MIDI_BUF_SIZE]; // Buffer for MIDI->USB

//---------------------------------------------------------------------------//
//                                                                           //
//---------------------------------------------------------------------------//
int main( void )
{
	WDT_Init();                             // Disable WDTimer (not used)
	PORT_Init();                            // Initialize ports (UART, LEDs)
	SYSCLK_Init();                          // Set system clock to 48MHz
	UART0_Init();                           // Initialize UART0 @31250, 8-N-1
	USBD_Init( &usbInitStruct );            // Initialize USB, clock calibrate
	LED_IN  = 1;                            // Blink LED
	LED_OUT = 1;                            // Blink LED
	IE_EA   = 1;                            // Global enable IRQ

	while(1)
	{
		//--- MIDI => USB
		if( nMidiCount > 0 )
		{
			IE_EA  = 0;                     // Begin: Critical section
			if( USB_STATUS_OK==USBD_Write(EP1IN,aMidiBuffer,nMidiCount,false) )
			{
				nMidiCount = 0;             // Reset MIDI data byte counter
			}
			IE_EA  = 1;                     // End of: Critical section
			LED_IN = 0;                     // Turn off input LED
		}

		//--- USB => MIDI
		if( nUsbCount )
		{
			uint8_t i;
			LED_OUT = 1;                    // Turn on Led on New packet
			for(i = 0; i < nUsbCount; i++)  // Process every data byte
			{
				USB2MIDI( aUsbBuffer[i] );  // Convert USB packet into MIDI
			}
			nUsbCount = 0;                  // Reset counter
			USBD_Read(EP2OUT, aUsbBuffer, sizeof(aUsbBuffer), true);
			LED_OUT = 0;                    // Turn off Led, when done
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

