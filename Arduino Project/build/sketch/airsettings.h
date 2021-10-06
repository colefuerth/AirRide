#line 1 "/home/pi/Air Ride Project I/Arduino Project/airsettings.h"
#ifndef airsettings_h
#define airsettings_h

// define state table, DO NOT MODIFY
#define IDLEMODE 0
#define PSIMODE 1
#define HEIGHTMODE 2
#define ON 1
#define OFF 0

// ADVANCED: EEPROM data
#define EE_VER 1    // EEPROM version number
#define EE_DATA_START 20 // data starting address
#define EE_PROF_START 500 // profiles starting address
#define EE_FORCE_CLEAR_ON_INIT // forces EEPROM to clear itself on any init_eeprom() call, disables warnings
//#define EE_IGNORE_VERSIONS // ignore version mismatch

// set log levels
#define SERIALLOGGING // define whether to send logs to pi
#define DEBUG_PORT Serial
#define BAUD_R 115200
#define EEPROM_DEBUG 0
#define PROFILE_DEBUG 0

// default values for profiles and profile manager
#define DEFAULT_PROFILE 0   // default profile to load on boot, can be changed later, this is just a default for compilation
#define DEFAULT_PROFILE_NAME "new_profile" // default profile name when creating a new profile
#define DEFAULT_MODE PSIMODE // default mode when creating a profile
#define DEFAULT_VAL 20 // default value, associated with mode chosen. PSIMODE is selected, so this stands for 20PSI default


// default pressure sensor high and low voltages, use floats; matched with high and low PSI values below
#define P_LOW_VOLTS 1.0 * ( 1024.0 / 5.0 ) // 1V low
#define P_LOW_PRES 15.0 // 15psi low
#define P_HI_VOLTS 4.0 * ( 1024.0 / 5.0 )  // 4V high
#define P_HIGH_PRES 50.0 // 150psi high

// default ride height sensor high and low voltages and heights, in millimeters
#define H_LOW_VOLTS 1.0 * ( 1024.0 / 5.0 )
#define H_LOW_HEIGHT 30.0
#define H_HI_VOLTS 4.0 * ( 1024.0 / 5.0 )
#define H_HIGH_HEIGHT 100.0

// Compressor settings
#define COMP_TARGETPSI 105   // default pressure to maintain
#define COMP_IDLEPSI 60   // pressure to idle at when inactive
#define COMP_HYS 15        // hysteresis setting (psi tolerance)
#define COMP_MAXPSI 150  // absolute maximum pressure before ESTOP and fault
#define COMP_MAXRUNTIME 5 * 60 // 5 minutes before cooldown
#define COMP_COOLTIME 1 * 60   // 1 minute cooldown before running again

// SHOCK SETTINGS
#define SHK_MAX_PSI 50

// PIN SETTINGS
#define SHOCK_VALVE_PIN_START 2
#define SHOCK_HEIGHT_PIN_START SHOCK_VALVE_PIN_START + 4
#define SHOCK_PRESSURE_PIN_START SHOCK_VALVE_PIN_START + 8
#define COMPRESSOR_MOTOR_PIN SHOCK_VALVE_PIN_START + 12
#define COMPRESSOR_PRESSURE_PIN COMPRESSOR_MOTOR_PIN + 1

// Shock settings
//#define IDLEPSI 60


// TODO: NEED TO CREATE A TABLE FOR EEPROM DEVICE SIZES

#endif