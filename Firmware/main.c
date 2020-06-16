//---------------------------------------------------------------------------//
// Project: Midi2Usb - firmware that connects musical instruments to PC.     //
//                     Converter USB <=> MIDI (in, out).                     //
// File:    Main.c - Firmware entry point, initialization and main loop.     //
// Date:    May 2020                                                         //
// Author:  Maximov K.M. (c) https://makbit.com                              //
// Source:  https://github.com/makbit/Midi2USB                               //
// License: Free for non-commercial use.
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
SI_SEG_IDATA uint8_t aUartBufferTX[UART_BUF_SIZE];
SI_SEG_IDATA uint8_t aUartBufferRX[UART_BUF_SIZE];
SI_SEG_IDATA uint8_t aMidiBuffer[MIDI_BUF_SIZE];
SI_SEG_IDATA uint8_t nUartBytesTX = 0;
SI_SEG_IDATA uint8_t nUartBytesRX = 0;
SI_SEG_IDATA uint8_t nMidiCount   = 0;

static SI_SEG_XDATA uint8_t usbBuffer[SLAB_USB_EP2OUT_MAX_PACKET_SIZE];
//---------------------------------------------------------------------------//
//                                                                           //
//---------------------------------------------------------------------------//
int main( void )
{
	WDT_Init();
	PORT_Init();
	SYSCLK_Init();
	UART_Init();
	USBD_Init( &usbInitStruct );

	IE_EA = 1;

	P1 = 0;
	while(1)
	{
		//----------- only MIDI => USB ----------
		if( (nMidiCount > 0) && (false==USBD_EpIsBusy(EP1IN)) )
		{
			int8_t status;
			IE_EA = 0;
			status = USBD_Write(EP1IN, aMidiBuffer, nMidiCount, false);
			if( status==USB_STATUS_OK )
			{
				nMidiCount = 0;
				LED_IN = 0;
			}
			IE_EA = 1;
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
		USBD_Read(EP2OUT, usbBuffer, sizeof(usbBuffer), false);//true);
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
	UNREFERENCED_ARGUMENT(xferred);
	UNREFERENCED_ARGUMENT(remaining);
	return 0;
}

//---------------------------------------------------------------------------//
#ifndef NDEBUG
void slab_Assert()
{
  while ( 1 );
}
#endif

