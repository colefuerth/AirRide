#line 1 "/home/pi/Air Ride Project I/Arduino Project/AirRide.h"
#ifndef AirRide_h
#define AirRide_h

#include <Arduino.h>
#include <EEPROM.h>
#include "airsettings.h"
#include "debug.h"
#include <stdint.h> // for uint types
#include "ee_functions.h"
#include "hardware_devices.h"

// Compressor object control class
// update() MUST be run regularly, to keep PID loop runnung
// defaults setpoint to
class Compressor
{
private:
    Motor _mtr;                                       // tank pump Motor control
    PSensor _pressure;                                // tank pressure sensor
    unsigned long _mtrLastStateChangeTime = millis(); // millis() reading when Motor engaged
    int _tgtPres;                                     // target pressure actively being pursued
    int _hysteresis = COMP_HYS;                       // hysteresis setting
    bool _active = false;                             // whether the Compressor is actively maintaining its pressure
    bool _cooldown = false;                           // Motor cooldown, if running too long
    bool _idle = false;                               // maintain idle pressure rather than target pressure
    //bool _vent_tank = false;                          // vent tank contents, not yet implemented
public:
    // Compressor(uint8_t motor_control_pin, uint8_t pres_sensor_pin);
    int _runPres = COMP_TARGETPSI;                    // active PSI, default set in airsettings.h
    int _idlePres = COMP_IDLEPSI;                     // idle PSI, default set in airsettings.h
    Compressor(uint8_t, uint8_t);
    void setPressure(float psi);       // running pressure for Compressor to maintain
    float getPressure();               // get current PSI in tank on sensor
    bool isFilling();                  // fetch Motor state
    int getState();                    // Compressor monitor state (active, idle, off)
    void eeprom_store(uint16_t *addr); // pass starting address, returns finishing address
    void eeprom_load(uint16_t *addr);  // pass starting address, returns finishing address
    void setIdlePressure(float psi);   // idle pressure to maintain
    void start();                      // start regulating tank pressure
    void start_idle();                 // start idling
    void stop();                       // stop regulating tank pressure
    void update();                     // main update loop
};

// Shock assembly object control class
// update() MUST be run regularly, to run PID loop
class Shock
{
private:
    PValve _valve;     // hardware object for valve control
    PSensor _pressure; // hardware object for pressure sensor
    HSensor _height;   // hardware object for height sensor
    float _mm_target;  // store target
    float _psi_target;
    int _mode; // 0=off, 1=psi, 2=height
public:
    Shock(uint8_t valve_pin, uint8_t height_pin, uint8_t pres_pin);
    void setHeight(float mm);    // set target height in mm; sets mode to maintain height
    void setPressure(float psi); // set target pressure; sets mode to pressure mode
    float getHeight();           // get height from sensor
    float getPressure();         // get pressure from sensor
    // TODO: no implementation of eeprom functions for shock, as of yet
    void eeprom_store(uint16_t *addr); // pass starting address, returns finishing address
    void eeprom_load(uint16_t *addr);  // pass starting address, returns finishing address
    void update();                     // MUST be run regularly to update the status
};

// ******** START OF PROFILE LOGIC **********

// Create a class for holding temporary profile data
class Profile
{
public:
    String name = DEFAULT_PROFILE_NAME;
    int mode = DEFAULT_MODE;
    float val = (float)DEFAULT_VAL;
};

uint16_t profileExists(int _profile_index);     // returns true if the EEPROM profile is initialized at that index
int createProfile(Profile *_new_profile);       // creates a new profile, and fills the profile with default values. Returns 0 if successful, 1 if max profiles reached, 2 if EEPROM uninitialized
int loadProfile(Profile *_profile, int _index); // loads profile indexed by '_index'. returns 0 if successful, 1 if unsuccessful
int saveProfile(Profile *_profile, int _index); // saves profile _profile, in profile index _index. Returns 0 if successful, 1 if unsuccessful

#endif