#include "TextNavigator.h"

String TextNavigator::text = "";
int TextNavigator::currentIndex = 0;

void TextNavigator::setText(const String& newText) {
    text = newText;
    currentIndex = 0;
}

char TextNavigator::getCurrentChar() {
    if (currentIndex >= 0 && currentIndex < text.length()) {
        return text.charAt(currentIndex);
    }
    return '\0';
}

bool TextNavigator::nextChar() {
    if (currentIndex < text.length() - 1) {
        currentIndex++;
        return true;
    }
    return false;
}

bool TextNavigator::prevChar() {
    if (currentIndex > 0) {
        currentIndex--;
        return true;
    }
    return false;
}

bool TextNavigator::hasText() {
    return text.length() > 0;
}
