//---------------------------------------------------------------------------//
// Project: Midi2Usb - MIDI to USB converter.                                //
// File:    Init.c - initialization routines.                                //
// Date:    May-August 2020                                                  //
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
		if( HFO0CN & 0x40 ) break;     // Wait for oscillator READY flag
	}
}

//---------------------------------------------------------------------------//
// Configure the UART0 using Timer1, for <BAUDRATE> and 8-N-1.               //
//---------------------------------------------------------------------------//
void UART0_Init (void)
{
	uint8_t     div = 4000000/31250/2; // MIDI Baudrate @31250
	SCON0           = 0x10;            // 8-bit mode, TI/RI clear, Rcv Enable
	CKCON0         &= ~0x0B;           // Timer1 clock control (clear)
	CKCON0         |= 0x00;            // Timer1 @prescaler, SYSCLK_DIV_12
	TMOD           &= ~0xF0;           // Timer1 clear mode bits
	TMOD           |= 0x20;            // Timer1 8-bit autoreload
	TH1             = -div;            // @31250 Hz (12M/31250/2)
	TL1             = TH1;             // TLow := THigh
	TCON_TR1        = 1;               // Timer1 enable
	IP             |= 0x10;            // UART0 high priority (PS0)
	IE_ES0          = 1;               // Enable UART0 interrupts
}

//---------------------------------------------------------------------------//
// Configure the UART1, for <BAUDRATE> and 8-N-1.                            //
//---------------------------------------------------------------------------//
void UART1_Init (void)
{
	uint8_t     div = 4000000/31250/2; // MIDI Baudrate @31250
	SCON1           = 0x10;            // RX enable, Extra: off.
	SMOD1           = 0x0C;            // MCE, SPT, PE, XBE: off; SDL: 8-bits.
	SBCON1          = 0;               // Baud rate prescaler: SYSCLK_DIV_12
	SBCON1         |= 0x40;            // Enable baud rate generator
	SBRLH1          = 0xFF;            // Baud rate generator: High byte
	//SBRLH1          = 0;               // Baud rate generator: High byte
	SBRLL1          = -div;            // Baud rate generator: Low byte
	EIE2           |= 0x02;            // Enable UART1 interrupts (ES1)
}

//---------------------------------------------------------------------------//
static volatile SI_SEG_IDATA uint8_t bUartTX = 0;
//---------------------------------------------------------------------------//
// Outputs character into UART0 in blocking mode.                            //
//---------------------------------------------------------------------------//
void UART0_Write (uint8_t ch)
{
	bUartTX  = 1;                      // Set UART0 TX flag
	SBUF0    = ch;                     // Write data into UART
	while( bUartTX );                  // Wait I/O complete
}

//---------------------------------------------------------------------------//
// UART0 interrupt handler                                                   //
//---------------------------------------------------------------------------//
//#pragma vector=UART0_IRQn __interrupt void UART0_ISR(void)                 //
//---------------------------------------------------------------------------//
SI_INTERRUPT (UART0_ISR, UART0_IRQn)
{
	if( SCON0_RI == 1 )                // Check if RX flag is set
	{
		LED_IN   = 1;                  // Input LED on
		SCON0_RI = 0;                  // Clear interrupt flag
		MIDI2USB( SBUF0 );             // Parse MIDI data
	}
	if( SCON0_TI == 1 )                // Check if TX flag is set
	{
		SCON0_TI = 0;                 // Clear interrupt flag
		bUartTX  = 0;                 // Clear global TX flag (complete)
	}
}

//---------------------------------------------------------------------------//
// UART1 interrupt handler                                                   //
//---------------------------------------------------------------------------//
SI_INTERRUPT (UART1_ISR, UART1_IRQn)
{
	if( SCON1 & 0x01 )                 // Check RI flag (FIFO not empty)
	{
		LED_IN = 1;                    // Input LED on
		MIDI2USB( SBUF1 );             // Read MIDI data, Auto clear IRQ flag
	}
	if( SCON1 & 0x02 )                 // Check if TX flag is set
	{
		SCON1 &=~0x02;                 // Clear interrupt flag
	}
}

//===========================================================================//
/*
static volatile uint16_t nSystemTick;
//---------------------------------------------------------------------------//
void TIMER_Init (void)
{
	uint16_t div    = 65536 - 48000/12;// 1kHz
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
		nSystemTick = 0;
	LED_OUT = 1-LED_OUT;
	}
}
*/