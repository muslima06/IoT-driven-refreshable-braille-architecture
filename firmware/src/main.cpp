#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "CameraManager.h"
#include "camera_server.h"
#include "OCRClient.h"
#include "BrailleTranslator.h"
#include "SolenoidController.h"
#include "ButtonManager.h"
#include "LCDManager.h"
#include "TextNavigator.h"
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
// Uncomment to enable Firebase streaming (default enabled)
#define ENABLE_FIREBASE_STREAM
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool g_isPaused = false;
bool g_isDisplaying = false;           // Busy flag: prevents re-entrant display calls
String g_lastReceivedText = "";        // Tracks last Firebase text to prevent duplicate triggers
// Initialize to a large value so auto-advance does NOT fire immediately on boot
unsigned long lastDisplayUpdate = 0xFFFFFFFF - 10000;
const unsigned long autoNextDelay = 2000; // 2 seconds between auto-advance

// Firebase configuration objects
FirebaseData fbdoStream;
FirebaseAuth auth;
FirebaseConfig configFb;

// Forward declaration of display update helper
void updateDisplayForCurrentChar();

void streamCallback(FirebaseStream data) {
    if (data.dataType() == "string") {
        String text = data.stringData();
        Serial.printf("Firebase update: %s\n", text.c_str());
        
        // GUARD: Firebase stream fires twice on connect (once with existing value,
        // once on change). Ignore the duplicate to prevent showing the same letter twice.
        if (text.length() > 0 && text != g_lastReceivedText) {
            g_lastReceivedText = text;
            TextNavigator::setText(text);
            g_isPaused = false;
            lastDisplayUpdate = millis();
            updateDisplayForCurrentChar();
        }
    }
}

void streamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("Firebase Stream timeout, resuming...");
    }
}
// Pause/resume Firebase stream during OCR upload
void pauseFirebaseStream(bool pause) {
    if (pause) {
        Firebase.RTDB.endStream(&fbdoStream);
        Serial.println("Firebase stream paused for OCR upload");
    } else {
        Firebase.RTDB.beginStream(&fbdoStream, "/active_text");
        Serial.println("Firebase stream resumed");
    }
}

#ifdef ENABLE_FIREBASE_STREAM
void setupFirebase() {
    configFb.host = FIREBASE_HOST;
    configFb.signer.tokens.legacy_token = FIREBASE_AUTH;
    
    Firebase.begin(&configFb, &auth);
    Firebase.reconnectWiFi(true);
    
    if (!Firebase.RTDB.beginStream(&fbdoStream, "/active_text")) {
        Serial.printf("Firebase stream error: %s\n", fbdoStream.errorReason().c_str());
    } else {
        Firebase.RTDB.setStreamCallback(&fbdoStream, streamCallback, streamTimeoutCallback);
        Serial.println("Firebase Stream configured successfully!");
    }
}
#endif

void setupWiFi() {
    LCDManager::displayStatus("Connecting WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        LCDManager::displayStatus("WiFi Connected!");
    } else {
        Serial.println("");
        Serial.println("WiFi connection failed");
        LCDManager::displayStatus("WiFi Failed");
    }
    delay(1000);
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    // Initialize peripherals
    ButtonManager::init();
    SolenoidController::init();
    SolenoidController::runSelfTest();
    LCDManager::init();

    // Connect to WiFi
    setupWiFi();

    // Set up Realtime Database listener
    setupFirebase();

    // Initialize camera
    if (!CameraManager::init()) {
        LCDManager::displayStatus("Camera Error");
        Serial.println("Camera init failed!");
    } else {
        Serial.println("Camera initialized OK");
    }

    // Start the camera web server (stream + control UI)
    startCameraServer();

    // Print access URLs
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
    Serial.print("Stream URL: http://");
    Serial.print(WiFi.localIP());
    Serial.println(":81/stream");

    // Show camera stream address on LCD for 500ms
    LCDManager::displayIPAddress(WiFi.localIP().toString());
    delay(500);

    LCDManager::displayStatus("System Ready");

    // Default text for test
    TextNavigator::setText("Hello");
    updateDisplayForCurrentChar();
}

void updateDisplayForCurrentChar() {
    // GUARD: Prevent re-entrant calls. If solenoids are already active (3-sec hold),
    // ignore any duplicate trigger from Firebase or the auto-advance timer.
    if (g_isDisplaying) return;
    g_isDisplaying = true;

    char c = TextNavigator::getCurrentChar();
    int pattern[6] = {0};
    BrailleTranslator::getBraillePattern(c, pattern);
    LCDManager::displayCharacter(c, pattern);
    SolenoidController::displayBrailleCell(pattern);
    lastDisplayUpdate = millis(); // Reset timer AFTER display so auto-advance counts from now

    g_isDisplaying = false;
}

void loop() {
    ButtonManager::update();

    if (ButtonManager::isCapturePressed()) {

    LCDManager::displayStatus("Capturing...");
    camera_fb_t* fb = CameraManager::capture();
    if (fb) {
        LCDManager::displayStatus("Uploading...");
        String result = OCRClient::sendImageForOCR(fb);
        CameraManager::release(fb);

        if (result == "__ERROR__") {
            LCDManager::displayStatus("OCR Failed");
        } else if (result == "SENT_TO_FIREBASE") {
            LCDManager::displayStatus("Waiting for Data...");
            // The Firebase streamCallback will handle the rest when the data arrives!
        } else if (result.length() > 0) {
            TextNavigator::setText(result);
            g_isPaused = false;
            updateDisplayForCurrentChar();
        } else {
            LCDManager::displayStatus("No Text Found");
        }
    } else {
        LCDManager::displayStatus("Cap Error");
    }
}


    if (ButtonManager::isPausePressed()) {
        g_isPaused = !g_isPaused;
        if (g_isPaused) {
            LCDManager::displayStatus("Paused");
        } else {
            updateDisplayForCurrentChar();
        }
    }

    if (ButtonManager::isPrevPressed()) {
        if (TextNavigator::prevChar()) {
            updateDisplayForCurrentChar();
        }
        lastDisplayUpdate = millis();
    }

    if (ButtonManager::isNextPressed()) {
        if (TextNavigator::nextChar()) {
            updateDisplayForCurrentChar();
        }
        lastDisplayUpdate = millis();
    }

    // Auto navigate if not paused
    if (!g_isPaused && TextNavigator::hasText()) {
        if (millis() - lastDisplayUpdate > autoNextDelay) {
            if (TextNavigator::nextChar()) {
                updateDisplayForCurrentChar();
            }
            lastDisplayUpdate = millis();
        }
    }

    delay(10);
}
