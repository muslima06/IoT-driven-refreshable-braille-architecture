#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

class ButtonManager {
public:
    static void init();
    static void update();
    static bool isCapturePressed();
    static bool isPausePressed();
    static bool isPrevPressed();
    static bool isNextPressed();
};

#endif
