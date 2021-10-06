#line 1 "/home/pi/Air Ride Project I/Arduino Project/hardware_devices.cpp"
#include "hardware_devices.h"

// ------------------- PRESSURE SENSOR FUNCTIONS -----------------------

// return PSI of sensor, using currently loaded settings
float PSensor::psi()
{
    // scale output using settings in airsettings.h
    return map((float)analogRead(_pin), this->low_v, this->hi_v, this->low_p, this->hi_p);
}

void PSensor::setPin(uint8_t pin)
{
    if (this->_pin != pin)
    {
        this->_pin = pin; // store pin for later
        pinMode(pin, INPUT);
    }
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
    ee_write_float(addr, this->hi_v);  // high analogRead calibration value
    ee_write_float(addr, this->low_v); // low analogRead calibration value
    ee_write_float(addr, this->hi_p);  // high psi calibration value
    ee_write_float(addr, this->low_p); // low psi calibration value

    /*
    // spare integers
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    */

    EEPROM.update(*addr++, 0xFF); // close with FF byte
    return;
}

// pass starting address, returns finishing address
void PSensor::eeprom_load(uint16_t *addr)
{
    if (EEPROM.read(*addr) != 0xFE) // load defaults case, if not initialized
    {
        debug_log("pressure sensor EEPROM unitialized or improperly terminated at location " + String(*addr) + ", loading defaults.");
        hi_v = P_HI_VOLTS;
        low_v = P_LOW_VOLTS;
        hi_p = P_HIGH_PRES;
        low_p = P_LOW_PRES;
        *addr += (sizeof(float) * 4) + 2; // need to increment address pointer past data, spares and terminator
        return;                           // once defaults are loaded, dont bother reading rest of EEPROM
    }

    hi_v = ee_read_float(addr);  // high analogRead calibration value
    low_v = ee_read_float(addr); // low analogRead calibration value
    hi_p = ee_read_float(addr);  // high psi calibration value
    low_p = ee_read_float(addr); // low psi calibration value

    // no more spares
    // addr += (sizeof(int) * 4) + 1; // need to increment address pointer past spares and terminator
    *addr++; // dont bother checking FF terminator
    return;
}

// ------------------- HEIGHT SENSOR FUNCTIONS -----------------------

// return PSI of sensor, using mapping in airsettings.h
float HSensor::h_mm()
{
    // scale output using settings in airsettings.h
    return map((float)analogRead(_pin), low_v, hi_v, low_h, hi_h);
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
        debug_log("pressure sensor EEPROM uninitialized at location " + String(*addr) + ", initializing.");
        EEPROM.write(*addr, 0xFE); // initializer
    }
    *addr++; // move pointer past initializer

    // bulk of data stored
    ee_write_float(addr, this->hi_v);  // high analogRead calibration value
    ee_write_float(addr, this->low_v); // low analogRead calibration value
    ee_write_float(addr, this->hi_h);  // high psi calibration value
    ee_write_float(addr, this->low_h); // low psi calibration value

    /*
    // spare integers
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    ee_write_int(addr, 0);
    */

    EEPROM.update(*addr++, 0xFF); // close with FF byte
    return;
}

// pass starting address, returns finishing address
void HSensor::eeprom_load(uint16_t *addr)
{
    if (EEPROM.read(*addr) != 0xFE) // load defaults case, if not initialized
    {
        debug_log("pressure sensor EEPROM unitialized or improperly terminated at location " + String(*addr) + ", loading defaults.");
        this->hi_v = P_HI_VOLTS;
        this->low_v = P_LOW_VOLTS;
        this->hi_h = P_HIGH_PRES;
        this->low_h = P_LOW_PRES;
        *addr += (sizeof(float) * 4) + 2; // need to increment address pointer past data, spares and terminator
        return;                           // once defaults are loaded, dont bother reading rest of EEPROM
    }

    this->hi_v = ee_read_float(addr);  // high analogRead calibration value
    this->low_v = ee_read_float(addr); // low analogRead calibration value
    this->hi_h = ee_read_float(addr);  // high psi calibration value
    this->low_h = ee_read_float(addr); // low psi calibration value

    // no more spares
    // addr += (sizeof(int) * 4) + 1; // need to increment address pointer past spares and terminator
    *addr++; // dont bother checking FF terminator
    return;
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
