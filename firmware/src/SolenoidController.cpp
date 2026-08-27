#include "SolenoidController.h"
#include "Config.h"
#include "driver/gpio.h" // Required to reset JTAG peripheral configuration on GPIO pins

// Each row is one complete H-bridge channel.  The order is the standard
// Braille order: 1, 2, 3 down the left column, then 4, 5, 6 down the right.
// Each dot is mapped directly to its H-bridge channel in the physical wiring.
static const int DOT_PINS[6][2] = {
    {PIN_DRV1_AIN1, PIN_DRV1_AIN2}, // Dot 1
    {PIN_DRV1_BIN1, PIN_DRV1_BIN2}, // Dot 2
    {PIN_DRV2_AIN1, PIN_DRV2_AIN2}, // Dot 3
    {PIN_DRV2_BIN1, PIN_DRV2_BIN2}, // Dot 4
    {PIN_DRV3_AIN1, PIN_DRV3_AIN2}, // Dot 5
    {PIN_DRV3_BIN1, PIN_DRV3_BIN2}  // Dot 6
};

static bool getDotPins(int dotNumber, int &p1, int &p2) {
    if (dotNumber < 1 || dotNumber > 6) {
        return false;
    }
    p1 = DOT_PINS[dotNumber - 1][0];
    p2 = DOT_PINS[dotNumber - 1][1];
    return true;
}

void SolenoidController::init() {
    // GPIO39-GPIO42 can be claimed by the ESP32-S3 GPIO-JTAG interface at boot.
    // They drive dots 1-4 in this design, so release their alternate functions
    // before configuring them as ordinary output pins. Reset all motor pins too,
    // including the strapping pins used by driver #3, to start from a known state.
    const int motorPins[] = {
        PIN_DRV1_AIN1, PIN_DRV1_AIN2, PIN_DRV1_BIN1, PIN_DRV1_BIN2,
        PIN_DRV2_AIN1, PIN_DRV2_AIN2, PIN_DRV2_BIN1, PIN_DRV2_BIN2,
        PIN_DRV3_AIN1, PIN_DRV3_AIN2, PIN_DRV3_BIN1, PIN_DRV3_BIN2
    };
    for (int pin : motorPins) {
        gpio_reset_pin(static_cast<gpio_num_t>(pin));
    }

    // Set pin modes
    pinMode(PIN_DRV1_AIN1, OUTPUT); pinMode(PIN_DRV1_AIN2, OUTPUT);
    pinMode(PIN_DRV1_BIN1, OUTPUT); pinMode(PIN_DRV1_BIN2, OUTPUT);
    
    pinMode(PIN_DRV2_AIN1, OUTPUT); pinMode(PIN_DRV2_AIN2, OUTPUT);
    pinMode(PIN_DRV2_BIN1, OUTPUT); pinMode(PIN_DRV2_BIN2, OUTPUT);
    
    pinMode(PIN_DRV3_AIN1, OUTPUT); pinMode(PIN_DRV3_AIN2, OUTPUT);
    pinMode(PIN_DRV3_BIN1, OUTPUT); pinMode(PIN_DRV3_BIN2, OUTPUT);

    // Start with all solenoids OFF
    digitalWrite(PIN_DRV1_AIN1, LOW); digitalWrite(PIN_DRV1_AIN2, LOW);
    digitalWrite(PIN_DRV1_BIN1, LOW); digitalWrite(PIN_DRV1_BIN2, LOW);
    digitalWrite(PIN_DRV2_AIN1, LOW); digitalWrite(PIN_DRV2_AIN2, LOW);
    digitalWrite(PIN_DRV2_BIN1, LOW); digitalWrite(PIN_DRV2_BIN2, LOW);
    digitalWrite(PIN_DRV3_AIN1, LOW); digitalWrite(PIN_DRV3_AIN2, LOW);
    digitalWrite(PIN_DRV3_BIN1, LOW); digitalWrite(PIN_DRV3_BIN2, LOW);
}

void SolenoidController::runSelfTest() {
    Serial.println("--- STARTING SOLENOID SELF-TEST ---");
    for (int i = 1; i <= 6; i++) {
        Serial.print("Testing Solenoid ");
        Serial.println(i);
        raiseDot(i);
        delay(400); // 400ms pulse
        lowerDot(i);
        delay(100);
        digitalWrite(PIN_DRV1_AIN1, LOW); digitalWrite(PIN_DRV1_AIN2, LOW);
        digitalWrite(PIN_DRV1_BIN1, LOW); digitalWrite(PIN_DRV1_BIN2, LOW);
        digitalWrite(PIN_DRV2_AIN1, LOW); digitalWrite(PIN_DRV2_AIN2, LOW);
        digitalWrite(PIN_DRV2_BIN1, LOW); digitalWrite(PIN_DRV2_BIN2, LOW);
        digitalWrite(PIN_DRV3_AIN1, LOW); digitalWrite(PIN_DRV3_AIN2, LOW);
        digitalWrite(PIN_DRV3_BIN1, LOW); digitalWrite(PIN_DRV3_BIN2, LOW);
        delay(200); // gap between tests
    }
    Serial.println("--- SELF-TEST COMPLETE ---");
}

void SolenoidController::raiseDot(int dotNumber) {
    int p1 = -1, p2 = -1;
    if (getDotPins(dotNumber, p1, p2)) {
        digitalWrite(p1, HIGH);
        digitalWrite(p2, LOW);
    }
}

