/*

ChatGPT Hardware Hack for caluclators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

UI.h: UI-Settings, Variable Types, extern Variables and Functions

*/

#ifndef UI_H
#define UI_H






// Includes

#include "driver/i2c_master.h"



// Display dimension defines

#define MAIN_DISPLAY_ROWS 4
#define MAIN_DISPLAY_COLUMNS 20





// Page length defines

#define MAX_SCRIBBLE_PAGE_SECTORS 6
#define SCRIBBLE_PAGE_LENGTH ((MAX_SCRIBBLE_PAGE_SECTORS * MAIN_DISPLAY_COLUMNS * MAIN_DISPLAY_ROWS) - MAIN_DISPLAY_COLUMNS)

#define MAX_ANSWER_PAGE_SECTORS 60
#define ANSWER_PAGE_LENGHT (MAX_ANSWER_PAGE_SECTORS * MAIN_DISPLAY_COLUMNS * MAIN_DISPLAY_ROWS)





// UI-Task Defines

#define UI_LOOP_DELAYTIME 10        // in ms
#define UI_LOOPS_PER_MIN ((1000 / UI_LOOP_DELAYTIME) * 60)






// UI-Unlock Code in calculator mode

#define UI_UNLOCK_CODE "02-04-2004"





// Menu-Defines

#define NO_MENU_SELECTED 0



// Type-Definitions

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

} html_mode_typeDef;





// Extern global Variables

extern i2c_master_bus_handle_t i2c_bus;
extern char UI_mode;





// Exported functions

void i2c_bus_init(void);
void gpios_init(void);
void gpios_set_default(void);
void free_gpios_init(void);




#endif

