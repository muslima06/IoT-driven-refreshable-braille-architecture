#include "LCDManager.h"
#include "Config.h"

extern LiquidCrystal_I2C lcd;

void LCDManager::init() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Init...");
}

void LCDManager::displayStatus(const String& status) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(status);
}

void LCDManager::displayCharacter(char c, int pattern[6]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reading: ");
    lcd.print(c);
    
    lcd.setCursor(0, 1);
    lcd.print(c);
    lcd.print(" -> ");
    for(int i=0; i<6; i++) {
        lcd.print(pattern[i]);
    }
}

void LCDManager::displayIPAddress(const String& ip) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stream:");
    lcd.setCursor(0, 1);
    lcd.print(ip + ":81");
}
