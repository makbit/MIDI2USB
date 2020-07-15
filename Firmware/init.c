//---------------------------------------------------------------------------//
// Project: Midi2Usb - MIDI to USB converter.                                //
// File:    Init.c - initialization routines.                                //
// Date:    May 2020                                                         //
//---------------------------------------------------------------------------//
#include "globals.h"
#include <si_toolchain.h>

//---------------------------------------------------------------------------//
// Disable Watch Dog Timer                                                   //
//---------------------------------------------------------------------------//
void WDT_Init (void)
{
	PCA0MD        &= ~0x40;            // Disable WDT
}

//---------------------------------------------------------------------------//
// Configure Port0 for UART, Port1 for LEDS                                  //
//---------------------------------------------------------------------------//
void PORT_Init (void)
{
	P0MDOUT       |= 0x10;             // Push-Pull mode for UART_TX P0.4
	P1MDOUT       |= 0x03;             // Push-Pull mode for LEDS P1.0, P1.1
	XBR0           = 0x01;             // Enable UART on P0.4(TX), P0.5(RX)
	XBR1           = 0x40;             // Enable crossbar and weak pull-ups
	P1             = 0;                // Turn off Leds
}

//---------------------------------------------------------------------------//
// Configure system clock for 48MHz + USB clock                              //
//---------------------------------------------------------------------------//
void SYSCLK_Init (void)
{
	int i;
	RSTSRC    = 0x02;                  // Reset on VDD Event
	VDM0CN    = 0x80;                  // VDD Monitor enable
	FLSCL     = 0x80;                  // Flash one-shot enable
	PFE0CN   |= 0x20;                  // Prefetch enable
	CLKSEL    = 0x03;                  // SYSCLK (HFOSC), USBCLK (HFOSC)
	HFO0CN    = 0x83;                  // IOSCEN, IFCN=3 (SYSCLK_div_1)
	for( i = 0; i < 1000; i++ )
	{
		if( HFO0CN & 0x40 ) break;     // wait for oscillator READY flag
	}
}

//---------------------------------------------------------------------------//
// Configure the UART0 using Timer1, for <BAUDRATE> and 8-N-1.               //
//---------------------------------------------------------------------------//
void UART_Init (void)
{
	uint8_t     div = 12000000/31250/2;// MIDI Baudrate @31250
	SCON0           = 0x10;            // 8-bit mode, TI/RI clear, Rcv Enable
	CKCON0         &= ~0x0B;           // Timer1 clock control (clear)
	CKCON0         |= 0x01;            // Timer1 @prescaler, SYSCLK_DIV_4
	TMOD           &= ~0xF0;           // Timer1 clear mode bits
	TMOD           |= 0x20;            // Timer1 8-bit autoreload
	TH1             = -div;            // @31250 Hz (12M/31250/2)
	TL1             = TH1;             // Low:=High
	TCON_TR1        = 1;               // Timer1 enable
	IP             |= 0x10;            // UART0 high priority (PS0)
	IE_ES0          = 1;               // Enable UART0 interrupts
}

//---------------------------------------------------------------------------//
// UART0 interrupt handler                                                   //
//---------------------------------------------------------------------------//
//#pragma vector=UART0_IRQn __interrupt void UART0_ISR(void)                 //
//---------------------------------------------------------------------------//
SI_INTERRUPT (UART0_ISR, UART0_IRQn)
{
	if( SCON0_RI == 1 )            // Check if RX flag is set
	{
		SCON0_RI = 0;              // Clear interrupt flag
		LED_IN = 1;                // Blink led
		if( nUartBytesRX < UART_BUF_SIZE )
		{
			aUartBufferRX[nUartBytesRX] = SBUF0;
			nUartBytesRX++;
		}
		// Convert MIDI Packet to USB MIDI Event.
		nUartBytesRX -= MIDI2USB(aUartBufferRX, nUartBytesRX);
		// MIDI2USB(u8DataByte, 1);
	}
	if( SCON0_TI == 1 )            // Check if TX flag is set
	{
		SCON0_TI = 0;              // Clear interrupt flag
	}
}

/*
//===========================================================================//
static uint16_t nSystemTick;
//---------------------------------------------------------------------------//
void TIMER_Init (void)
{
	uint16_t div    = 65536 - 48000/12;// 100kHz
	TMR2CN0         = 0;               // Timer2 SysClk/12, Auto, RunCtrl off
	TMR2RL          = div;             // Timer2: 100kHz
	TMR2            = div;             // Timer2 initial counter value
	TMR2CN0_TR2     = 1;               // Run Timer2
	IE_ET2          = 1;               // Timer2 interrupt enabled
}

SI_INTERRUPT (Timer2_ISR, TIMER2_IRQn)
{
	nSystemTick++;                     // Every millisecond (1/1000s)
	TMR2CN0_TF2H = 0;                  // Reset IRQ flag

	if( nSystemTick==1000 )
	{
		nMidiCount = 2;
		nSystemTick = 0;
	LED_OUT = 1-LED_OUT;
	}
}
*/