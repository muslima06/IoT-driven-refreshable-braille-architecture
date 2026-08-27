#include "BrailleTranslator.h"
#include <ctype.h>

void BrailleTranslator::init() {
    // Initialization if needed
}

void BrailleTranslator::getBraillePattern(char c, int pattern[6]) {
    // Reset pattern
    for(int i = 0; i < 6; i++) pattern[i] = 0;
    
    c = tolower(c);
    
    // Standard Braille Layout Mapping:
    // pattern[0] = Dot 1 (Top-Left)
    // pattern[1] = Dot 2 (Middle-Left)
    // pattern[2] = Dot 3 (Bottom-Left)
    // pattern[3] = Dot 4 (Top-Right)
    // pattern[4] = Dot 5 (Middle-Right)
    // pattern[5] = Dot 6 (Bottom-Right)
    
    switch(c) {
        case 'a': pattern[0]=1; break;
        case 'b': pattern[0]=1; pattern[1]=1; break;
        case 'c': pattern[0]=1; pattern[3]=1; break;
        case 'd': pattern[0]=1; pattern[3]=1; pattern[4]=1; break;
        case 'e': pattern[0]=1; pattern[4]=1; break;
        case 'f': pattern[0]=1; pattern[1]=1; pattern[3]=1; break;
        case 'g': pattern[0]=1; pattern[1]=1; pattern[3]=1; pattern[4]=1; break;
        case 'h': pattern[0]=1; pattern[1]=1; pattern[4]=1; break;
        case 'i': pattern[1]=1; pattern[3]=1; break;
        case 'j': pattern[1]=1; pattern[3]=1; pattern[4]=1; break;
        case 'k': pattern[0]=1; pattern[2]=1; break;
        case 'l': pattern[0]=1; pattern[1]=1; pattern[2]=1; break;
        case 'm': pattern[0]=1; pattern[2]=1; pattern[3]=1; break;
        case 'n': pattern[0]=1; pattern[2]=1; pattern[3]=1; pattern[4]=1; break;
        case 'o': pattern[0]=1; pattern[2]=1; pattern[4]=1; break;
        case 'p': pattern[0]=1; pattern[1]=1; pattern[2]=1; pattern[3]=1; break;
        case 'q': pattern[0]=1; pattern[1]=1; pattern[2]=1; pattern[3]=1; pattern[4]=1; break;
        case 'r': pattern[0]=1; pattern[1]=1; pattern[2]=1; pattern[4]=1; break;
        case 's': pattern[1]=1; pattern[2]=1; pattern[3]=1; break;
        case 't': pattern[1]=1; pattern[2]=1; pattern[3]=1; pattern[4]=1; break;
        case 'u': pattern[0]=1; pattern[2]=1; pattern[5]=1; break;
        case 'v': pattern[0]=1; pattern[1]=1; pattern[2]=1; pattern[5]=1; break;
        case 'w': pattern[1]=1; pattern[3]=1; pattern[4]=1; pattern[5]=1; break;
        case 'x': pattern[0]=1; pattern[2]=1; pattern[3]=1; pattern[5]=1; break;
        case 'y': pattern[0]=1; pattern[2]=1; pattern[3]=1; pattern[4]=1; pattern[5]=1; break;
        case 'z': pattern[0]=1; pattern[2]=1; pattern[4]=1; pattern[5]=1; break;
        case ' ': // space is all zeros
            break; 
        default:
            // unmapped character, leave pattern as all zeros
            break;
    }
}
