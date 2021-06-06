#ifndef AirRide_h
#define AirRide_h


#include <Arduino.h>
#include <EEPROM.h>
#include "airsettings.h"

// LOGGING function
bool debug_log(String msg);

// EEPROM functions
void ee_write_int(uint16_t *addr, int val);
int ee_read_int(uint16_t *addr);
int ee_write_string(uint16_t *addr, String str);
String ee_read_string(uint16_t *addr);

int ee_initialized();
int ee_init(bool clear_data, bool skip_master, bool skip_profiles);
int ee_init(bool clear_data);   // overload with only clear_data
int ee_init();                  // overload with default values
void ee_clear();

// pressure sensor hardware class
class PSensor
{
public:
    int psi();
    void setPin(uint8_t pin);
    void eeprom_store(uint16_t *addr); // pass starting address, returns finishing address
    void eeprom_load(uint16_t *addr);  // pass starting address, returns finishing address

private:
    uint8_t _pin;
    // load calibrations with default values
    int hi_v = P_HI_VOLTS;
    int low_v = P_LOW_VOLTS;
    int hi_p = P_HIGH_PRES;
    int low_p = P_LOW_PRES;
};

// height sensor hardware class
class HSensor
{
public:
    int h_mm();
    void setPin(uint8_t pin);
    void eeprom_store(uint16_t *addr); // pass starting address, returns finishing address
    void eeprom_load(uint16_t *addr);  // pass starting address, returns finishing address

private:
    uint8_t _pin;

    // load calibrations with defafult values
    int hi_v = H_HI_VOLTS;
    int low_v = H_LOW_VOLTS;
    int hi_h = H_HIGH_HEIGHT;
    int low_h = H_LOW_HEIGHT;
};

// pressure valve output class
class PValve
{
public:
    void setState(bool state);
    bool getState();
    void setPin(uint8_t pin);

private:
    uint8_t _pin;
    bool _state = 0;
};

// class for controlling the Compressor Motor
class Motor
{
public:
    void setState(bool state);
    bool getState();
    void setPin(uint8_t pin);

private:
    uint8_t _pin;
    bool _state = 0;
};

// Compressor object control class
// update() MUST be run regularly, to keep PID loop runnung
// defaults setpoint to
class Compressor
{
public:
    Compressor(uint8_t motor_control_pin, uint8_t pres_sensor_pin);
    void setPressure(int psi);         // running pressure for Compressor to maintain
    int getPressure();                 // get current PSI in tank on sensor
    bool isFilling();                  // fetch Motor state
    int getState();                   // Compressor monitor state (active, idle, off)
    void eeprom_store(uint16_t *addr); // pass starting address, returns finishing address
    void eeprom_load(uint16_t *addr);  // pass starting address, returns finishing address
    void setIdlePressure(int psi);     // idle pressure to maintain
    void start();                      // start regulating tank pressure
    void start_idle();                 // start idling
    void stop();                       // stop regulating tank pressure
    void update();                     // main update loop

private:
    Motor _mtr;                                       // tank pump Motor control
    PSensor _pressure;                                // tank pressure sensor
    unsigned long _mtrLastStateChangeTime = millis(); // millis() reading when Motor engaged
    int _tgtPres;                                     // target pressure actively being pursued
    int _runPres = COMP_TARGETPSI;                    // active PSI, default set in airsettings.h
    int _idlePres = COMP_IDLEPSI;                     // idle PSI, default set in airsettings.h
    int _hysteresis = COMP_HYS;                       // hysteresis setting
    bool _active = false;                             // whether the Compressor is actively maintaining its pressure
    bool _cooldown = false;                           // Motor cooldown, if running too long
    bool _idle = false;                               // maintain idle pressure rather than target pressure
    //bool _vent_tank = false;                          // vent tank contents, not yet implemented
};

// Shock assembly object control class
// update() MUST be run regularly, to run PID loop
class Shock
{
public:
    Shock(uint8_t valve_pin, uint8_t height_pin, uint8_t pres_pin);
    void setHeight(int mm);            // set target height in mm; sets mode to maintain height
    void setPressure(int psi);         // set target pressure; sets mode to pressure mode
    int getHeight();                   // get height from sensor
    int getPressure();                 // get pressure from sensor
    void eeprom_store(uint16_t *addr); // pass starting address, returns finishing address
    void eeprom_load(uint16_t *addr);  // pass starting address, returns finishing address
    void update();                     // MUST be run regularly to update the status
private:
    PValve _valve;     // hardware object for valve control
    PSensor _pressure; // hardware object for pressure sensor
    HSensor _height;   // hardware object for height sensor
    int _mm_target;    // store target
    int _psi_target;
    int _mode; // 0=off, 1=psi, 2=height
};

// ******** START OF PROFILE LOGIC **********

// Create a class for holding temporary profile data
class Profile{
public:
    String name = DEFAULT_PROFILE_NAME;
    int mode = DEFAULT_MODE;
    int val = DEFAULT_VAL;
};

uint16_t profileExists(int _profile_index); // returns true if the EEPROM profile is initialized at that index
int createProfile(Profile _new_profile); // creates a new profile, and fills the profile with default values. Returns 0 if successful, 1 if max profiles reached, 2 if EEPROM uninitialized
int loadProfile(Profile *_profile, int _index); // loads profile indexed by '_index'. returns 0 if successful, 1 if unsuccessful
int saveProfile(Profile _profile, int _index); // saves profile _profile, in profile index _index. Returns 0 if successful, 1 if unsuccessful


#endif