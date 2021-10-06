
#include "testbench.h"

// heightPin, pressurePin, fillPin, dumpPin
TestShock *RF = new TestShock(2,3,4,5); // right front shock
TestShock *LF = new TestShock(6,7,8,9); // left front shock
TestShock *RR = new TestShock(10,11,12,13); // right rear shock
TestShock *LR = new TestShock(14,15,16,17); // left rear shock

// motorPin, pressurePin
TestCompressor *comp = new TestCompressor(18,19); // compressor

// counts to 4 repeatedly
// allows for one data dump per second
uint8_t count = 0;

void setup()
{
    Serial.begin(115200); // enable serial for debug
    while (!Serial)
        delay(10);
}

void loop()
{
    // start by doing compressor update loop
    // I am just doing this in here, since there is only one compressor
    if (digitalRead(comp->_ppin))
        comp->runMotor();
    
    // calculate the number of shocks pulling from compressor
    uint8_t drawing = 0;
    drawing += RF->_filling;
    drawing += LF->_filling;
    drawing += RR->_filling;
    drawing += LR->_filling;
    comp->drawRate(drawing);

    // update pressure sensor output
    analogWrite(comp->_ppin, (uint8_t) map(comp->getPressure(), 0.0, COMP_MAXPSI, 0, 255));

    // then, run update loop for all four shocks
    updateShock(RF, comp);
    updateShock(LF, comp);
    updateShock(RR, comp);
    updateShock(LR, comp);

    delay(250);

    // TODO: do a data dump here
    if (++count == 4)
    {
        count = 0; // pee pee poo poo check
        // display each shock psi to serial
        Serial.println("RF: " + String(RF->getPressure()) + "psi");
        Serial.println("LF: " + String(LF->getPressure()) + "psi");
        Serial.println("RR: " + String(RR->getPressure()) + "psi");
        Serial.println("LR: " + String(LR->getPressure()) + "psi");
        Serial.println("CP: " + String(comp->getPressure()) + "psi\n\n");
        
    }

}