void SolenoidController::lowerDot(int dotNumber) {
    int p1 = -1, p2 = -1;
    if (getDotPins(dotNumber, p1, p2)) {
        digitalWrite(p1, LOW);
        digitalWrite(p2, HIGH);
    }
}

static void turnOffDot(int dotNumber) {
    int p1 = -1, p2 = -1;
    if (getDotPins(dotNumber, p1, p2)) {
        digitalWrite(p1, LOW);
        digitalWrite(p2, LOW);
    }
}

static void turnOffAllDots() {
    for (int dot = 1; dot <= 6; dot++) {
        turnOffDot(dot);
    }
}

static void lowerAllDots() {
    // A new character must never inherit a raised pin from the previous one.
    // Drive every H-bridge briefly in its retract direction, then coast.
    for (int dot = 1; dot <= 6; dot++) {
        int p1, p2;
        getDotPins(dot, p1, p2);
        digitalWrite(p1, LOW);
        digitalWrite(p2, HIGH);
    }
    delay(100);
    turnOffAllDots();
}

void SolenoidController::displayBrailleCell(int pattern[6]) {
    // pattern array now uses STANDARD Braille numbering:
    // pattern[0] -> Dot 1 (Solenoid 1)
    // pattern[1] -> Dot 2 (Solenoid 2)
    // pattern[2] -> Dot 3 (Solenoid 3)
    // pattern[3] -> Dot 4 (Solenoid 4)
    // pattern[4] -> Dot 5 (Solenoid 5)
    // pattern[5] -> Dot 6 (Solenoid 6)

    // Extract active dots
    int activeDots[6];
    int count = 0;
    for (int i = 0; i < 6; i++) {
        if (pattern[i] == 1) {
            activeDots[count++] = i + 1; // Map index 0->1, 1->2... 5->6
        }
    }

    Serial.print("Braille output dots:");
    for (int i = 0; i < count; i++) {
        int p1, p2;
        getDotPins(activeDots[i], p1, p2);
        Serial.printf(" %d(GPIO%d/GPIO%d)", activeDots[i], p1, p2);
    }
    Serial.println();

    // Clear the full cell, rather than only the dots used by the previous
    // letter. This prevents an old dot (for example dot 3) staying raised.
    lowerAllDots();

    if (count == 0) {
        delay(2000); // Wait 2 seconds for space / empty pattern
        return;
    }

    // 1. SEQUENTIAL POP PHASE:
    // Pop each active dot one-by-one at 100% duty cycle for 120ms.
    // To prevent already popped dots from falling down, we hold them at 75% duty cycle.
    for (int i = 0; i < count; i++) {
        int currentDot = activeDots[i];
        
        if (i == 0) {
            // First dot: just turn it ON at 100% for 120ms
            raiseDot(currentDot);
            delay(120);
        } else {
            // Subsequent dots: pop currentDot (100% duty cycle) while holding previous dots (75% duty cycle)
            // Period = 4ms. We do this for 120ms (30 cycles).
            for (int cycle = 0; cycle < 30; cycle++) {
                // Slot 0: All ON (3ms)
                raiseDot(currentDot);
                for (int j = 0; j < i; j++) {
                    raiseDot(activeDots[j]);
                }
                delayMicroseconds(3000);

                // Slot 1: Current ON, previous OFF (1ms)
                for (int j = 0; j < i; j++) {
                    turnOffDot(activeDots[j]);
                }
                delayMicroseconds(1000);
            }
        }
    }

    // 2. STAGGERED HOLD PHASE:
    // Hold all active dots together for exactly 2.0 seconds using a 75% to 83% duty cycle software PWM.
    // At most (count - 1) solenoids are ON at any single microsecond, preventing power supply overload.
    unsigned long startHold = millis();
    if (count <= 3) {
        // Simple 75% duty cycle for 3 or fewer pins (no staggering needed)
        while (millis() - startHold < 2000) {
            // Turn ON all active pins (3ms)
            for (int i = 0; i < count; i++) {
                raiseDot(activeDots[i]);
            }
            delayMicroseconds(3000);

            // Turn OFF all active pins (1ms)
            for (int i = 0; i < count; i++) {
                turnOffDot(activeDots[i]);
            }
            delayMicroseconds(1000);

            // Button check to abort
            if (digitalRead(PIN_BTN_NEXT) == LOW || 
                digitalRead(PIN_BTN_PREV) == LOW || 
                digitalRead(PIN_BTN_PAUSE) == LOW) {
                break;
            }
        }
    } else {
        // Staggered software PWM for 4 or more pins
        // Period = 4ms = 4000 microseconds.
        // Each pin is turned OFF for one slot of duration (4000 / count) microseconds.
        int slotDuration = 4000 / count;
        while (millis() - startHold < 2000) {
            for (int slot = 0; slot < count; slot++) {
                // In this slot, turn OFF the slot-th pin, and turn ON all other pins
                for (int i = 0; i < count; i++) {
                    if (i == slot) {
                        turnOffDot(activeDots[i]);
                    } else {
                        raiseDot(activeDots[i]);
                    }
                }
                delayMicroseconds(slotDuration);
            }

            // Button check to abort
            if (digitalRead(PIN_BTN_NEXT) == LOW || 
                digitalRead(PIN_BTN_PREV) == LOW || 
                digitalRead(PIN_BTN_PAUSE) == LOW) {
                break;
            }
        }
    }

    // Clear all six dots. This also releases dots that were raised by an
    // interrupted display cycle or by a previous character.
    lowerAllDots();
}
