#include <AirRide.h>
#include <Arduino.h>
#include <EEPROM.h>
#include "airsettings.h"

// LOGGING function
bool debug_log(String msg)
{
#ifdef SERIALLOGGING
    // if serial debug port is not yet on, turn it on
    static bool triedSerial = false; // only try turning serial on once
    if (!triedSerial && !DEBUG_PORT) // wait up to 100ms for serial to connect
    {
        triedSerial = true;
        DEBUG_PORT.begin(BAUD_R);
        for (int i = 0; !DEBUG_PORT && i < 10; i++)
            delay(10);
    }
    DEBUG_PORT.println(msg);
    return true;
#endif
    return false;
}

// ----------------------- EEPROM r/w functions ----------------------

// update int at eeprom address in *addr with value val
// int pointed to by *addr is automatically incremented
void ee_write_int(uint16_t *addr, int val)
{
    // if (*addr >= EEPROM.length() - sizeof(int)) // FAULT FOR LATER

    // EEPROM.put(*addr, val); // write val at EEPROM address addr
    uint8_t a, b;
    a = (val >> 8) & 0xFF; // a holds upper 8 bits
    b = val & 0xFF;        // b holds lower 8 bits
    EEPROM.update(*addr, a);
    EEPROM.update(*addr + 1, b);
    if (EEPROM_DEBUG)
        debug_log("writing " + String(val) + " at address " + String(*addr));
    *addr += 2;
}

// returns int value at eeprom address in *addr
// int pointed to by *addr is automatically incremented
int ee_read_int(uint16_t *addr)
{
    // if (addr >= EEPROM.length() - sizeof(int)) // FAULT FOR LATER
    int val;
    // EEPROM.get(*addr, val); // read byte at address
    uint8_t a, b;               // declare temp memory for upper and lower bits
    a = EEPROM.read(*addr);     // grab upper 8 bits in a
    b = EEPROM.read(*addr + 1); // grab lower 8 bits in b
    val = a;                    // store upper 8 bits in val
    val = (val << 8) & 0xFF00;  // shift upper 8 bits, then clear lower 8 bits
    val |= b & 0xFF;            // copy lower 8 bits into val
    if (EEPROM_DEBUG)
        debug_log("reading " + String(val) + " at address " + String(*addr));
    *addr += 2; // increment addr to next unread space
    return val; // return integer value read
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
    if (EEPROM_DEBUG) debug_log("Checking if eeprom initialized.");
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
    if (EEPROM_DEBUG) debug_log("eeprom init check successful.");
    return 0;
}

// NEEDS TESTING
// initialize EEPROM
// affected by settings in airsettings: EE_FORCE_CLEAR_ON_INIT, EE_IGNORE_VERSIONS
// clear_data is an optional passed bit, if true, will clear all stored data in EEPROM when initialization occurs
// skip_master skips master data and immediately moves to profile EEPROM init
// skip_profiles skips profile data when it gets there
// return codes:
// 0: successful init
// 1: Master EEPROM already initialized
// 2: Profile EEPROM already initialized
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
    Serial.print("Clearing EEPROM...");
    for (uint16_t i = 0; i < 1024; EEPROM.update(i++, 0xFF))
        ;
    Serial.println("Done.");
}

// ------------------- PRESSURE SENSOR FUNCTIONS -----------------------

// return PSI of sensor, using currently loaded settings
int PSensor::psi()
{
    // scale output using settings in airsettings.h
    return (int)map((float)analogRead(_pin), low_v, hi_v, low_p, hi_p);
}

void PSensor::setPin(uint8_t pin)
{
    _pin = pin;
    pinMode(pin, INPUT);
}

// pass starting address, returns finishing address
void PSensor::eeprom_store(uint16_t *addr)
{
    if (EEPROM.read(*addr) != 0xFE)
    {
        debug_log("pressure sensor EEPROM uninitialized at location " + String(*addr) + ", initializing.");
        EEPROM.write(*addr, 0xFE); // initializer
    }
    *addr++; // move pointer past initializer

    // bulk of data stored
    ee_write_int(addr, hi_v);  // high analogRead calibration value
    ee_write_int(addr, low_v); // low analogRead calibration value
    ee_write_int(addr, hi_p);  // high psi calibration value
    ee_write_int(addr, low_p); // low psi calibration value

    // spare integers
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);

    EEPROM.update(*addr++, 0xFF); // close with FF byte
}

