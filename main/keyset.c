// ChatGPT Hardware Hack compatible with Casio FX Series

// Keyset.c -> Assigns the signs and functions to the keys, also defines the LCD symbol addresses

// © 2026 Jonas Heselschwerdt
// Licensed under CC BY-NC 4.0




#include "keyset.h"
#include "config.h"
#include <stdint.h>

Key keyset[256];

// Keypad Layout Definition:

void define_keyset(){

    // Eventkeys -> \0 = Not defined
    // Hex- Values are from TCA8418 Keyevent Register (see schematics / PCB Layout and TCA8418 datasheet)

    keyset[0xB1] = (Key){SHIFT,'\0','\0','\0'};
    keyset[0xA7] = (Key){ALPHA,'\0','\0','\0'};
    keyset[0x9D] = (Key){UP,'u','\0','\0'};
    keyset[0x92] = (Key){DOWN,'d','\0','\0'};
    keyset[0x93] = (Key){RIGHT,'r','\0','\0'};
    keyset[0x9C] = (Key){LEFT,'l','\0','\0'};
    keyset[0x89] = (Key){MENU,'\0','\0','\0'};
    keyset[0x8E] = (Key){BACK,'\0','\0','\0'};
    keyset[0x84] = (Key){ENTER,'\0','\0','\0'};

    // Normal Keys

    keyset[0xB0] = (Key){EVENT_NONE,'a','A','@'};
    keyset[0xA6] = (Key){EVENT_NONE,'b','B','\0'};
    keyset[0x88] = (Key){EVENT_NONE,'c','C','\0'};
    keyset[0xAF] = (Key){EVENT_NONE,'d','D','\0'};
    keyset[0xA5] = (Key){EVENT_NONE,'e','E','\0'};
    keyset[0x9B] = (Key){EVENT_NONE,'f','F','\0'};
    keyset[0x91] = (Key){EVENT_NONE,'g','G','\0'};
    keyset[0x87] = (Key){EVENT_NONE,'h','H','\0'};
    keyset[0xAE] = (Key){EVENT_NONE,'i','I','\0'};
    keyset[0xA4] = (Key){EVENT_NONE,'j','J','\0'};
    keyset[0x9A] = (Key){EVENT_NONE,'k','K','\0'};
    keyset[0x90] = (Key){EVENT_NONE,'l','L','\0'};
    keyset[0x86] = (Key){EVENT_NONE,'m','M','\0'};
    keyset[0xAD] = (Key){EVENT_NONE,'n','N','\0'};
    keyset[0xA3] = (Key){EVENT_NONE,'o','O','\0'};
    keyset[0x99] = (Key){EVENT_NONE,'p','P','\0'};
    keyset[0x8F] = (Key){EVENT_NONE,'q','Q','\0'};
    keyset[0x85] = (Key){EVENT_NONE,'r','R','\0'};
    keyset[0xAC] = (Key){EVENT_NONE,'s','S','7'};
    keyset[0xA2] = (Key){EVENT_NONE,'t','T','8'};
    keyset[0x98] = (Key){EVENT_NONE,'u','U','9'};
    keyset[0xAB] = (Key){EVENT_NONE,'v','V','4'};
    keyset[0xA1] = (Key){EVENT_NONE,'w','W','5'};
    keyset[0x97] = (Key){EVENT_NONE,'x','X','6'};
    keyset[0x8D] = (Key){EVENT_NONE,'y','Y','$'};
    keyset[0x83] = (Key){EVENT_NONE,'z','Z',scharfS};
    keyset[0xAA] = (Key){EVENT_NONE,'.','*','1'};
    keyset[0xA0] = (Key){EVENT_NONE,',','\'','2'};
    keyset[0x96] = (Key){EVENT_NONE,'!','&','3'};
    keyset[0x8C] = (Key){EVENT_NONE,'?','%','{'};
    keyset[0x82] = (Key){EVENT_NONE,'/','#','}'};
    keyset[0xA9] = (Key){EVENT_NONE,'"','^','0'};
    keyset[0x9F] = (Key){EVENT_NONE,'-','+','_'};
    keyset[0x95] = (Key){EVENT_NONE,' ','=','\\'};
    keyset[0x8B] = (Key){EVENT_NONE,'(',':','['};
    keyset[0x81] = (Key){EVENT_NONE,')',';',']'};

}

uint8_t lcd_charset[256];

