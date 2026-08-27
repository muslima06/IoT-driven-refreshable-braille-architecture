#ifndef TEXT_NAVIGATOR_H
#define TEXT_NAVIGATOR_H

#include <Arduino.h>

class TextNavigator {
public:
    static void setText(const String& newText);
    static char getCurrentChar();
    static bool nextChar();
    static bool prevChar();
    static bool hasText();
    
private:
    static String text;
    static int currentIndex;
};

#endif
