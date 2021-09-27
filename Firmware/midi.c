//---------------------------------------------------------------------------//
// File:    Midi.c - MIDI constant definitions, MIDI Event packet parser.    //
// Project: Midi2Usb - MIDI to USB converter.                                //
// Author:  Maximov K.M. (c) https://makbit.com                              //
// Date:    September 2021, May-August 2020                                  //
//---------------------------------------------------------------------------//
#include "globals.h"

//---------------------------------------------------------------------------//
// MIDI Constants                                                            //
//---------------------------------------------------------------------------//
// Channel Voice Messages (p.9)
#define MIDI_NOTE_OFF             0x80 // 2 bytes data
#define MIDI_NOTE_ON              0x90 // 2 bytes data
#define MIDI_AFTER_TOUCH          0xA0 // 2 bytes data
#define MIDI_CONTROL_CHANGE       0xB0 // 2 bytes data
#define MIDI_PROGRAM_CHANGE       0xC0 // 1 byte data
#define MIDI_CHANNEL_PRESSURE     0xD0 // 1 byte data
#define MIDI_PITCH_BEND           0xE0 // 2 bytes data

#define MIDI_STATUS_BITMASK       0x80 // Status byte mask (p.5)
#define MIDI_CMD_BITMASK          0xF0 // Command bitmask (p.5)

#define GET_MIDI_CMD(arg)         ((arg) & MIDI_CMD_BITMASK)
#define MIDI_IS_STATUS(arg)       (((arg) & MIDI_STATUS_BITMASK) ? 1 : 0)
#define MIDI_IS_DATA(arg)         (((arg) & MIDI_STATUS_BITMASK) ? 0 : 1)
//---------------------------------------------------------------------------//
// SysEx command: F0 <sub-ID> <data bytes> F7                                //
//        Sub-ID: <0-7C | 7E | 7F | (00 00 id)>                              //
// Pages: 100, 107                                                           //
//---------------------------------------------------------------------------//
// System common msg (p.27) System real-time msg (p.30) System Exclusive (p.34)
#define MIDI_SYSEX_START          0xF0 // 0..N bytes data
#define MIDI_TIME_CODE            0xF1 // 1 byte data
#define MIDI_SONG_POSITION        0xF2 // 2 bytes data
#define MIDI_SONG_SELECT          0xF3 // 1 byte data
#define MIDI_TUNE_REQUEST         0xF6 // no data
#define MIDI_SYSEX_END            0xF7 // no data
#define MIDI_CLOCK                0xF8 // no data
#define MIDI_TICK                 0xF9 // no data
#define MIDI_START                0xFA // no data
#define MIDI_CONTINUE             0xFB // no data
#define MIDI_STOP                 0xFC // no data
#define MIDI_ACTIVE_SENSE         0xFE // no data
#define MIDI_SYSTEM_RESET         0xFF // no data

typedef enum {
	MIDI_STATE_IDLE = 0,               // Parser is idle/ready (in/out)
	MIDI_STATE_STATUS,                 // Read Status/Command byte (out)
	MIDI_STATE_DATA,                   // Read single data byte
	MIDI_STATE_DATA1,                  // Read first (1 of 2) data byte
	MIDI_STATE_DATA2,                  // Read second (2 of 2) data byte
	MIDI_STATE_SYSEX                   // SysEx state
} MIDI_STATE;

typedef union
{
	struct PACKET
	{
		uint8_t  cable : 4;            // Cable Number (we use #0)
		uint8_t  cin   : 4;            // Code Index Number (cmd: 0x08)
		uint8_t  cmd;                  // MIDI command (status byte)
		uint8_t  data[2];              // MIDI data bytes (1 or 2)
	}midi;
	uint8_t buffer[sizeof(struct PACKET)];
} MIDI_EVENT_PACKET;