// pass starting address, returns finishing address
void PSensor::eeprom_load(uint16_t *addr)
{
    if (EEPROM.read(*addr) != 0xFE || EEPROM.read(*addr + (sizeof(int) * 8)) != 0xFF) // load defaults case, if not initialized
    {
        debug_log("pressure sensor EEPROM unitialized or improperly terminated at location " + String(*addr - 1) + ", loading defaults.");
        hi_v = P_HI_VOLTS;
        low_v = P_LOW_VOLTS;
        hi_p = P_HIGH_PRES;
        low_p = P_LOW_PRES;
        *addr += (sizeof(int) * 8) + 2; // need to increment address pointer past data, spares and terminator
        return;                         // once defaults are loaded, dont bother reading rest of EEPROM
    }

    hi_v = ee_read_int(addr);  // high analogRead calibration value
    low_v = ee_read_int(addr); // low analogRead calibration value
    hi_p = ee_read_int(addr);  // high psi calibration value
    low_p = ee_read_int(addr); // low psi calibration value

    addr += (sizeof(int) * 4) + 1; // need to increment address pointer past spares and terminator
}

// ------------------- HEIGHT SENSOR FUNCTIONS -----------------------

// return PSI of sensor, using mapping in airsettings.h
int HSensor::h_mm()
{
    // scale output using settings in airsettings.h
    return (int)map((float)analogRead(_pin), low_v, hi_v, low_h, hi_h);
}

void HSensor::setPin(uint8_t pin)
{
    pinMode(pin, INPUT);
    _pin = pin;
}

// pass starting address, returns finishing address
void HSensor::eeprom_store(uint16_t *addr)
{
    if (EEPROM.read(*addr) != 0xFE)
    {
        debug_log("height sensor EEPROM uninitialized at location " + String(*addr) + ", initializing.");
        EEPROM.write(*addr, 0xFE); // initializer
    }
    *addr++; // move pointer past initializer

    // bulk of data stored
    ee_write_int(addr, hi_v);  // high analogRead calibration value
    ee_write_int(addr, low_v); // low analogRead calibration value
    ee_write_int(addr, hi_h);  // high psi calibration value
    ee_write_int(addr, low_h); // low psi calibration value

    // spare integers
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);

    EEPROM.update(*addr++, 0xFF); // close with FF byte
}

// pass starting address, returns finishing address
void HSensor::eeprom_load(uint16_t *addr)
{
    if (EEPROM.read(*addr) != 0xFE || EEPROM.read(*addr + (sizeof(int) * 8)) != 0xFF) // load defaults case, if not initialized
    {
        debug_log("height sensor EEPROM unitialized or improperly terminated at location " + String(*addr - 1) + ", loading defaults.");
        hi_v = H_HI_VOLTS;
        low_v = H_LOW_VOLTS;
        hi_h = H_HIGH_HEIGHT;
        low_h = H_LOW_HEIGHT;
        *addr += (sizeof(int) * 8) + 2; // need to increment address pointer past initializer, data, spares and terminator
        return;                         // once defaults are loaded, dont bother reading rest of EEPROM
    }
    *addr++; // move past initialization check

    hi_v = ee_read_int(addr);  // high analogRead calibration value
    low_v = ee_read_int(addr); // low analogRead calibration value
    hi_h = ee_read_int(addr);  // high psi calibration value
    low_h = ee_read_int(addr); // low psi calibration value

    addr += (sizeof(int) * 4) + 1; // need to increment address pointer past spares and terminator
}

// ------------------- PRESSURE VALVE FUNCTIONS -----------------------

// state 1 = open, 0 = closed
void PValve::setState(bool state)
{
    _state = state;
    digitalWrite(_pin, !_state); // valves run by relays, so active low
}

bool PValve::getState()
{
    return _state;
}

void PValve::setPin(uint8_t pin)
{
    pinMode(pin, OUTPUT);
    _pin = pin;
}

// ------------------- COMPRESSOR MOTOR FUNCTIONS -----------------------

// state 1 = powered, 0 = idle
void Motor::setState(bool state)
{
    _state = state;
    digitalWrite(_pin, !_state); // valves run by relays, so active low
}

// state 1 = powered, 0 = idle
bool Motor::getState()
{
    return _state;
}

// set Motor output pin
void Motor::setPin(uint8_t pin)
{
    _pin = pin;
    pinMode(pin, OUTPUT);
}

// ------------------- SHOCK OBJECT FUNCTIONS -----------------------

// initialize a Shock, with all pin IO values
Shock::Shock(uint8_t valve_pin_out, uint8_t height_pin, uint8_t pres_pin)
{
    _valve.setPin(valve_pin_out);
    _pressure.setPin(pres_pin);
    _height.setPin(height_pin);
}

// set target height Shock should try to maintain
// also sets Shock to maintain a certain height
void Shock::setHeight(int h_mm)
{
    _mm_target = h_mm;
    _mode = HEIGHTMODE;
}

// set target pressure that Shock should maintain
// sets Shock control mode to maintain psi rather than height
void Shock::setPressure(int psi)
{
    _psi_target = psi;
    _mode = PSIMODE;
}

