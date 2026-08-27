#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "Sam"
#define WIFI_PASSWORD "sadiashams"

// Cloud API Configuration
#define API_URL "http://10.187.228.36:8000/scan"

// Firebase Configuration
#define FIREBASE_HOST "ocr-smart-braille-book-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "Qf6tCQnZUJgvy9wMaX2L52trv6MwD2tXReOX9W6f"

// I2C LCD
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 14

// DRV8833 #1
#define PIN_DRV1_AIN1 38
#define PIN_DRV1_AIN2 39
#define PIN_DRV1_BIN1 40
#define PIN_DRV1_BIN2 41

// DRV8833 #2
#define PIN_DRV2_AIN1 1
#define PIN_DRV2_AIN2 2
#define PIN_DRV2_BIN1 3
#define PIN_DRV2_BIN2 42

// DRV8833 #3
#define PIN_DRV3_AIN1 45
#define PIN_DRV3_AIN2 46
#define PIN_DRV3_BIN1 47
#define PIN_DRV3_BIN2 48

// Buttons
#define PIN_BTN_CAPTURE 0 // On-board BOOT button
#define PIN_BTN_PAUSE 19  // USB D- (safe if Native USB unplugged)
#define PIN_BTN_PREV 20   // USB D+ (safe if Native USB unplugged)
#define PIN_BTN_NEXT 44 // RX pin (safe as input, TX still works for Serial output)

extern bool g_isPaused;

#endif
