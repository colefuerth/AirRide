#line 1 "/home/pi/Air Ride Project I/Arduino Project/ee_functions.cpp"
#include "ee_functions.h"

// ----------------------- EEPROM r/w functions ----------------------

// update int at eeprom address in *addr with value val
// int pointed to by *addr is automatically incremented
void ee_write_int(uint16_t *addr, int val)
{
    // if (*addr >= (EEPROM.length() - sizeof(int))) // TODO: FAULT FOR LATER
    // EEPROM.put(*addr, val); // write val at EEPROM address addr
    uint8_t size = sizeof(int);
    unsigned char *temp = reinterpret_cast<unsigned char *>(&val); // pointer cast val to long; long is same size as float
    for (int i = 0; i < size; i++)
    {
        EEPROM.update(*addr + i, *(temp + i));
    }
    if (EEPROM_DEBUG)
    {
        debug_log("writing " + String(val) + " at address " + String(*addr));
    }
    *addr += size;
}

// returns int value at eeprom address in *addr
// int pointed to by *addr is automatically incremented
int ee_read_int(uint16_t *addr)
{
    // if (*addr >= EEPROM.length() - sizeof(int)) // FAULT FOR LATER
    // EEPROM.get(*addr); // write val at EEPROM address addr
    uint8_t fsize = sizeof(int);
    unsigned char temp[fsize];
    for (int i = 0; i < fsize; i++)
    {
        temp[i] = EEPROM.read(*addr + i);
    }
    int *y = reinterpret_cast<int *>(&temp);
    if (EEPROM_DEBUG)
    {
        debug_log("read float " + String(*y) + " at address " + String(*addr));
    }
    *addr += fsize;
    return *y;
}

// update int at eeprom address in *addr with value val
// int pointed to by *addr is automatically incremented
void ee_write_float(uint16_t *addr, float val)
{
    // if (*addr >= (EEPROM.length() - sizeof(int))) // TODO: FAULT FOR LATER

    // EEPROM.put(*addr, val); // write val at EEPROM address addr
    uint8_t fsize = sizeof(float);
    unsigned char *temp = reinterpret_cast<unsigned char *>(&val); // pointer cast val to long; long is same size as float
    for (int i = 0; i < fsize; i++)
    {
        EEPROM.update(*addr + i, *(temp + i));
    }
    if (EEPROM_DEBUG)
    {
        debug_log("writing " + String(val) + " at address " + String(*addr));
    }
    *addr += fsize;
}

// returns float value at eeprom address in *addr
// int pointed to by *addr is automatically incremented
// does bitwise copy of float into destination
float ee_read_float(uint16_t *addr)
{
    // if (*addr >= EEPROM.length() - sizeof(int)) // FAULT FOR LATER
    // EEPROM.get(*addr); // write val at EEPROM address addr
    uint8_t fsize = sizeof(float);
    unsigned char temp[fsize];
    for (int i = 0; i < fsize; i++)
    {
        temp[i] = EEPROM.read(*addr + i);
    }
    float *y = reinterpret_cast<float *>(&temp);
    if (EEPROM_DEBUG)
    {
        debug_log("read float " + String(*y) + " at address " + String(*addr));
    }
    *addr += fsize;
    return *y;
}

// write string at address, increment address by 20
// requires: max string length is 19
// returns 0 if successful
// returns 1 if string too long
int ee_write_string(uint16_t *addr, String str)
{
    if (str.length() >= 19) // string too long to store
    {
        debug_log("String too long to store");
        return 1; // fail condition
    }
    if (EEPROM_DEBUG)
        debug_log("writing string '" + str + "' at address " + String(*addr));
    // EEPROM.put(*addr, str);
    for (uint16_t i = 0; i < 20; i++)
    {
        if (i < str.length())
            EEPROM.update(*addr + i, str.charAt(i));
        else
            EEPROM.update(*addr + i, 0);
    }
    *addr += 20; // ensure address is updated
    return 0;
}

// returns string found at address *addr, then increments address by 20
String ee_read_string(uint16_t *addr)
{
    String _str = "";
    // EEPROM.get(*addr, _str); // might not work for this purpose
    uint16_t _len = 0;
    char c;
    while ((c = EEPROM.read(*addr + _len)) != 0 && _len < 20)
    {
        _str = _str + String(c); // store character that was read
        _len++;
    }
    if (EEPROM_DEBUG)
        debug_log("reading string \"" + _str + "\" at address " + String(*addr));
    *addr += 20;
    return _str;
}

