
#include "cslib_config.h"
#include "cslib.h"
#include <si_toolchain.h>             // compiler declarations



// Struct defined in sensor_node_routines.h.  Contains all state data for a
// sensor.
SI_SEGMENT_VARIABLE (CSLIB_node[DEF_NUM_SENSORS], SensorStruct_t, SI_SEG_XDATA);

SI_SEGMENT_VARIABLE (CSLIB_numSensors, uint8_t, SI_SEG_DATA)                        = DEF_NUM_SENSORS;
SI_SEGMENT_VARIABLE (CSLIB_sensorBufferSize, uint8_t, SI_SEG_DATA)                  = DEF_SENSOR_BUFFER_SIZE;
SI_SEGMENT_VARIABLE (CSLIB_singleCSLIB_activeSensorDelta, uint16_t, SI_SEG_DATA)          = DEF_SINGLE_ACTIVE_SENSOR_DELTA;
SI_SEGMENT_VARIABLE (CSLIB_buttonDebounce, uint8_t, SI_SEG_DATA)                    = DEF_BUTTON_DEBOUNCE;
SI_SEGMENT_VARIABLE (CSLIB_activeModePeriod, uint16_t, SI_SEG_DATA)              = DEF_ACTIVE_MODE_PERIOD;
SI_SEGMENT_VARIABLE (CSLIB_sleepModePeriod, uint16_t, SI_SEG_DATA)               = DEF_SLEEP_MODE_PERIOD;
SI_SEGMENT_VARIABLE (CSLIB_countsBeforeSleep, uint16_t, SI_SEG_DATA)                = DEF_COUNTS_BEFORE_SLEEP;
SI_SEGMENT_VARIABLE (CSLIB_freeRunSetting, uint8_t, SI_SEG_DATA)                    = DEF_FREE_RUN_SETTING;
SI_SEGMENT_VARIABLE (CSLIB_sleepModeEnable, uint8_t, SI_SEG_DATA)                   = DEF_SLEEP_MODE_ENABLE;
SI_SEGMENT_VARIABLE (CSLIB_activeSensorDelta, uint16_t, SI_SEG_DATA)                = DEF_ACTIVE_SENSOR_DELTA;
SI_SEGMENT_VARIABLE (CSLIB_inactiveSensorDelta, uint16_t, SI_SEG_DATA)              = DEF_INACTIVE_SENSOR_DELTA;
SI_SEGMENT_VARIABLE (CSLIB_averageTouchDelta, uint16_t, SI_SEG_DATA)                = DEF_AVERAGE_TOUCH_DELTA;
