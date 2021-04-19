#ifndef airsettings_h
#define airsettings_h

// set log levels
#define SERIALLOGGING
#define DEBUG_PORT Serial
#define BAUD_R 115200

// pressure sensor high and low voltages, use floats; matched with high and low PSI values below
#define P_LOW_VOLTS 1.0 * ( 1024.0 / 5.0 )
#define P_LOW_PRES 30.0
#define P_HI_VOLTS 4.0 * ( 1024.0 / 5.0 )
#define P_HIGH_PRES 1000.0

// ride height sensor high and low voltages and heights, in millimeters
#define H_LOW_VOLTS 1.0 * ( 1024.0 / 5.0 )
#define H_LOW_HEIGHT 30.0
#define H_HI_VOLTS 4.0 * ( 1024.0 / 5.0 )
#define H_HIGH_HEIGHT 100.0

// compressor settings
#define COMP_TARGETPSI 105   // default pressure to maintain
#define COMP_IDLEPSI 60   // pressure to idle at when inactive
#define COMP_HYS 15        // hysteresis setting (psi tolerance)
#define COMP_MAXPSI 150  // absolute maximum pressure before ESTOP and fault
#define COMP_MAXRUNTIME 5 * 60 // 5 minutes before cooldown
#define COMP_COOLTIME 1 * 60   // 1 minute cooldown before running again

// shock settings
//#define IDLEPSI 60

#endif