//---------------------------------------------------------------------------//
// MIDI (Uart RX) converter (parser), called from IRQ.                       //
// Input: Data byte from UART (MIDI).                                        //
// Info: see p.16 (midi10), uses 32-bit packets, added zero-padding byte.    //
// MIDI Packet:                                                              //
//              <status/cmd byte> [<data byte #0>, <data byte #1>]           //
//---------------------------------------------------------------------------//
void MIDI2USB(uint8_t dataRX)
{
	static MIDI_STATE        state;
	static MIDI_EVENT_PACKET packet;

	if( MIDI_IS_STATUS(dataRX) )            // System Real Time message
	{
		switch( dataRX )
		{
			case MIDI_SYSTEM_RESET:
				nMidiCount = 0;
				state = MIDI_STATE_IDLE;
				return;
			case MIDI_CLOCK:
			case MIDI_TICK:
			case MIDI_START:
			case MIDI_CONTINUE:
			case MIDI_STOP:
			case MIDI_ACTIVE_SENSE:
			default:
				break;
		}
	}

	if( state == MIDI_STATE_IDLE )
	{
		switch( GET_MIDI_CMD(dataRX) )
		{
			case MIDI_NOTE_OFF:
			case MIDI_NOTE_ON:
			case MIDI_AFTER_TOUCH:
			case MIDI_CONTROL_CHANGE:
			case MIDI_PITCH_BEND:
				packet.midi.cin = dataRX >> 4;   // Save Code Index Number (cmd)
				packet.midi.cmd = dataRX;        // Save 'status byte' (cmd)
				state = MIDI_STATE_DATA1;        // Step to 'data byte 1 of 2'
				break;
			case MIDI_PROGRAM_CHANGE:
			case MIDI_CHANNEL_PRESSURE:
				packet.midi.cin = dataRX >> 4;   // Save Code Index Number (cmd)
				packet.midi.cmd = dataRX;        // Save 'status byte' (cmd)
				state = MIDI_STATE_DATA;         // Step to single data byte
				break;
			case MIDI_SYSEX_START:
				switch( dataRX )
				{
					case MIDI_SYSEX_START:       // Start SysEx stream
						packet.midi.cin = 0;     // Default CIN #0
						packet.midi.cmd = dataRX;
						aMidiBuffer[nMidiCount++] = dataRX;
						state = MIDI_STATE_SYSEX;
						break;
					case MIDI_TIME_CODE:
					case MIDI_SONG_SELECT:       // Cmd with single data byte
						packet.midi.cin = 0;
						packet.midi.cmd = dataRX;
						state = MIDI_STATE_DATA;
						break;
					case MIDI_SONG_POSITION:      // Cmd with two data bytes
						packet.midi.cin = 0;
						packet.midi.cmd = dataRX;
						state = MIDI_STATE_DATA1;
						break;
				}
				break;
			default:
				//--- unknown command: skip.
				break;
		}
	}
	else if( state == MIDI_STATE_DATA1 )
	{
		state = MIDI_STATE_DATA2;                // Step to 'data byte 2 of 2'
		packet.midi.data[0] = dataRX;            // Save 'data byte 1 of 2'
	}
	else if( state == MIDI_STATE_DATA2 )
	{
		state = MIDI_STATE_IDLE;                 // Reset state (finished)
		packet.midi.data[1] = dataRX;            // Save 'data byte 2 of 2'
		if( nMidiCount+4 <= MIDI_BUF_SIZE )      // Check for free space
		{
			// Put MIDI message into the USB stream (32-bit aligned).
			aMidiBuffer[nMidiCount++] = packet.midi.cin;
			aMidiBuffer[nMidiCount++] = packet.midi.cmd;
			aMidiBuffer[nMidiCount++] = packet.midi.data[0];
			aMidiBuffer[nMidiCount++] = packet.midi.data[1];
		}
	}
	else if( state == MIDI_STATE_DATA )
	{
		state = MIDI_STATE_IDLE;                 // Reset state (finished)
		packet.midi.data[0] = dataRX;            // Save 'data byte 1 of 1'
		if( nMidiCount+4 <= MIDI_BUF_SIZE )      // Check for free space
		{
			// Put MIDI message into the USB stream (zero padding).
			aMidiBuffer[nMidiCount++] = packet.midi.cin;
			aMidiBuffer[nMidiCount++] = packet.midi.cmd;
			aMidiBuffer[nMidiCount++] = packet.midi.data[0];
			aMidiBuffer[nMidiCount++] = 0;
		}
	}
	else if( state == MIDI_STATE_SYSEX )
	{
		if( nMidiCount <= MIDI_BUF_SIZE )        // Check for free space
		{
			aMidiBuffer[nMidiCount++] = dataRX;
		}
		if( dataRX == MIDI_SYSEX_END )           // Exit SysEx stream (finish)
		{
			state = MIDI_STATE_IDLE;
		}
	}
}

