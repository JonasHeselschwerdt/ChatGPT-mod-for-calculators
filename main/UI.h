// ChatGPT Hardware Hack compatible with Casio FX Series

// UI.h -> Functions and variables to run the UI with the 4x20 DOGM204 LCD and the TCA8418

// © 2026 Jonas Heselschwerdt
// Licensed under CC BY-NC 4.0




#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "driver/i2c_master.h"

typedef struct{

    uint8_t x;
    uint8_t y;

} Cursor;

typedef struct{

    char opened_from;
    bool main_menu;         // false: menu closed true: menu opened
    uint8_t sub_menu;       // which sub_menu has been chosen
    uint8_t subsub_menu;    // which subsub_menu has been chosen
    uint8_t page;           // keeps track of the menu page

} Menu;


extern uint8_t scribble_page[scribble_page_length];
extern char UI_mode;
extern char answer_page[answer_page_length+1];
extern char extracted[scribble_page_length+1];

extern Menu menu;

extern uint16_t html_menu[9];
extern char url[64];
extern uint8_t html_text_passage;
extern char current_html_text[81];

extern uint8_t autooff_mins;
extern uint32_t autooff_timer;

void UI_init(void);
void update_keyregister(uint8_t* registerpointer, uint8_t* registerpointer_old);
void print_start_screen(Cursor cursor);
void handle_key_register_scribble(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor);
void setcursor(bool cursor, bool blink);
void insert_scribble_header(char scribble_mode);
void print_scribble_page(void);
void initialize_cursor(Cursor* cursor);
void handle_keyregister_menu(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor);
void handle_keyregister_html_view(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor);
void handle_keyregister_answer(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor);
void print_screen(const char* print_text, Cursor *cursor);
void handle_key_register_stealth(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor);
void moveCursor(char direction, Cursor *cursor);
void set_display_cursor(uint8_t x, uint8_t y);
void device_shutdown(Cursor cursor);
void handle_keyregister_keypad(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor);
void print_line(const char* print_text, uint8_t line, Cursor* cursor);
void clear_display(void);

extern char keypad_mode;


#endif