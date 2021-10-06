#include "AirRide.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "airsettings.h"

// ------------------- SHOCK OBJECT FUNCTIONS -----------------------

// initialize a Shock, with all pin IO values
Shock::Shock(uint8_t valve_pin_out, uint8_t height_pin, uint8_t pres_pin)
{
    this->_valve.setPin(valve_pin_out);
    this->_pressure.setPin(pres_pin);
    this->_height.setPin(height_pin);
}

// set target height Shock should try to maintain
// also sets Shock to maintain a certain height
void Shock::setHeight(float h_mm)
{
    this->_mm_target = h_mm;
    this->_mode = HEIGHTMODE;
}

// set target pressure that Shock should maintain
// sets Shock control mode to maintain psi rather than height
void Shock::setPressure(float psi)
{
    this->_psi_target = psi;
    this->_mode = PSIMODE;
}

// check height on sensor associated with Shock
float Shock::getHeight()
{
    return this->_height.h_mm();
}

// check pressure in psi on sensor associated with Shock
float Shock::getPressure()
{
    return this->_pressure.psi();
}

void Shock::update()
{
    // TODO: use _mode and height/psi values to maintain a target setting
}

// ------------------- COMPRESSOR OBJECT FUNCTIONS -----------------------

// pass a Motor pin and pressure sensor analog pin.
Compressor::Compressor(uint8_t motor_control_pin, uint8_t pres_sensor_pin)
{
    this->_pressure.setPin(pres_sensor_pin);
    this->_mtr.setPin(motor_control_pin);
}

// pressure (PSI) for Compressor to maintain when active
void Compressor::setPressure(float psi)
{
    this->_runPres = psi;
}

// pressure (PSI) for Compressor to maintain when idle
void Compressor::setIdlePressure(float psi)
{
    this->_idlePres = psi;
}

// return pressure in PSI
float Compressor::getPressure()
{
    return this->_pressure.psi();
}

// fetch Motor state
bool Compressor::isFilling()
{
    return this->_mtr.getState();
}

// 0 if off, 1 if active, 2 if idle
int Compressor::getState()
{
    if (!this->_active) // inactive
        return 0;
    else if (!this->_idle) // active and not idle
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
    this->_pressure.eeprom_store(addr);
    // TODO: compressor needs a bleed valve
    // this->_valve.eeprom_store(addr);

    /*
    // spare integers
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    */

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
    // TODO: instead of 28 size, have a define table for each eeprom item size
    addr += _profile_index * 28; // each profile is 28 bytes wide
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
