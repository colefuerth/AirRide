#line 1 "/home/pi/Air Ride Project I/Arduino Project/hardware_devices.h"
#ifndef _HARDWARE_DEVICES_H
#define _HARDWARE_DEVICES_H

#include <Arduino.h>
#include <EEPROM.h>
#include "debug.h"
#include "airsettings.h"
#include "ee_functions.h"

// Supporting class for pressure sensor hardware
class PSensor
{
private:
    uint8_t _pin;
    // load calibrations with default values
    float hi_v = P_HI_VOLTS;
    float low_v = P_LOW_VOLTS;
    float hi_p = P_HIGH_PRES;
    float low_p = P_LOW_PRES;
public:
    float psi();
    void setPin(uint8_t pin);
    void eeprom_store(uint16_t *addr); // pass starting address, returns finishing address
    void eeprom_load(uint16_t *addr);  // pass starting address, returns finishing address
};

// Supporting class for height sensor hardware
class HSensor
{
private:
    uint8_t _pin;
    // load calibrations with defafult values
    float hi_v = H_HI_VOLTS;
    float low_v = H_LOW_VOLTS;
    float hi_h = H_HIGH_HEIGHT;
    float low_h = H_LOW_HEIGHT;
public:
    float h_mm();
    void setPin(uint8_t pin);
    void eeprom_store(uint16_t *addr); // pass starting address, returns finishing address
    void eeprom_load(uint16_t *addr);  // pass starting address, returns finishing address
};

// Supporting class for pressure valve output
class PValve
{
private:
    uint8_t _pin;
    bool _state = 0;
public:
    void setState(bool state);
    bool getState();
    void setPin(uint8_t pin);
};

// Supporting class for controlling the Compressor Motor
class Motor
{
private:
    uint8_t _pin;
    bool _state = 0;
public:
    void setState(bool state);
    bool getState();
    void setPin(uint8_t pin);
};

#endif