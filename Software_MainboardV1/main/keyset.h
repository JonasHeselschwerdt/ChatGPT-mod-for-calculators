// ChatGPT Hardware Hack compatible with Casio FX Series

// Keyset.h -> Assigns the signs and functions to the keys, also defines the LCD symbol addresses

// © 2026 Jonas Heselschwerdt
// Licensed under CC BY-NC 4.0




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
    char stealth_value;

} Key;

typedef struct{

    uint8_t hid_address;
    uint8_t modifier_byte;

} Hid_Key;

typedef struct{

    key_event_type special_event_key;
    uint8_t hid_address;
    uint8_t modifier_byte;

} Special_Hid_Key;

extern Key keyset[256];

extern uint8_t lcd_charset[256];

extern Hid_Key hid_LUT[256];
extern Special_Hid_Key special_hid_LUT[128];

void define_keyset(void);
void setup_charset(void);
void setup_hid_LUT(void);       // also for special_hid_LUT
void setup_special_hid_LUT(char orient);

#endif