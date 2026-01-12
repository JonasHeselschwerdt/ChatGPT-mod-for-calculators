// ChatGPT Hardware Hack compatible with Casio FX Series

// Keyset.h -> Assigns the signs and functions to the keys, also defines the LCD symbol addresses

// © 2026 Jonas Heselschwerdt
// Free for personal, research and educational use
// Commercial use requires written permission




#ifndef KEYMAP_H   
#define KEYMAP_H   

#include <stdint.h>

typedef enum {

    EVENT_NONE,
    SHIFT,
    ALPHA,
    UP,
    DOWN,
    RIGHT,
    LEFT,
    MENU,
    BACK,
    ENTER

} key_event_type;

typedef struct{

    key_event_type event_key;
    uint8_t normal_value;
    uint8_t shift_value;
    uint8_t alpha_value;

} Key;

extern Key keyset[256];

extern uint8_t lcd_charset[256];

void define_keyset(void);
void setup_charset(void);

#endif