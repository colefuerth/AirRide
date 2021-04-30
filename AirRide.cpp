#include <AirRide.h>
#include <Arduino.h>
#include <EEPROM.h>
#include "airsettings.h"

// LOGGING function
bool debug_log(String msg)
{
#ifdef SERIALLOGGING
    // if serial debug port is not yet on, turn it on
    static bool triedSerial = false;    // only try turning serial on once
    if (!triedSerial && !DEBUG_PORT) // wait up to 100ms for serial to connect
    {
        triedSerial = true;
        DEBUG_PORT.begin(BAUD_R);
        for (int i=0; !DEBUG_PORT && i < 10; i++)
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

    EEPROM.put(*addr, val); // write val at EEPROM address addr
    *addr+= sizeof(int);
}

// returns int value at eeprom address in *addr
// int pointed to by *addr is automatically incremented
int ee_read_int(uint16_t *addr)
{
    // if (addr >= EEPROM.length() - sizeof(int)) // FAULT FOR LATER
    int val;
    EEPROM.get(*addr, val); // read byte at address
    *addr += sizeof(int);   // increment addr to next unread space
    return val;             // return integer value read
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
    EEPROM.put(*addr, str);
    *addr += 20;
    return 0;
}

// returns string found at address *addr, then increments address by 20
String ee_read_string(uint16_t *addr)
{
    String _str;
    EEPROM.get(*addr, _str);
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
    uint8_t temp;

    // start by checking for master 1 at address 0
    // stating EEPROM has been initialized in the past
    temp = EEPROM.read(0);
    if (temp != 1)
        return 1; // master EEPROM uninitialized condition

    // check EEPROM version number
    temp = EEPROM.read(1);
    if (temp != EE_VER)
        return 2; // EEPROM version mismatch warning
    
    // check data portion is initialized
    temp = EEPROM.read(EE_DATA_START);
    if (temp != 0xFE)
        return 3;   // EEPROM data partition uninitialized

    // check profiles portion is initialized
    temp = EEPROM.read(EE_PROF_START);
    if (temp != 0xFE)
        return 4;   // EEPROM data partition uninitialized

    return 0;
}


// ------------------- PRESSURE SENSOR FUNCTIONS -----------------------

// return PSI of sensor, using currently loaded settings
int psensor::psi()
{
    // scale output using settings in airsettings.h
    return (int)map((float)analogRead(_pin), low_v, hi_v, low_p, hi_p);
}

void psensor::setPin(uint8_t pin)
{
    _pin = pin;
    pinMode(pin, INPUT);
}

// pass starting address, returns finishing address
void psensor::eeprom_store(uint16_t *addr)
{
    if (EEPROM.read(*addr) != 0xFE)
    {
        debug_log("pressure sensor EEPROM uninitialized at location " + String(*addr) + ", initializing.");
        EEPROM.write(*addr, 0xFE); // initializer
    }
    *addr++;    // move pointer past initializer

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
void psensor::eeprom_load(uint16_t *addr)
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
int hsensor::h_mm()
{
    // scale output using settings in airsettings.h
    return (int)map((float)analogRead(_pin), low_v, hi_v, low_h, hi_h);
}

void hsensor::setPin(uint8_t pin)
{
    pinMode(pin, INPUT);
    _pin = pin;
}

// pass starting address, returns finishing address
void hsensor::eeprom_store(uint16_t *addr)
{
    if (EEPROM.read(*addr) != 0xFE)
    {
        debug_log("height sensor EEPROM uninitialized at location " + String(*addr) + ", initializing.");
        EEPROM.write(*addr, 0xFE); // initializer
    }
    *addr++;    // move pointer past initializer

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
void hsensor::eeprom_load(uint16_t *addr)
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
void pvalve::setState(bool state)
{
    _state = state;
    digitalWrite(_pin, !_state); // valves run by relays, so active low
}

bool pvalve::getState()
{
    return _state;
}

void pvalve::setPin(uint8_t pin)
{
    pinMode(pin, OUTPUT);
    _pin = pin;
}

// ------------------- COMPRESSOR MOTOR FUNCTIONS -----------------------

// state 1 = powered, 0 = idle
void motor::setState(bool state)
{
    _state = state;
    digitalWrite(_pin, !_state); // valves run by relays, so active low
}

// state 1 = powered, 0 = idle
bool motor::getState()
{
    return _state;
}

// set motor output pin
void motor::setPin(uint8_t pin)
{
    _pin = pin;
    pinMode(pin, OUTPUT);
}

// ------------------- SHOCK OBJECT FUNCTIONS -----------------------

// initialize a shock, with all pin IO values
shock::shock(uint8_t valve_pin_out, uint8_t height_pin, uint8_t pres_pin)
{
    _valve.setPin(valve_pin_out);
    _pressure.setPin(pres_pin);
    _height.setPin(height_pin);
}

// set target height shock should try to maintain
// also sets shock to maintain a certain height
void shock::setHeight(int h_mm)
{
    _mm_target = h_mm;
    _mode = HEIGHTMODE;
}

// set target pressure that shock should maintain
// sets shock control mode to maintain psi rather than height
void shock::setPressure(int psi)
{
    _psi_target = psi;
    _mode = PSIMODE;
}

// check height on sensor associated with shock
int shock::getHeight()
{
    return _height.h_mm();
}

// check pressure in psi on sensor associated with shock
int shock::getPressure()
{
    return _pressure.psi();
}

void shock::update()
{
    // TODO: use _mode and height/psi values to maintain a target setting
}

// ------------------- COMPRESSOR OBJECT FUNCTIONS -----------------------

// pass a motor pin and pressure sensor analog pin.
compressor::compressor(uint8_t motor_control_pin, uint8_t pres_sensor_pin)
{
    _pressure.setPin(pres_sensor_pin);
    _mtr.setPin(motor_control_pin);
}

// pressure (PSI) for compressor to maintain when active
void compressor::setPressure(int psi)
{
    _runPres = psi;
}

// pressure (PSI) for compressor to maintain when idle
void compressor::setIdlePressure(int psi)
{
    _idlePres = psi;
}

// return pressure in PSI
int compressor::getPressure()
{
    return _pressure.psi();
}

// fetch motor state
bool compressor::isFilling()
{
    return _mtr.getState();
}

// 0 if off, 1 if active, 2 if idle
int compressor::get_state()
{
    if (!_active) // inactive
        return 0;
    else if (!_idle) // active and not idle
        return 1;
    else // active and idle
        return 2;
}

// store EEPROM state of Compressor
void compressor::eeprom_store(uint16_t *addr)
{
    // start by checking initializer
    if (EEPROM.read(*addr) != 0xFE)
    {
        debug_log("Compressor EEPROM uninitialized at location " + String(*addr) + ", initializing.");
        EEPROM.write(*addr, 0xFE); // initializer
    }
    *addr++;    // move pointer past initializer

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
void compressor::start()
{
    _active = true;
    _idle = false;
}

// set compressor to idle mode, maintaining stored idle PSI
void compressor::start_idle()
{
    _active = true;
    _idle = true;
}

// stop regulating tank pressure
void compressor::stop()
{
    _active = false;
}

// main update loop
void compressor::update()
{
    // set target PSI based on compressor state
    if (_idle)
        _tgtPres = _idlePres;
    else
        _tgtPres = _runPres;

    // if tank is active, monitor pressure and run motor to maintain target
    if (_active)
    {
        // start motor case, requirements: pres too low, not in cooldown, motor is off
        if (_mtr.getState() == OFF && _pressure.psi() < (_tgtPres - _hysteresis) && !_cooldown)
        {
            _mtr.setState(ON);
            _mtrLastStateChangeTime = millis();
        }
        // stop motor case, requirements: motor is on and pres at spec or max time reached
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
    // if inactive, then ensure motor is off
    else
    {
        if (_mtr.getState() == ON)
            _mtr.setState(OFF);
        _mtrLastStateChangeTime = millis();
    }

    // if cooldown was engaged, then disengage it when the timer runs out.
    // Cooldown must finish once started before motor starts again, regardless of machine state.
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
    if (!ee_initialized() || ee_read_int(&addr) - 1 < _profile_index)
        return 0;
    // move pointer to relevant pointer
    addr += _profile_index * 26; // each profile is 26 bytes wide
    // return address pointed to
    return addr;
}

// creates a new profile, and fills the profile with default values. 
// Returns 0 if successful, 1 if max profiles reached, 2 if EEPROM uninitialized
int createProfile(Profile _new_profile) 
{
    uint16_t addr = EE_PROF_START + 3; // set address to location of profile counter
    int numprofiles = ee_read_int(&addr);
    if (ee_initialized()) // check if eeprom is initialized
    {
        debug_log("Unable to create new profile; a problem was found when checking EEPROM validity.");
        return 2;
    }
    if (numprofiles >= 5)   // check if too many profiles exist
    {
        debug_log("Unable to create new profile, maximum number of profiles reached!");
        return 1;
    }
    addr -= 2; // reset address pointer to profile counter in EEPROM
    ee_write_int(&addr, numprofiles + 1);

    addr += numprofiles * 26; // increment to the starting address of new profile
    EEPROM.update(addr++, 0xFE); // initializer
    ee_write_string(&addr, _new_profile.name); // store profile name
    ee_write_int(&addr, _new_profile.mode); // store mode
    ee_write_int(&addr, _new_profile.val); // store value
    EEPROM.update(addr++, 0xFF);

    return 0; // success condition
}

// loads profile indexed by '_index'. 
// 0: successful load
// 1: Master EEPROM problem
// 2: Profile not yet created
// 3: Profile data corrupted
int loadProfile(Profile *_profile, int _index) 
{
    // check if eeprom is initialized
    if (ee_initialized()) 
    {
        debug_log("Unable to load profile; a problem was found when checking EEPROM validity.");
        return 1;
    }
    // address pointer is 0 if not exist
    uint16_t addr = profileExists(_index);
    if (!addr)
    {
        debug_log("Unable to load profile; specified profile does not exist");
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

    return 0; // success condition
}

// UNFINISHED
// saves profile _profile, in profile index _index. 
// Returns 0 if successful, 1 if unsuccessful
int saveProfile(Profile _profile, int _index) 
{
    return 0; // success condition
}
