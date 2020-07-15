//---------------------------------------------------------------------------//
// File:    Midi.c - MIDI constant definitions, MIDI Event packet parser.    //
// Project: Midi2Usb - MIDI to USB converter.                                //
// Author:  Maximov K.M. (c) https://makbit.com                              //
// Date:    May 2020                                                         //
//---------------------------------------------------------------------------//
#include "globals.h"

//---------------------------------------------------------------------------//
// MIDI Constants                                                            //
//---------------------------------------------------------------------------//
#define MIDI_NOTE_OFF                  0x80 // 2 bytes data
#define MIDI_NOTE_ON                   0x90 // 2 bytes data
#define MIDI_AFTER_TOUCH               0xA0 // 2 bytes data
#define MIDI_CONTROL_CHANGE            0xB0 // 2 bytes data
#define MIDI_PROGRAM_CHANGE            0xC0 // 1 byte data
#define MIDI_CHANNEL_PRESSURE          0xD0 // 1 byte data
#define MIDI_PITCH_WHEEL               0xE0 // 2 bytes data
#define MIDI_BITMASK                   0xF0

#define MIDI_SYSEX_START               0xF0
#define MIDI_MTC_QUARTER_FRAME         0xF1 // 1 byte data
#define MIDI_SONG_POSITION_PTR         0xF2 // 2 bytes data
#define MIDI_SONG_SELECT               0xF3 // 1 byte data
#define MIDI_TUNE_REQUEST              0xF6 // no data
#define MIDI_SYSEX_END                 0xF7
#define MIDI_CLOCK                     0xF8 // no data
#define MIDI_TICK                      0xF9 // no data
#define MIDI_START                     0xFA // no data
#define MIDI_CONTINUE                  0xFB // no data
#define MIDI_STOP                      0xFC // no data
#define MIDI_ACTIVE_SENSE              0xFE // no data
#define MIDI_RESET                     0xFF // no data

/*
typedef struct
{
    unsigned cable : 4;
    unsigned cin   : 4;
    uint8_t  cmd;
    uint8_t  byte1;
    uint8_t  byte2;
} MIDI_EVENT_PACKET;

enum MIDIcommand {
	NoteOff	              = 0x80,	///< Note Off
	NoteOn                = 0x90,	///< Note On
	AfterTouchPoly        = 0xA0,	///< Polyphonic AfterTouch
	ControlChange         = 0xB0,	///< Control Change / Channel Mode
	ProgramChange         = 0xC0,	///< Program Change
	AfterTouchChannel     = 0xD0,	///< Channel (monophonic) AfterTouch
	PitchBend             = 0xE0,	///< Pitch Bend
	SystemExclusive       = 0xF0,	///< System Exclusive
	TimeCodeQuarterFrame  = 0xF1,	///< System Common - MIDI Time Code Quarter Frame
	SongPosition          = 0xF2,	///< System Common - Song Position Pointer
	SongSelect            = 0xF3,	///< System Common - Song Select
	TuneRequest           = 0xF6,	///< System Common - Tune Request
	Clock                 = 0xF8,	///< System Real Time - Timing Clock
	Start                 = 0xFA,	///< System Real Time - Start
	Continue              = 0xFB,	///< System Real Time - Continue
	Stop                  = 0xFC,	///< System Real Time - Stop
	ActiveSensing         = 0xFE,	///< System Real Time - Active Sensing
	SystemReset           = 0xFF,	///< System Real Time - System Reset
	InvalidType           = 0x00    ///< For notifying errors
};
*/
//---------------------------------------------------------------------------//
// MIDI Converter (parser)                                                   //
// Input: UART data buffer (0..nBytes)                                       //
// Returns: number of bytes converted.                                       //
//---------------------------------------------------------------------------//
uint8_t MIDI2USB(uint8_t SI_SEG_XDATA * aBuffer, uint8_t nBytes)
{
	// Possible values: 0, 0x10, 0x20, etc.
	//const uint8_t cable = 0;
	//static enum STATE { IDLE, COMMAND, BYTE1of1, BYTE1of2, BYTE2, SYSEX } state;

	if( nBytes==0 ) return 0;

	switch( aBuffer[0] & MIDI_BITMASK ) // fix it
	{
		//--- 2 byte data cmd
		case MIDI_NOTE_OFF:
		case MIDI_NOTE_ON:
		case MIDI_AFTER_TOUCH:
		case MIDI_CONTROL_CHANGE:
		case MIDI_PITCH_WHEEL:
			if( (nBytes >= 3) && (nMidiCount+4 <= MIDI_BUF_SIZE) )
			{
				// Put data into MIDI stream.
				aMidiBuffer[nMidiCount+0] = ((aBuffer[0] >> 4) & 0x0F);
				aMidiBuffer[nMidiCount+1] = aBuffer[0];
				aMidiBuffer[nMidiCount+2] = aBuffer[1];
				aMidiBuffer[nMidiCount+3] = aBuffer[2];
				nMidiCount += 4;
			}
			else
			{
				// Nothing was converted. Need more data bytes.
				return 0;
			}
			// 3 bytes were read.
			return 3;

		//--- 1 byte data cmd
		case MIDI_PROGRAM_CHANGE:
		case MIDI_CHANNEL_PRESSURE:
			if( (nBytes >= 2) && (nMidiCount+3 <= MIDI_BUF_SIZE) )
			{
				// Put data into MIDI stream.
				aMidiBuffer[nMidiCount+0] = ((aBuffer[0] >> 4) & 0x0F);
				aMidiBuffer[nMidiCount+1] = aBuffer[0];
				aMidiBuffer[nMidiCount+2] = aBuffer[1];
				nMidiCount += 3;
			}
			else
			{
				// Nothing was converted. Need more data bytes.
				return 0;
			}
			// 2 bytes were read.
			return 2;
		case MIDI_RESET:
			// 1 byte was read.
			return 1;
	}
	LED_OUT = 1 - LED_OUT; // ERROR
	//--- unknown command - skip all bytes.
	return nBytes;
}
