#ifndef testbench_h
#define testbench_h

#define ROOM_PRES 15

#include <airsettings.h>    // i am so sorry about this but it was the fastest way to make it work
#include <Arduino.h>

// ASSUMING COMPRESSOR HAS DOUBLE THE VOLUME THAT EACH AIRBAG HAS
// also assuming the main loop runs at 4Hz

// TestShock simulates the shock behaviour for testing of AirRide system on a bench
// this class does not self-manage; it simply simulates the hardware signals for main controller to interact with
class TestShock
{
public:
    TestShock(uint8_t heightPin, uint8_t pressurePin, uint8_t fillPin, uint8_t dumpPin);
    void fill(float cmp_psi);  // fill rate is affected by source psi
    void dump();   // each time this is called. bag pressure drops by 0.25psi
    float getPressure(); // grab psi value
    float getHeight();  // map height off of _psi value, using mapping in airsettings.h
    uint8_t _hpin;  // analog height sensor pin
    uint8_t _ppin;  // analog pressure sensor pin
    uint8_t _fillPin;   // digital fill valve input pin
    uint8_t _dumpValvePin;  // digital dump valve input pin
    uint8_t _filling = 0; // 1 if filling, 0 otherwise

private:
    int _maxPSI = SHK_MAX_PSI;    // used as the overpressure valve simulation setpoint
    int _minPSI = ROOM_PRES;    // pressure at ground level
    float _psi = ROOM_PRES;
};

class TestCompressor
{
public:
    TestCompressor(uint8_t motorPin, uint8_t pressurePin);
    void runMotor();    // increase PSI by 0.25 / call
    void drawRate(int numShocksFilling);   // each shock dumps 0.125psi/call
    float getPressure();    // returns value of _psi
    uint8_t _motorPin;  // digital motor run input pin
    uint8_t _ppin;  // analog pressure sensor pin
private:
    float _psi = ROOM_PRES; // start at 15psi
    int _minPSI = ROOM_PRES; // cannot dump pressure below 0psi
    int _maxPSI = COMP_MAXPSI; // dump valve setting, stop filling beyond this point
};

void updateShock (TestShock *shk, TestCompressor *compr);

#endif // end of testbench_h define