// NEEDS LOGGING MESSAGES ADDED
// check for EEPROM correct initialization
// 0: correctly initialized
// 1: EEPROM version mismatch
// 2: data partition error
// 3: profile storage partition error
int ee_initialized()
{
    if (EEPROM_DEBUG)
        debug_log("Checking if eeprom initialized.");
    uint8_t temp;

    // start by checking for master 1 at address 0
    // stating EEPROM has been initialized in the past
    temp = EEPROM.read(0);
    if (temp != 1)
    {
        debug_log("EEPROM not correctly initialized: EEPROM not yet initialized.");
        return 1; // master EEPROM uninitialized condition
    }

#ifndef EE_IGNORE_VERSIONS
    // check EEPROM version number
    temp = EEPROM.read(1);
    if (temp != EE_VER)
    {
        debug_log("EEPROM not correctly initialized: EEPROM version mismatch.");
        return 2; // EEPROM version mismatch warning
    }
#endif

    // check data portion is initialized
    temp = EEPROM.read(EE_DATA_START);
    if (temp != 0xFE)
    {
        debug_log("EEPROM not correctly initialized: EEPROM embedded data portion not initialized.");
        return 3; // EEPROM data partition uninitialized
    }

    // check profiles portion is initialized
    temp = EEPROM.read(EE_PROF_START);
    if (temp != 0xFE)
    {
        debug_log("EEPROM not correctly initialized: EEPROM profiles data portion not initialized.");
        return 4; // EEPROM data partition uninitialized
    }
    if (EEPROM_DEBUG)
        debug_log("eeprom init check successful.");
    return 0;
}
/**
 * NEEDS TESTING
 * initialize EEPROM
 * affected by settings in airsettings: EE_FORCE_CLEAR_ON_INIT, EE_IGNORE_VERSIONS
 * clear_data is an optional passed bit, if true, will clear all stored data in EEPROM when initialization occurs
 * skip_master skips master data and immediately moves to profile EEPROM init
 * skip_profiles skips profile data when it gets there
 * return codes:
 * 0: successful init
 * 1: Master EEPROM already initialized
 * 2: Profile EEPROM already initialized
 */
int ee_init(bool clear_data, bool skip_master, bool skip_profiles)
{

// if force clear debug setting is on, then force clear_data to true
#ifdef EE_FORCE_CLEAR_ON_INIT
    clear_data = true;
#endif

    if (EEPROM_DEBUG)
    {
        debug_log("initializing eeprom with flags:");
        debug_log("\tclear_data: " + String(clear_data));
        debug_log("\tskip_master: " + String(skip_master));
        debug_log("\tskip_profiles: " + String(skip_profiles));
    }

    uint16_t addr = 0;

    // ******* Master Data EEPROM *********
    if (!skip_master)
    {
        // only check for preinitialized EEPROM if not forcing clear
        bool isInitialized = EEPROM.read(addr) == 1;
        if (!isInitialized)
        {
            debug_log("Beginning first initialization of EEPROM.");
            EEPROM.write(addr, 1);
        }
        addr++; // increment address to version

        // check version number
        bool ee_mismatch = false;
#ifndef EE_IGNORE_VERSIONS
        if (isInitialized && EEPROM.read(addr) != EE_VER)
        {
            debug_log("EEPROM version mismatch. Updating version to " + String(EE_VER));
            EEPROM.update(addr, EE_VER);
            ee_mismatch = true; // set flag for EEPROM version mismatch
            clear_data = true;  // clear data on version mismatch
        }
        else
            EEPROM.update(addr, EE_VER);
#endif
        addr = 20;

        // check for master data block initializer
#ifndef EE_FORCE_CLEAR_ON_INIT
        if (EEPROM.read(addr) == 0xFE)
        {
            // if already initialized, issue warning and skip to profiles
            debug_log("Could not initialize master EEPROM; already initialized.");
            // change skip_master to true and run function again
            // cursed, yes, but it was the simplest way to skip to profile checks
            skip_master = true;
            ee_init(skip_master, skip_profiles, clear_data);
            // return code for already initialized is 2
            return 2;
        }
#endif

        // ensure initializer bit for master data block is set
        EEPROM.update(addr++, 0xFE);

        // if clear_data bit is set, then clear all bulk data
        // clear_data is forced on if EE_FORCE_CLEAR_ON_INIT is defined
        // loop clears data, starting at beginning of bulk and ending at terminator FF FF FF
        if (clear_data)
        {
            uint8_t ff_conc = 0; // number of consecutive FF bits
            while (addr < EE_PROF_START && ff_conc < 3)
            {
                // increment counter if addr is FF
                if (EEPROM.read(addr) == 0xFF)
                    ff_conc++;
                // if addr is not FF, reset counter to 0 and set location to FF
                else
                {
                    ff_conc = 0;
                    EEPROM.write(addr, 0xFF);
                }
                addr++;
            }
        }

    } // end of master init

    // ******* PROFILES EEPROM ***********
    // finished, as far as I can tell
    if (!skip_profiles)
    {
        addr = EE_PROF_START;
#ifndef EE_FORCE_CLEAR_ON_INIT
        if (!clear_data && EEPROM.read(addr) == 0xFE)
        {
            debug_log("Could not initialize profile EEPROM; already initialized.");
            return 2;
        }
#endif

        EEPROM.update(addr++, 0xFE); // store initialized bit
        if (clear_data)              // this runs regardless if force clear setting is on
        {
            ee_write_int(&addr, DEFAULT_PROFILE); // default profile is defined in airsettings.h
            ee_write_int(&addr, 0);               // 0 profiles are stored on initialization
            // loop through all profile data slots, clearing all of them, plus the master terminator at the end
            const uint16_t lim = addr + 26 * 5; // predefined limit; saves time when doing loop
            while (addr <= lim)                 // '<=' to cover all profile data slots, PLUS the terminator at the very end
                EEPROM.update(addr++, 0xFF);
        }
    } // end of profiles init
    if (EEPROM_DEBUG)
        debug_log("successfully finished eeprom init.");
    return 0;
} // end of eeprom_init

// set clear data bool, dont skip any sections
int ee_init(bool clear_data)
{
    return ee_init(clear_data, false, false);
}

// default call clears all data and does not skip any sections
int ee_init()
{
    return ee_init(true);
}

void ee_clear()
{
    debug_log("Clearing EEPROM...");
    for (uint16_t i = 0; i < 1024; EEPROM.update(i++, 0xFF))
        ;
    debug_log("Done.");
}