// check height on sensor associated with Shock
int Shock::getHeight()
{
    return _height.h_mm();
}

// check pressure in psi on sensor associated with Shock
int Shock::getPressure()
{
    return _pressure.psi();
}

void Shock::update()
{
    // TODO: use _mode and height/psi values to maintain a target setting
}

// ------------------- COMPRESSOR OBJECT FUNCTIONS -----------------------

// pass a Motor pin and pressure sensor analog pin.
Compressor::Compressor(uint8_t motor_control_pin, uint8_t pres_sensor_pin)
{
    _pressure.setPin(pres_sensor_pin);
    _mtr.setPin(motor_control_pin);
}

// pressure (PSI) for Compressor to maintain when active
void Compressor::setPressure(int psi)
{
    _runPres = psi;
}

// pressure (PSI) for Compressor to maintain when idle
void Compressor::setIdlePressure(int psi)
{
    _idlePres = psi;
}

// return pressure in PSI
int Compressor::getPressure()
{
    return _pressure.psi();
}

// fetch Motor state
bool Compressor::isFilling()
{
    return _mtr.getState();
}

// 0 if off, 1 if active, 2 if idle
int Compressor::getState()
{
    if (!_active) // inactive
        return 0;
    else if (!_idle) // active and not idle
        return 1;
    else // active and idle
        return 2;
}

// store EEPROM state of Compressor
void Compressor::eeprom_store(uint16_t *addr)
{
    // start by checking initializer
    if (EEPROM.read(*addr) != 0xFE)
    {
        debug_log("Compressor EEPROM uninitialized at location " + String(*addr) + ", initializing.");
        EEPROM.write(*addr, 0xFE); // initializer
    }
    *addr++; // move pointer past initializer

    // bulk of data stored
    // TODO: ADD DATA TO BE STORED BETWEEN POWER CYCLES

    // spare integers
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);

    EEPROM.update(*addr++, 0xFF); // close with FF byte
}

// start regulating tank pressure at running pressure
void Compressor::start()
{
    _active = true;
    _idle = false;
}

// set Compressor to idle mode, maintaining stored idle PSI
void Compressor::start_idle()
{
    _active = true;
    _idle = true;
}

// stop regulating tank pressure
void Compressor::stop()
{
    _active = false;
}

// main update loop
void Compressor::update()
{
    // set target PSI based on Compressor state
    if (_idle)
        _tgtPres = _idlePres;
    else
        _tgtPres = _runPres;

    // if tank is active, monitor pressure and run Motor to maintain target
    if (_active)
    {
        // start Motor case, requirements: pres too low, not in cooldown, Motor is off
        if (_mtr.getState() == OFF && _pressure.psi() < (_tgtPres - _hysteresis) && !_cooldown)
        {
            _mtr.setState(ON);
            _mtrLastStateChangeTime = millis();
        }
        // stop Motor case, requirements: Motor is on and pres at spec or max time reached
        if (_mtr.getState() == ON)
        {
            if (_pressure.psi() >= (_tgtPres + _hysteresis) || _pressure.psi() >= COMP_MAXPSI)
            {
                _mtr.setState(OFF);
                _mtrLastStateChangeTime = millis();
            }
            if (millis() - _mtrLastStateChangeTime >= (long)COMP_COOLTIME * 1000)
            {
                _mtr.setState(OFF);
                _mtrLastStateChangeTime = millis();
                _cooldown = true;
            }
        }
    }
    // if inactive, then ensure Motor is off
    else
    {
        if (_mtr.getState() == ON)
            _mtr.setState(OFF);
        _mtrLastStateChangeTime = millis();
    }

    // if cooldown was engaged, then disengage it when the timer runs out.
    // Cooldown must finish once started before Motor starts again, regardless of machine state.
    if (_cooldown && millis() - _mtrLastStateChangeTime >= (long)COMP_COOLTIME * 1000)
        _cooldown = false;
}

// ******** EEPROM PROFILE FUNCTIONS **********

// looks up EEPROM address of profile
// returns 0 if not found, or the address of the profile in EEPROM if exists
uint16_t profileExists(int _profile_index)
{
    // start by pointing at integer value for number of profiles initialized
    uint16_t addr = EE_PROF_START + 3;
    // check for uninitialized EEPROM or not enough profiles
    if (ee_initialized() || ee_read_int(&addr) - 1 < _profile_index)
        return 0;
    // move pointer to relevant pointer
    // at this point, address pointer is pointing at first address of first profile.
    addr += _profile_index * 26; // each profile is 26 bytes wide
    // return address pointed to
    return addr;
}

