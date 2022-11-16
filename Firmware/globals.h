//---------------------------------------------------------------------------//
// File:    Globals.h - Definitions, Macros, Global variables and functions. //
// Project: Midi2Usb - MIDI to USB converter.                                //
// Author:  Maximov K.M. (c) https://makbit.com                              //
// Date:    November 2022, September 2021, May-August 2020                   //
//---------------------------------------------------------------------------//
#include <stdint.h>
#include <stdbool.h>
#include <SI_EFM8UB2_Defs.h>
#include <efm8_usb.h>

#define LED_IN          P1_B1
#define LED_OUT         P1_B0

#define MIDI_BUF_SIZE   (SLAB_USB_EP1IN_MAX_PACKET_SIZE)
#define USB_BUF_SIZE    (SLAB_USB_EP2OUT_MAX_PACKET_SIZE)

extern volatile SI_SEG_IDATA uint8_t nUsbCount;
extern volatile SI_SEG_IDATA uint8_t nMidiCount;
extern volatile SI_SEG_IDATA uint8_t nMidiRTMsg;
extern          SI_SEG_XDATA uint8_t aUsbBuffer [USB_BUF_SIZE];
extern          SI_SEG_XDATA uint8_t aMidiBuffer[MIDI_BUF_SIZE];

extern const USBD_Init_TypeDef usbInitStruct;
//---------------------------------------------------------------------------//
// Initialization section                                                    //
//---------------------------------------------------------------------------//
extern void WDT_Init    (void);
extern void PORT_Init   (void);
extern void SYSCLK_Init (void);
extern void TIMER_Init  (void);
extern void UART0_Init  (void);
extern void UART1_Init  (void);
extern void MIDI2USB    (uint8_t dataByte);
extern void USB2MIDI    (uint8_t dataSize);
extern void UART0_Write (uint8_t ch);
extern void UART1_Write (uint8_t ch);
//---------------------------------------------------------------------------//
