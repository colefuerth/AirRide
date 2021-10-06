
#include "testbench.h"

TestShock::TestShock(uint8_t heightPin, uint8_t pressurePin, uint8_t fillPin, uint8_t dumpPin)
{
    _hpin = heightPin;
    _ppin = pressurePin;
    _fillPin = fillPin;
    _dumpValvePin = dumpPin;
    pinMode(_hpin, OUTPUT);
    pinMode(_ppin, OUTPUT);
    pinMode(_fillPin, INPUT_PULLUP);
    pinMode(_dumpValvePin, INPUT_PULLUP);
}

// max fill rate is 1psi/call
// slows to 0.2psi/call from max over the last 10psi to cmp_psi
void TestShock::fill(float cmp_psi)
{   
    this->_filling = 1;
    // check for absolute max shock pressure
    if (_psi >= _maxPSI)
    {
        _psi = _maxPSI;
        return;
    }

    // calculate the difference in pressure
    float diff = cmp_psi - _psi;  

    // if _psi is greater than cmp_psi, then diff will be less than 0
    // _psi cannot be filled greater than the compressor PSI
    if (diff <= 0.0)
    {
        if (diff < 0.0)
            _psi = cmp_psi;
        return;
    }

    if (diff < 10.0)
    {
        _psi += map(diff, 0.0, 10.0, 0.2, 1.0);
        if (_psi > _maxPSI)
            _psi = _maxPSI;
        return;
    }

    // if none of the other conditions are met, 
    // then the compressor is filling at full tits.
    _psi += 1.0; 
    return;
}

// each time this is called. bag pressure drops by 0.25psi
void TestShock::dump()
{
    // check for absolute min shock pressure
    if (_psi <= _minPSI)
    {
        _psi = _minPSI;
        return;
    }

    // calculate the difference in pressure
    float diff = _psi - _minPSI;  

    if (diff < 10.0)
    {
        _psi -= map(diff, 0.0, 10.0, 0.2, 1.0);
        if (_psi < _minPSI)
            _psi = _maxPSI;
        return;
    }

    // if none of the other conditions are met, 
    // then the compressor is filling at full tits.
    _psi -= 1.0; 

    return;
}

// grab psi value
float TestShock::getPressure()
{
    return _psi;
}

// map height off of _psi value, using mapping in airsettings.h
float TestShock::getHeight()
{
    return map(_psi, _minPSI, _maxPSI, H_LOW_HEIGHT, H_HIGH_HEIGHT);
}

// increase PSI by 0.25 / call
void TestCompressor::runMotor()
{
    if (_psi >= COMP_MAXPSI - 0.25)
        _psi = COMP_MAXPSI;
    else
        _psi += 0.25;
    return;
}

TestCompressor::TestCompressor(uint8_t motorPin, uint8_t pressurePin)
{
    _motorPin = motorPin;
    _ppin = pressurePin;
    pinMode(_ppin, OUTPUT);
    pinMode(_motorPin, INPUT_PULLUP);
}

// each shock dumps 0.125psi/call
void TestCompressor::drawRate(int numShocksFilling)
{
    _psi -= numShocksFilling * 0.125;
}

// returns value of _psi
float TestCompressor::getPressure()
{
    return _psi;
}


void updateShock (TestShock *shk, TestCompressor *compr)
{
    // start by checking digital control pins
    boolean fillInput = digitalRead(shk->_fillPin);
    boolean dumpInput = digitalRead(shk->_dumpValvePin);

    // run relevant functions for updating state, depending on state of control pins
    shk->_filling = 0; // set filling to 0; fill() will set to 1 again if filling
    if (fillInput)
        shk->fill(compr->getPressure());
    if (dumpInput)
        shk->dump();

    // update bag pressure output
    // yes, map is expensive. no, i dont care
    uint8_t h = (uint8_t) map(shk->getPressure(), 0, SHK_MAX_PSI, 0, 255);
    analogWrite(shk->_ppin, h);
    analogWrite(shk->_hpin, h);
}