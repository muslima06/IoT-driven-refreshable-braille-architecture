#ifndef BRAILLE_TRANSLATOR_H
#define BRAILLE_TRANSLATOR_H

#include <Arduino.h>

class BrailleTranslator {
public:
    static void init();
    static void getBraillePattern(char c, int pattern[6]);
};

#endif
