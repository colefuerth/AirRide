#line 1 "/home/pi/Air Ride Project I/Arduino Project/ee_functions.h"
#ifndef _EE_FUNC_H
#define _EE_FUNC_H

#include <EEPROM.h>
#include "debug.h"
#include "airsettings.h"

// EEPROM functions
void ee_write_int(uint16_t *addr, int val);
int ee_read_int(uint16_t *addr);
void ee_write_float(uint16_t *addr, float val);
float ee_read_float(uint16_t *addr);
int ee_write_string(uint16_t *addr, String str);
String ee_read_string(uint16_t *addr);

int ee_initialized();
int ee_init(bool clear_data, bool skip_master, bool skip_profiles);
int ee_init(bool clear_data); // overload with only clear_data
int ee_init();                // overload with default values
void ee_clear();

#endif