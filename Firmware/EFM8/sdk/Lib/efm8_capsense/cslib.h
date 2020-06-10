//-----------------------------------------------------------------------------
// cslib.h

//-----------------------------------------------------------------------------
// Copyright 2014 Silicon Laboratories, Inc.
// http://www.silabs.com
//

#ifndef _LIBRARY_PUBLICS_H
#define _LIBRARY_PUBLICS_H

// Header file for CSLibrary.lib v 1.2
#include <si_toolchain.h>
// Defines the depth of the buffers for raw and processed data
// used in the sensor node struct.
#define DEF_SENSOR_BUFFER_SIZE 2


// Algorithm state init/update functions
void CSLIB_initHardware(void);
void CSLIB_initLibrary(void);
void CSLIB_update(void);

// Sensor access API
typedef struct
{
   uint16_t rawBuffer[DEF_SENSOR_BUFFER_SIZE];
   uint16_t processBuffer[DEF_SENSOR_BUFFER_SIZE];
   uint16_t currentBaseline;
   uint8_t touchDeltaDiv16;
   uint8_t activeIndicator;
   uint8_t debounceCounts;
   int8_t baselineAccumulator;
   SI_UU32_t expValue;
} SensorStruct_t;

#define DEBOUNCE_ACTIVE_MASK 0x80
#define SINGLE_ACTIVE_MASK   0x40


void CSLIB_initSensorStruct_ts(void);
void CSLIB_resetSensorStruct_t(uint8_t sensorIndex, uint16_t fillValue);
uint8_t CSLIB_anySensorSingleActive(void);
uint8_t CSLIB_isSensorSingleActive(sensorIndex);
uint8_t CSLIB_anySensorDebounceActive(void);
uint8_t CSLIB_isSensorDebounceActive(sensorIndex);

uint8_t CSLIB_nodePushRaw(uint8_t sensorIndex, uint16_t newValue);
uint8_t CSLIB_nodePushProcess(uint8_t sensorIndex, uint16_t newValue);

uint16_t CSLIB_nodeGetRaw(uint8_t sensorIndex, uint8_t index);
uint16_t CSLIB_nodeGetProcess(uint8_t sensor_index, uint8_t index);
uint8_t CSLIB_getActiveDebounceCount(uint8_t sensorIndex);
uint16_t CSLIB_getUnpackedTouchDelta(uint8_t sensorIndex);

void CSLIB_nodeOverwriteProcess(uint8_t, uint8_t, uint16_t);

uint8_t* CSLIB_getInfoArrayPointer(void);

extern xdata SensorStruct_t CSLIB_node[];

extern SI_SEGMENT_VARIABLE (CSLIB_activeSensorDelta, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_inactiveSensorDelta, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_numSensors, uint8_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_sensorBufferSize, uint8_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_averageTouchDeltaDefault, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_activeSensorDeltaDefault, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_inactiveSensorDeltaDefault, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_singleCSLIB_activeSensorDelta, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_buttonDebounce, uint8_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_activeModePeriod, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_sleepModePeriod, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_countsBeforeSleep, uint16_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_freeRunSetting, uint8_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_sleepModeEnable, uint8_t, SI_SEG_DATA);
extern SI_SEGMENT_VARIABLE (CSLIB_averageTouchDelta, uint16_t, SI_SEG_DATA);
// Low power routines
void CSLIB_lowPowerUpdate(void);

// Noise information
extern idata uint16_t CSLIB_systemNoiseAverage;
uint8_t CSLIB_checkNoiseOnChannel(uint8_t index);

#endif
