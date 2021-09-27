//---------------------------------------------------------------------------//
// Project: Midi2Usb - MIDI to USB converter.                                //
// File:    Init.c - initialization routines.                                //
// Date:    September 2021, May-August 2020                                  //
//---------------------------------------------------------------------------//
#include "globals.h"
#include <si_toolchain.h>

//---------------------------------------------------------------------------//
// Disable Watch Dog Timer                                                   //
//---------------------------------------------------------------------------//
void WDT_Init (void)
{
	PCA0MD &= ~PCA0MD_WDTE__ENABLED;
}

//---------------------------------------------------------------------------//
// Configure Ports #0, #1                                                    //
//    Push-Pull P0.4 (TX), P0.5 (RX) for UART1, Skip unused 0..3             //
//    Push-Pull P1.0 and P1.1 for LEDS                                       //
//    Enable peripherals                                                     //
//---------------------------------------------------------------------------//
void PORT_Init (void)
{
	P0MDOUT = P0MDOUT_B4__PUSH_PULL;
	P1MDOUT = P1MDOUT_B0__PUSH_PULL | P1MDOUT_B1__PUSH_PULL;

	P0SKIP  = P0SKIP_B0__SKIPPED | P0SKIP_B1__SKIPPED |
	          P0SKIP_B2__SKIPPED | P0SKIP_B3__SKIPPED;

	XBR0 = XBR0_URT0E__DISABLED;
//	XBR0 = XBR0_URT0E__ENABLED;
	XBR1 = XBR1_WEAKPUD__PULL_UPS_ENABLED | XBR1_XBARE__ENABLED;
	XBR2 = XBR2_URT1E__ENABLED | XBR2_SMB1E__DISABLED;
}

//---------------------------------------------------------------------------//
// Configure system (48MHz + USB clock)                                      //
//    Reset on VDD Event                                                     //
//    VDD Monitor enable                                                     //
//    VREF use the output of the internal regulator (unused)                 //
//    HFOSC enabled and div_1                                                //
//    SysClk is HFOSC, USB Clock enabled                                     //
//    Flash one-shot enable                                                  //
//    Prefetch enable                                                        //
//    Wait for HFOSC ready                                                   //
//---------------------------------------------------------------------------//
void SYSCLK_Init (void)
{
	int i;
	RSTSRC = RSTSRC_PORSF__SET;
	VDM0CN = VDM0CN_VDMEN__ENABLED;
	REF0CN = REF0CN_TEMPE__DISABLED | REF0CN_REGOVR__VREG;
	HFO0CN = HFO0CN_IOSCEN__ENABLED | HFO0CN_IFCN__SYSCLK_DIV_1;
	CLKSEL = CLKSEL_CLKSL__HFOSC | CLKSEL_USBCLK__HFOSC;
	FLSCL  = FLSCL_FOSE__ENABLED | FLSCL_FLRT__SYSCLK_BELOW_48_MHZ;
	PFE0CN |= PFE0CN_PFEN__ENABLED;
	for( i = 0; i < 1000; i++ )
	{
		if( HFO0CN & HFO0CN_IFRDY__SET ) break;
	}
}

