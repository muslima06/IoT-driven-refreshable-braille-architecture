#ifndef SOLENOID_CONTROLLER_H
#define SOLENOID_CONTROLLER_H

#include <Arduino.h>

class SolenoidController {
public:
    static void init();
    static void displayBrailleCell(int pattern[6]);
    static void runSelfTest();
    
private:
    static void raiseDot(int dotNumber);
    static void lowerDot(int dotNumber);
};

#endif