//---------------------------------------------------------------------------//
// USB -> MIDI Converter.                                                    //
// Input: MIDI EVENT Packet in aUsbBuffer[]                                  //
// USB MIDI Event Packet:                                                    //
//     <cin, cmd> <status/cmd byte> <data byte #0> <data byte #1 or zero>    //
//---------------------------------------------------------------------------//
void USB2MIDI (uint8_t dataIn)
{
	static MIDI_STATE state;                // Finite-State-Machine variable

	if( state == MIDI_STATE_IDLE )
	{
		if( (dataIn >> 4)==0 )              // Check our Cable number (#0)
		{
			state = MIDI_STATE_STATUS;      // Step to 'status/cmd byte' state
		}
	}
	else if( state == MIDI_STATE_STATUS )
	{
		switch( GET_MIDI_CMD(dataIn) )
		{
			case MIDI_NOTE_OFF:
			case MIDI_NOTE_ON:
			case MIDI_AFTER_TOUCH:
			case MIDI_CONTROL_CHANGE:
			case MIDI_PITCH_BEND:
				state = MIDI_STATE_DATA1;   // Step to first 'data byte 1/2'
				UART1_Write( dataIn );      // Output MIDI status byte (cmd)
				break;
			case MIDI_PROGRAM_CHANGE:
			case MIDI_CHANNEL_PRESSURE:
				state = MIDI_STATE_DATA;    // Step to single 'data byte'
				UART1_Write( dataIn );      // Output MIDI status byte (cmd)
				break;
			case MIDI_SYSEX_START:
				UART1_Write( dataIn );      // Output SysEx status byte (cmd)
				if( dataIn == MIDI_SYSEX_START )
				{
					state = MIDI_STATE_SYSEX;
				}
				break;
			default:
				//--- unknown command: skip.
				state = MIDI_STATE_IDLE;
				break;
		}
	}
	else if( state == MIDI_STATE_DATA && MIDI_IS_DATA(dataIn) )
	{
		state = MIDI_STATE_IDLE;            // End of packet (finished)
		UART1_Write( dataIn );              // Output into MIDI this data byte
	}
	else if( state == MIDI_STATE_DATA1 && MIDI_IS_DATA(dataIn) )
	{
		state = MIDI_STATE_DATA2;           // Step to read second byte
		UART1_Write( dataIn );              // Output into MIDI this data byte
	}
	else if( state == MIDI_STATE_DATA2 && MIDI_IS_DATA(dataIn) )
	{
		state = MIDI_STATE_IDLE;            // End of packet (finished)
		UART1_Write( dataIn );              // Output into MIDI this data byte
	}
	else if( state == MIDI_STATE_SYSEX )
	{
		UART1_Write( dataIn );              // Output SysEx data byte
		if( dataIn == MIDI_SYSEX_END )      // Check for SysEx End
		{
			state = MIDI_STATE_IDLE;
		}
	}
	else
	{
		state = MIDI_STATE_IDLE;            // Skip unknown command
	}
}
