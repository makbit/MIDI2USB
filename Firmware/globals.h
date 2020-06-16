//---------------------------------------------------------------------------//
// File:    Globals.h - Definitions, Macros, Global variables and functions. //
// Project: Midi2Usb - MIDI to USB converter.                                //
// Author:  Maximov K.M. (c) https://makbit.com                              //
// Date:    May 2020                                                         //
//---------------------------------------------------------------------------//
#include <stdint.h>
#include <SI_EFM8UB2_Defs.h>
#include "efm8_usb.h"

#define LED_IN          P1_B1
#define LED_OUT         P1_B0

#define UART_BUF_SIZE   30
#define MIDI_BUF_SIZE   40

extern SI_SEG_IDATA uint8_t aUartBufferTX[UART_BUF_SIZE];
extern SI_SEG_IDATA uint8_t aUartBufferRX[UART_BUF_SIZE];
extern SI_SEG_IDATA uint8_t aMidiBuffer[MIDI_BUF_SIZE];
extern SI_SEG_IDATA uint8_t nUartBytesRX;
extern SI_SEG_IDATA uint8_t nUartBytesTX;
extern SI_SEG_IDATA uint8_t nMidiCount;

extern const USBD_Init_TypeDef usbInitStruct;
//---------------------------------------------------------------------------//
// Initialization section                                                    //
//---------------------------------------------------------------------------//
extern void    WDT_Init    (void);
extern void    PORT_Init   (void);
extern void    SYSCLK_Init (void);
extern void    TIMER_Init  (void);
extern void    UART_Init   (void);
extern uint8_t MIDI2USB    (uint8_t* aBuffer, uint8_t nBytes);
//---------------------------------------------------------------------------//
