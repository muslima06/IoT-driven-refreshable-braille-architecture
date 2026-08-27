#include "ButtonManager.h"
#include "Config.h"

static bool capturePressed = false;
static bool pausePressed = false;
static bool prevPressed = false;
static bool nextPressed = false;

static unsigned long lastDebounceCapture = 0;
static unsigned long lastDebouncePause = 0;
static unsigned long lastDebouncePrev = 0;
static unsigned long lastDebounceNext = 0;
const unsigned long debounceDelay = 50;

static int lastCaptureState = HIGH;
static int lastPauseState = HIGH;
static int lastPrevState = HIGH;
static int lastNextState = HIGH;

void ButtonManager::init() {
    pinMode(PIN_BTN_CAPTURE, INPUT_PULLUP);
    pinMode(PIN_BTN_PAUSE, INPUT_PULLUP);
    pinMode(PIN_BTN_PREV, INPUT_PULLUP);
    pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
}

void ButtonManager::update() {
    capturePressed = false;
    pausePressed = false;
    prevPressed = false;
    nextPressed = false;

    // Capture Button
    int readingCapture = digitalRead(PIN_BTN_CAPTURE);
    if (readingCapture != lastCaptureState) {
        lastDebounceCapture = millis();
    }
    if ((millis() - lastDebounceCapture) > debounceDelay) {
        if (readingCapture == LOW) {
            capturePressed = true;
        }
    }
    lastCaptureState = readingCapture;

    // Pause Button
    int readingPause = digitalRead(PIN_BTN_PAUSE);
    if (readingPause != lastPauseState) {
        lastDebouncePause = millis();
    }
    if ((millis() - lastDebouncePause) > debounceDelay) {
        if (readingPause == LOW) {
            pausePressed = true;
        }
    }
    lastPauseState = readingPause;

    // Prev Button
    int readingPrev = digitalRead(PIN_BTN_PREV);
    if (readingPrev != lastPrevState) {
        lastDebouncePrev = millis();
    }
    if ((millis() - lastDebouncePrev) > debounceDelay) {
        if (readingPrev == LOW) {
            prevPressed = true;
        }
    }
    lastPrevState = readingPrev;

    // Next Button
    int readingNext = digitalRead(PIN_BTN_NEXT);
    if (readingNext != lastNextState) {
        lastDebounceNext = millis();
    }
    if ((millis() - lastDebounceNext) > debounceDelay) {
        if (readingNext == LOW) {
            nextPressed = true;
        }
    }
    lastNextState = readingNext;
}

bool ButtonManager::isCapturePressed() {
    bool temp = capturePressed;
    capturePressed = false; // consume
    return temp;
}

bool ButtonManager::isPausePressed() {
    bool temp = pausePressed;
    pausePressed = false; // consume
    return temp;
}

bool ButtonManager::isPrevPressed() {
    bool temp = prevPressed;
    prevPressed = false;
    return temp;
}

bool ButtonManager::isNextPressed() {
    bool temp = nextPressed;
    nextPressed = false;
    return temp;
}
