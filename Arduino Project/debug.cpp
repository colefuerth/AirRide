#include "debug.h"

// LOGGING function
bool debug_log(const char *msg)
{
    debug_log(String(msg));
}
bool debug_log(String msg)
{
#ifdef SERIALLOGGING
    // if serial debug port is not yet on, turn it on
    /*
    static bool triedSerial = false; // only try turning serial on once
    if (!triedSerial && !DEBUG_PORT) // wait up to 100ms for serial to connect
    {
        triedSerial = true;
        DEBUG_PORT.begin(BAUD_R);
        for (int i = 0; !DEBUG_PORT && i < 10; i++)
            delay(10);
    }*/
    Serial.println(LOG_PREFIX + msg);
    return true;
#endif
    return false;
}
