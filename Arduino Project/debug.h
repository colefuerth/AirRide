#ifndef _DEBUG_H
#define _DEBUG_H

#include <Arduino.h>
#include "airsettings.h"

// LOGGING function
#define LOG_PREFIX "log "
bool debug_log(const char *);
bool debug_log(String);

#endif