//---------------------------------------------------------------------------//
// Configure the UART0 using Timer1, for <BAUDRATE> and 8-N-1.               //
//    Clear Timer1 clock control register bits                               //
//    Set Timer1 @prescaler, SYSCLK_DIV_12                                   //
//    Clear Timer1 mode register bits                                        //
//    Set Timer1 mode 2, 8-bit autoreload, CKCON0                            //
//---------------------------------------------------------------------------//
/*
void UART0_Init (void)
{
	uint8_t baudRateDiv = 4000000/31250/2;

	CKCON0 &= ~(CKCON0_SCA__FMASK | CKCON0_T1M__BMASK);
	CKCON0 |= (CKCON0_SCA__SYSCLK_DIV_12 | CKCON0_T1M__PRESCALE);
	TMOD &= ~(TMOD_T1M__FMASK | TMOD_CT1__BMASK | TMOD_GATE1__BMASK);
	TMOD |= TMOD_T1M__MODE2;

	TH1             = -baudRateDiv;    // @31250 Hz (4M/31250/2)
	TL1             = TH1;             // TLow := THigh (TX & RX)
	TCON_TR1        = true;            // Timer1 enable
	SCON0_SMODE     = false;           // UART0 8-bit mode
	SCON0_REN       = true;            // UART0 Receive Enable
	IP_PS0          = true;            // UART0 high priority (PS0)
	IE_ES0          = true;            // Enable UART0 interrupts
}
*/
//---------------------------------------------------------------------------//
// Configure the UART1, for <BAUDRATE> and 8-N-1.                            //
//---------------------------------------------------------------------------//
void UART1_Init (void)
{
	uint8_t div = 4000000/31250/2;            // MIDI Baudrate for 31250 b/s
	SBCON1      = SBCON1_BPS__DIV_BY_12;      // BR prescaler: SYSCLK_DIV_12
	SBCON1     |= SBCON1_BREN__ENABLED;       // Enable baud rate generator
	SBRLH1      = 0xFF;                       // Baud rate generator: High byte
	SBRLL1      = -div;                       // Baud rate generator: Low byte
	SMOD1       = SMOD1_SDL__8_BITS;          // MCE, XBE: off; 8-N-1
	SCON1       = SCON1_REN__RECEIVE_ENABLED; // RX enable, Extra: off.
	EIE2       |= EIE2_ES1__ENABLED;          // Enable UART1 interrupts (ES1)
}

//---------------------------------------------------------------------------//
static volatile bool bUartBusy = 0;
//---------------------------------------------------------------------------//
// Outputs character into UART0 in blocking mode.                            //
//---------------------------------------------------------------------------//
/*
void UART0_Write (uint8_t ch)
{
	bUartBusy = true;                  // Set UART TX flag
	SBUF0     = ch;                    // Write data into UART
	while( bUartBusy );                // Wait I/O complete
}
*/
void UART1_Write (uint8_t ch)
{
	bUartBusy = true;                  // Set UART TX flag
	SBUF1     = ch;                    // Write data into UART
	while( bUartBusy );                // Wait I/O complete
}

//---------------------------------------------------------------------------//
// UART0 interrupt handler                                                   //
//---------------------------------------------------------------------------//
//#pragma vector=UART0_IRQn __interrupt void UART0_ISR(void)                 //
//---------------------------------------------------------------------------//
/*
SI_INTERRUPT (UART0_ISR, UART0_IRQn)
{
	if( SCON0_RI )                     // Check if RX flag is set
	{
		LED_IN    = true;              // Input LED on
		SCON0_RI  = false;             // Clear interrupt flag
		MIDI2USB( SBUF0 );             // Parse MIDI data
	}
	if( SCON0_TI )                     // Check if TX flag is set
	{
		SCON0_TI  = false;             // Clear interrupt flag
		bUartBusy = false;             // Clear global TX flag (complete)
	}
}
*/
//---------------------------------------------------------------------------//
// UART1 interrupt handler (RI is cleared hardware)                          //
//---------------------------------------------------------------------------//
SI_INTERRUPT (UART1_ISR, UART1_IRQn)
{
	while( SCON1 & SCON1_RI__SET )     // Check RI flag (FIFO not empty)
	{
		SCON1 &= ~SCON1_RI__SET;       // Clear RI flag (no Auto clear)
		LED_IN = true;                 // Input LED on
		MIDI2USB( SBUF1 );             // Read MIDI data
	}
	if( SCON1 & SCON1_TI__SET )        // Check if TX flag is set
	{
		SCON1 &= ~SCON1_TI__SET;       // Clear TI interrupt flag
		bUartBusy = false;             // Clear global TX flag (complete)
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