void setup_charset(){

    for (int i = 0; i < 256; i++) lcd_charset[i] = 0x00;
    
    // Lookup Table Sign to Charset ROM A Hex Codes (see DOGM 204 datasheet)
    // ASCII Code of the chars below is used to index list lcd_charset

    // Big Letters

    lcd_charset['A'] = 0x41;
    lcd_charset['B'] = 0x42;
    lcd_charset['C'] = 0x43;
    lcd_charset['D'] = 0x44;
    lcd_charset['E'] = 0x45;
    lcd_charset['F'] = 0x46;
    lcd_charset['G'] = 0x47;
    lcd_charset['H'] = 0x48;
    lcd_charset['I'] = 0x49;
    lcd_charset['J'] = 0x4A;
    lcd_charset['K'] = 0x4B;
    lcd_charset['L'] = 0x4C;
    lcd_charset['M'] = 0x4D;
    lcd_charset['N'] = 0x4E;
    lcd_charset['O'] = 0x4F;
    lcd_charset['P'] = 0x50;
    lcd_charset['Q'] = 0x51;
    lcd_charset['R'] = 0x52;
    lcd_charset['S'] = 0x53;
    lcd_charset['T'] = 0x54;
    lcd_charset['U'] = 0x55;
    lcd_charset['V'] = 0x56;
    lcd_charset['W'] = 0x57;
    lcd_charset['X'] = 0x58;
    lcd_charset['Y'] = 0x59;
    lcd_charset['Z'] = 0x5A;

    // Small letters

    lcd_charset['a'] = 0x61;
    lcd_charset['b'] = 0x62;
    lcd_charset['c'] = 0x63;
    lcd_charset['d'] = 0x64;
    lcd_charset['e'] = 0x65;
    lcd_charset['f'] = 0x66;
    lcd_charset['g'] = 0x67;
    lcd_charset['h'] = 0x68;
    lcd_charset['i'] = 0x69;
    lcd_charset['j'] = 0x6A;
    lcd_charset['k'] = 0x6B;
    lcd_charset['l'] = 0x6C;
    lcd_charset['m'] = 0x6D;
    lcd_charset['n'] = 0x6E;
    lcd_charset['o'] = 0x6F;
    lcd_charset['p'] = 0x70;
    lcd_charset['q'] = 0x71;
    lcd_charset['r'] = 0x72;
    lcd_charset['s'] = 0x73;
    lcd_charset['t'] = 0x74;
    lcd_charset['u'] = 0x75;
    lcd_charset['v'] = 0x76;
    lcd_charset['w'] = 0x77;
    lcd_charset['x'] = 0x78;
    lcd_charset['y'] = 0x79;
    lcd_charset['z'] = 0x7A;

    // ASCII Symbols

    lcd_charset['!'] = 0x21;
    lcd_charset['"'] = 0x22;
    lcd_charset['#'] = 0x23;
    lcd_charset['$'] = 0xA2;
    lcd_charset['%'] = 0x25;
    lcd_charset['&'] = 0x26;
    lcd_charset['\''] = 0x27;
    lcd_charset['('] = 0x28;
    lcd_charset[')'] = 0x29;
    lcd_charset['*'] = 0x2A;
    lcd_charset['+'] = 0x2B;
    lcd_charset[','] = 0x2C;
    lcd_charset['-'] = 0x2D;
    lcd_charset['.'] = 0x2E;
    lcd_charset['/'] = 0x2F;
    lcd_charset[':'] = 0x3A;
    lcd_charset[';'] = 0x3B;
    lcd_charset['<'] = 0x3C;
    lcd_charset['='] = 0x3D;
    lcd_charset['>'] = 0x3E;
    lcd_charset['?'] = 0x3F;
    lcd_charset['@'] = 0xA0;
    lcd_charset['['] = 0xFA;
    lcd_charset['\\'] = 0xFB;
    lcd_charset[']'] = 0xFC;
    lcd_charset['^'] = 0x1D;
    lcd_charset['_'] = 0xC4;
    lcd_charset['{'] = 0xFD;
    lcd_charset['}'] = 0xFF;
    lcd_charset['~'] = 0xDE;
    lcd_charset[' '] = 0x20;

    // Special signs not in ASCII

    lcd_charset[Backarrow] = 0x1C;
    lcd_charset[ArrowR] = 0x10;
    lcd_charset[ArrowL] = 0x11;
    lcd_charset[Fullblock] = 0x1F;
    lcd_charset[Circle] = 0x94;
    lcd_charset[scharfS] = 0xBE; 

    // Numbers

    lcd_charset['0'] = 0x30;
    lcd_charset['1'] = 0x31;
    lcd_charset['2'] = 0x32;
    lcd_charset['3'] = 0x33;
    lcd_charset['4'] = 0x34;
    lcd_charset['5'] = 0x35;
    lcd_charset['6'] = 0x36;
    lcd_charset['7'] = 0x37;
    lcd_charset['8'] = 0x38;
    lcd_charset['9'] = 0x39;

}


