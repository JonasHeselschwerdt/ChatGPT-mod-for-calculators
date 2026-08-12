/*

ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

UI.h: UI-Settings, Variable Types, extern Variables and Functions

*/

#ifndef UI_H
#define UI_H

// Includes

#include "AI_calc_maindisplay.h"

#include <stdint.h>




// Page length defines

#define MAX_SCRIBBLE_PAGE_SECTORS 6
#define SCRIBBLE_PAGE_LENGTH ((MAX_SCRIBBLE_PAGE_SECTORS * MAIN_DISPLAY_COLUMNS * MAIN_DISPLAY_ROWS) - MAIN_DISPLAY_COLUMNS)

#define MAX_ANSWER_PAGE_SECTORS 60
#define ANSWER_PAGE_LENGHT (MAX_ANSWER_PAGE_SECTORS * MAIN_DISPLAY_COLUMNS * MAIN_DISPLAY_ROWS)





// UI-Task Defines

#define UI_LOOP_DELAYTIME 20        // in ms
#define UI_LOOPS_PER_MIN ((1000 / UI_LOOP_DELAYTIME) * 60)






// UI-Unlock Code in calculator mode

#define UI_UNLOCK_CODE "02-04-2004"





// Menu-Defines

#define NO_MENU_SELECTED 0



// Type-Definitions

typedef enum{
    UI_MODE_SCRIBBLE,
    UI_MODE_ANSWER,
    UI_MODE_MENU,
    UI_MODE_FILEVIEW
}UI_mode_TypeDef;

typedef struct{
    uint8_t autooff_tresh_mins;
    UI_mode_TypeDef UI_mode;
}UI_TypeDef;

typedef struct{
    char scribble_page[SCRIBBLE_PAGE_LENGTH + 1];           // +1 for String-terminator
    char scribble_page_backup[SCRIBBLE_PAGE_LENGTH + 1];    // +1 for String-terminator
    uint8_t current_scribble_page_sector;
    char scribble_page_header[MAIN_DISPLAY_COLUMNS];
    uint16_t scribble_cursor_pos;
} scribble_mode_typeDef;

typedef struct{
    char answer_page[ANSWER_PAGE_LENGHT + 1];               // +1 for String-terminator
    uint8_t current_answer_page_sector;
} answer_mode_typeDef;

typedef struct{
    char opened_from;
    uint8_t main_menu_selected;                             // option in main menu that was selected
    uint8_t sub_menu_selected;                              // option that was selected in sub menu
    uint8_t subsub_menu_selected;                           // option that was selected in sub sub menu
    uint8_t menu_cursor_pos;
} menu_mode_typeDef;

typedef struct{

} fileview_mode_typeDef;





// Extern global Variables

extern UI_TypeDef UI;




// Exported functions





#endif

