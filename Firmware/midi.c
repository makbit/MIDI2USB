//---------------------------------------------------------------------------//
// File:    Midi.c - MIDI constant definitions, MIDI Event packet parser.    //
// Project: Midi2Usb - MIDI to USB converter.                                //
// Author:  Maximov K.M. (c) https://makbit.com                              //
// Date:    May-August 2020                                                  //
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

// System Exclusive (p.29)
#define MIDI_SYSEX_START          0xF0
#define MIDI_MTC_QUARTER_FRAME    0xF1 // 1 byte data
#define MIDI_SONG_POSITION_PTR    0xF2 // 2 bytes data
#define MIDI_SONG_SELECT          0xF3 // 1 byte data
#define MIDI_TUNE_REQUEST         0xF6 // no data
#define MIDI_SYSEX_END            0xF7
#define MIDI_CLOCK                0xF8 // no data
#define MIDI_TICK                 0xF9 // no data
#define MIDI_START                0xFA // no data
#define MIDI_CONTINUE             0xFB // no data
#define MIDI_STOP                 0xFC // no data
#define MIDI_ACTIVE_SENSE         0xFE // no data
#define MIDI_RESET                0xFF // no data

typedef union
{
	struct PACKET
	{
		uint8_t  cable : 4;            // Cable Number (we use #0)
		uint8_t  cin   : 4;            // Code Index Number (cmd: 0x08)
		uint8_t  cmd;                  // MIDI command (status byte)
		uint8_t  data1;                // MIDI data byte #1
		uint8_t  data2;                // MIDI data byte #2
	};
	uint8_t buffer[sizeof(struct PACKET)];
} MIDI_EVENT_PACKET;

typedef enum {
	IDLE = 0,                          // Parser is idle/ready (in/out)
	STATUS,                            // Read Status/Command byte (out)
	DATA11,                            // Read 1 of 1 data byte (in/out)
	DATA12,                            // Read 1 of 2 data bytes (in/out)
	DATA22,                            // Read 2 of 2 data bytes (in/out)
	SYSEX                              // SysEx state
	//SYSEX2                              // SysEx state
} MIDI_STATE;


//---------------------------------------------------------------------------//
// MIDI Converter (parser), call from IRQ.                                   //
// Input: Data byte from UART (MIDI).                                        //
//---------------------------------------------------------------------------//
void MIDI2USB(uint8_t dataByte)
{
	static MIDI_STATE        state;
	static MIDI_EVENT_PACKET packet;

	if( state==IDLE )
	{
		switch( dataByte & MIDI_CMD_BITMASK )
		{
			case MIDI_NOTE_OFF:
			case MIDI_NOTE_ON:
			case MIDI_AFTER_TOUCH:
			case MIDI_CONTROL_CHANGE:
			case MIDI_PITCH_BEND:
				packet.cin = dataByte >> 4; // Save Code Index Number (cmd)
				packet.cmd = dataByte;      // Save 'status byte' (cmd)
				state      = DATA12;        // Step to 'data byte 1 of 2'
				break;
			case MIDI_PROGRAM_CHANGE:
			case MIDI_CHANNEL_PRESSURE:
				packet.cin = dataByte >> 4; // Save Code Index Number (cmd)
				packet.cmd = dataByte;      // Save 'status byte' (cmd)
				state      = DATA11;        // Step to 'data byte 1 of 1'
				break;
			case MIDI_RESET:
				nMidiCount = 0;             // Reset byte counter in packet
				state      = IDLE;          // Reset state
				break;
			case MIDI_SYSEX_START:
				// skip SysEx
				state = SYSEX;
				break;
			default:
				//--- unknown command: skip.
				break;
		}
	}
	else if( state==DATA12 )
	{
		packet.data1 = dataByte;            // Save 'data byte 1 of 2'
		state        = DATA22;              // Step to 'data byte 2 of 2'
	}
	else if( state==DATA11 )
	{
		state        = IDLE;                // Reset state (finished)
		packet.data1 = dataByte;            // Save 'data byte 1 of 1'
		if( nMidiCount+3 < MIDI_BUF_SIZE )  // Check for free space in buffer
		{
			// Put MIDI message into the stream.
			aMidiBuffer[nMidiCount++] = packet.cin;
			aMidiBuffer[nMidiCount++] = packet.cmd;
			aMidiBuffer[nMidiCount++] = packet.data1;
		}
	}
	else if( state==DATA22 )
	{
		state        = IDLE;                // Reset state (finished)
		packet.data2 = dataByte;            // Save 'data byte 2 of 2'
		if( nMidiCount+4 < MIDI_BUF_SIZE )  // Check for free space
		{
			// Put MIDI message into the stream.
			aMidiBuffer[nMidiCount++] = packet.cin;
			aMidiBuffer[nMidiCount++] = packet.cmd;
			aMidiBuffer[nMidiCount++] = packet.data1;
			aMidiBuffer[nMidiCount++] = packet.data2;
		}
	}
	else if( state==SYSEX )
	{
		if( dataByte==MIDI_SYSEX_END )
		{
			// skip SysEx
			state = IDLE;
		}
		// SKIP...
		state = IDLE;
	}
}

//---------------------------------------------------------------------------//
// USB -> MIDI Converter.                                                    //
// Input: MIDI EVENT Packet in aUsbBuffer[]                                  //
//---------------------------------------------------------------------------//
void USB2MIDI (uint8_t dataIn)
{
	static MIDI_STATE        state;         // Finite-State-Machine variable
	static MIDI_EVENT_PACKET packet;        // USB2MIDI packet (for debug)

	if( state==IDLE )
	{
		if( (dataIn >> 4)==0 )              // Check our Cable #0
		{
			packet.cable = 0;               // Save cable number
			packet.cin   = dataIn << 4;     // Save Code Index Number (cmd)
			state = STATUS;                 // Step to 'status byte' state
		}
	}
	else if( state==STATUS )
	{
		switch( dataIn & MIDI_CMD_BITMASK )
		{
			case MIDI_NOTE_OFF:
			case MIDI_NOTE_ON:
			case MIDI_AFTER_TOUCH:
			case MIDI_CONTROL_CHANGE:
			case MIDI_PITCH_BEND:
				state      = DATA12;        // Step to 'data byte 1 of 2'
				packet.cmd = dataIn;        // Save command (status byte)
				UART0_Write( dataIn );      // Output MIDI command
				break;
			case MIDI_PROGRAM_CHANGE:
			case MIDI_CHANNEL_PRESSURE:
				state      = DATA11;        // Step to 'data byte 1 of 1'
				packet.cmd = dataIn;        // Save command (status byte)
				UART0_Write( dataIn );      // Output MIDI command
				break;
			default:
				//--- unknown command: skip.
				state = IDLE;
				break;
		}
	}
	else if( state==DATA12 )
	{
		state        = DATA22;              // Step to read 'data byte 2 of 2'
		packet.data1 = dataIn;              // Save 'data byte 1 of 2'
		UART0_Write( dataIn );              // Output MIDI data byte
	}
	else if( state==DATA11 )
	{
		state        = IDLE;                // End of packet (finished)
		packet.data1 = dataIn;              // Save 'data byte 1 of 1'
		UART0_Write( dataIn );              // Output MIDI data byte
	}
	else if( state==DATA22 )
	{
		state        = IDLE;                // End of packet (finished)
		packet.data2 = dataIn;              // Save 'data byte 2 of 2'
		UART0_Write( dataIn );              // Output MIDI data byte
	}
	else
	{
		// no SysEx here...
		state = IDLE;
	}
}
