#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class LCDManager {
public:
    static void init();
    static void displayStatus(const String& status);
    static void displayCharacter(char c, int pattern[6]);
    static void displayIPAddress(const String& ip);
};

#endif