// creates a new profile, and fills the profile with default values.
// 0: profile creation success
// 1: max profiles reached
// 2: EEPROM uninitialized
int createProfile(Profile *_new_profile)
{
    if (PROFILE_DEBUG)
        debug_log("start of createprofile call");
    if (ee_initialized()) // check if eeprom is initialized
    {
        debug_log("Unable to create new profile; a problem was found when checking EEPROM validity.");
        return 2;
    }
    uint16_t addr = EE_PROF_START + 3; // set address to location of profile counter
    int numprofiles = ee_read_int(&addr);
    if (numprofiles >= 5) // check if too many profiles exist
    {
        debug_log("Unable to create new profile, maximum number of profiles reached!");
        return 1;
    }
    addr -= 2;                            // reset address pointer to profile counter in EEPROM
    ee_write_int(&addr, numprofiles + 1); // update profile counter in eeprom
    addr += numprofiles * 26;             // increment to the starting address of new profile
    if (PROFILE_DEBUG)
        debug_log("creating profile " + String(numprofiles) + " at address " + String(addr));
    EEPROM.update(addr++, 0xFE);                // initializer
    ee_write_string(&addr, _new_profile->name); // store profile name
    ee_write_int(&addr, _new_profile->mode);    // store mode
    ee_write_int(&addr, _new_profile->val);     // store value
    EEPROM.update(addr++, 0xFF);
    if (PROFILE_DEBUG)
        debug_log("done creating profile.");

    return 0; // success condition
}

// loads profile indexed by '_index'.
// index ranges from 0-4
// 0: successful load
// 1: Master EEPROM problem
// 2: Profile not yet created
// 3: Profile data corrupted
int loadProfile(Profile *_profile, int _index)
{
    if (PROFILE_DEBUG)
        debug_log("loading profile " + String(_index));
    // check if eeprom is initialized
    if (int err = ee_initialized())
    {
        debug_log("EEPROM error " + String(err));
        debug_log("Unable to load profile; a problem was found when checking EEPROM validity.");
        return 1;
    }
    // address pointer is 0 if not exist
    uint16_t addr = profileExists(_index);
    if (!addr || _index > 4)
    {
        debug_log("Unable to load profile" + String(_index) + "; specified profile does not exist");
        return 2; // unsuccessful condition
    }
    if (EEPROM.read(addr++) != 0xFE)
    {
        debug_log("Profile data corrupted. Index: " + String(_index));
        return 3;
    }

    // load body data, this method is verified
    _profile->name = ee_read_string(&addr);
    _profile->mode = ee_read_int(&addr);
    _profile->val = ee_read_int(&addr);

    // check for correct termination
    // NOTE: this is a WARNING, and will still load the data regardless
    if (EEPROM.read(addr++) != 0xFF)
    {
        debug_log("Warning: Profile data corrupted; improper termination. Index: " + String(_index));
        return 3;
    }
    if (PROFILE_DEBUG)
        debug_log("done loading profile " + String(_index));
    return 0; // success condition
}

// im pretty sure this stores strings improperly
// saves profile _profile, in profile index _index.
// index ranges from 0-4
// 0: successful read
// 1: EEPROM problem
// 2: Profile slot uninitialized
// 3: Profile data corrupted
int saveProfile(Profile *_profile, int _index)
{
    if (PROFILE_DEBUG)
        debug_log("saving profile " + String(_index));
    // check if eeprom is initialized
    if (ee_initialized())
    {
        debug_log("Unable to save profile; a problem was found when checking EEPROM validity.");
        return 1;
    }
    // address pointer is 0 if not exist
    uint16_t addr = EE_PROF_START + 3; // point address at number of profiles initialized
    // index is 0-4, number initialized is 1-5
    // so, if 1 profile is initialized but index is 1, then fail
    if (_index >= ee_read_int(&addr) || _index > 4)
    {
        debug_log("Unable to save profile; specified profile does not exist");
        return 2; // unsuccessful condition
    }

    // increment address pointer to desired start of profile
    int profile_size = 22 + (2 * sizeof(int)); // 20 for string, 2 for init and termination, and 2 ints
    addr += _index * profile_size;             // move to initializer

    // all profiles begin with FE when initialized
    if (EEPROM.read(addr++) != 0xFE)
    {
        debug_log("Profile data corrupted. Index: " + String(_index));
        return 3;
    }

    // load body data, this method is verified
    ee_write_string(&addr, _profile->name);
    ee_write_int(&addr, _profile->mode);
    ee_write_int(&addr, _profile->val);

    // check for correct termination
    // NOTE: this is a WARNING, and will still load the data regardless
    if (EEPROM.read(addr++) != 0xFF)
    {
        debug_log("Warning: Profile data corrupted; improper termination. Index: " + String(_index));
        return 3;
    }
    if (PROFILE_DEBUG)
        debug_log("done saving profile " + String(_index));
    return 0; // success condition
}
