/*

ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

UI.h: UI-Settings, Variable Types, extern Variables and Functions

*/

#ifndef UI_H
#define UI_H

// UI version
#define UI_VER_STRING "SWv2.0.0 for HWv2.0.1"


// Includes

#include "AI_calc_maindisplay.h"
#include "AI_calc_LLMs.h"
#include "AI_calc_keypad.h"

#include <stdint.h>





// Page length defines

#define MAX_SCRIBBLE_PAGE_SECTORS 6
#define SCRIBBLE_PAGE_LENGTH ((MAX_SCRIBBLE_PAGE_SECTORS * MAIN_DISPLAY_COLUMNS * MAIN_DISPLAY_ROWS) - MAIN_DISPLAY_COLUMNS)

#define MAX_ANSWER_PAGE_SECTORS 60
#define ANSWER_PAGE_LENGTH (MAX_ANSWER_PAGE_SECTORS * MAIN_DISPLAY_COLUMNS * MAIN_DISPLAY_ROWS)

#define TEXTINPUT_PAGE_LENGTH 280



// UI-Tasks Defines

#define UI_LOOP_DELAYTIME 20                // in ms (UI_LOOP = app_main whileloop)
#define UI_SIDETASK_LOOP_DELAYTIME 2000     // in ms







// Type-Definitions

// UI Modes
typedef enum{
    UI_MODE_CALCULATOR,     // device works like (basic) normal calculator
    UI_MODE_SCRIBBLE,       // for typing in prompts
    UI_MODE_TEXTINPUT,      // for typing in passwords, API keys, etc. (entered only from a menu)
    UI_MODE_CHATVIEW,       // for viewing AI conversations  
    UI_MODE_MENU,           // for changing settings, etc.
    UI_MODE_FILEVIEW        // viewing text files that have been saved to the device
}UI_mode_TypeDef;





// UI Typedef
typedef struct{
    uint8_t autooff_tresh_mins;                 // if 0: no autooff timer
    uint8_t delete_imgs_after_sending;          // boolean
    UI_mode_TypeDef UI_mode;                    // not saved in NVS
    ai_model_TypeDef current_ai_model;
    uint8_t cam_img_cnt;                        // not saved in NVS
    size_t file_dir_size;                       // not saved in NVS
    char openai_model_version[32];
    char gemini_model_version[32];
    char claude_model_version[32];
    char UI_version_string[32];                 // not saved in NVS
    char unlock_code[MAIN_DISPLAY_COLUMNS+1];
}UI_TypeDef;

/*
Warning: If you forget your unlock code, you will have to perform a factory reset manually
by running idf.py erase-flash and flash again
*/

// Default settings if NVS fails

#define UI_DEFAULT_AUTOOFFMINS 2
#define UI_DEFAULT_AI_MODEL AI_MODEL_OPENAI
#define UI_DEFAULT_DELETE_IMGS_AFTER_SENDING 1
#define UI_DEFAULT_OPENAI_MODEL "gpt-5.5"
#define UI_DEFAULT_GEMINI_MODEL ""                      // not implemented yet
#define UI_DEFAULT_CLAUDE_MODEL ""                      // not implemented yet
#define UI_DEFAULT_UNLOCK_CODE "02-04-2004          "   // has to have MAINDISPLAY_COLUMNS-1 chars and be right aligned






// Menu mode related

// 1 menu = Array of many menu_entry_TypeDef:

typedef struct menu_entry_TypeDef menu_entry_TypeDef;
struct menu_entry_TypeDef{
    char menu_title[MAIN_DISPLAY_COLUMNS];      // In a menu directory all entries should have the same title,fill with spaces if necessary 
    char entry_text[MAIN_DISPLAY_COLUMNS];      // text of entry, needs to have exactly MAIN_DISPLAY_COLUMNS-1 signs, fill with spaces if necessary
    menu_entry_TypeDef *menu_child;             // Pointer at menu-entry-array that gets opened through enter key
    menu_entry_TypeDef *menu_parent;            // Pointer at menu-entry-array that gets opened through back key
    void (*enter_callback)(void);               // Pointer at function that gets executed when the entry is opened with enter
    void (*left_callback)(void);                // Pointer at function that gets executed when the entry is selected and left key pressed
    void (*right_callback)(void);               // Pointer at function that gets executed when the entry is selected and right key pressed
};
#define MENU_END {"","",NO_CHILD,NO_PARENT,NO_ENTER_CB,NO_LEFT_CB,NO_RIGHT_CB}  // always has to be the last entry in a menu!

#define NO_CHILD NULL
#define NO_PARENT NULL
#define NO_ENTER_CB NULL
#define NO_LEFT_CB NULL
#define NO_RIGHT_CB NULL 





// Textinput mode related

typedef struct textinput_page_TypeDef textinput_page_TypeDef;
struct textinput_page_TypeDef{
    char command_prompt[MAIN_DISPLAY_COLUMNS +1];           // +1 for String-terminator
    uint8_t sensitive_information;                          // boolean
    menu_entry_TypeDef* return_to_menu;                     // textinput mode only accessed through menues, should not be empty
    textinput_page_TypeDef* goto_page;                      // if this exists, has higer priority than return_to_menu
    void (*enter_callback)(void);                           // executed when user presses enter, should not be empty
};

#define NO_SENSITIVE_INFO 0
#define SENSITIVE_INFO 1
#define NO_GOTO_PAGE NULL
#define NO_RETURN_TO_MENU NULL      // just for checking, do not assign, else cancelling textinputmode is not possible



// Different UI-Modes

typedef struct{
    char equation_page[MAIN_DISPLAY_COLUMNS + 1];           // +1 for String-terminator, always filled to the end with spaces during typing
    char solution[MAIN_DISPLAY_COLUMNS + 1];
    uint8_t calc_cursor_pos;
    uint8_t shift_active;                                   // boolean
    uint8_t equation_len;
    float previous_result;
    uint8_t previous_result_valid;                          // boolean
} calculator_mode_TypeDef;


typedef struct{
    char scribble_page[SCRIBBLE_PAGE_LENGTH + 1];           // +1 for String-terminator, always filled to the end with spaces during typing
    uint16_t scribble_page_length;
    uint16_t scribble_cursor_pos;
    uint8_t start_new_chat;                                 // boolean
} scribble_mode_typeDef;

typedef struct{
    char textinput_page[TEXTINPUT_PAGE_LENGTH +1];          // +1 for string terminator
    uint16_t textinput_cursor_pos;
    uint16_t textinput_page_length;
    textinput_page_TypeDef* current_page;
} text_input_mode_typeDef;

typedef struct{
    uint16_t current_chat_position;                         // 0-based
    uint16_t highest_chat_position;                         // 0-based
    uint8_t show_response;                                  // boolean, if 0 show prompt toggle with XOR operation
    char prompt_page[SCRIBBLE_PAGE_LENGTH+1];
    char response_page[ANSWER_PAGE_LENGTH+1];
    uint16_t current_item_section;
    uint16_t current_item_highest_section;                  // section = 1 screen of text
} chatview_mode_typeDef;

typedef struct{
    menu_entry_TypeDef* current_menu;           // Points at the array of menu_entry_TypeDef (= menu directory) that is currently opened
    uint8_t leave_directly_permitted;           // boolean, if 0 user can't escape this menu by pressing menu-key
    UI_mode_TypeDef opened_from_UI_mode;        // only relevant if leave_directly_permitted = 1
    uint8_t menu_cursor_pos;
} menu_mode_typeDef;

#define LEAVE_WITH_MENU_KEY_ALLOWED 1
#define LEAVE_WITH_MENU_KEY_DISABLED 0

/*
typedef struct{

} fileview_mode_typeDef;
*/




// Chatting restrictions

#define MAX_CONVERSATION_LENGTH 20
/*
max amount of prompt / response ID pairs in the metadata
After MAX_CONVERSATION_LENGTH text exchanges the user has to start a new chat!
Flash memory needed in worst case = MAX_CONVERSATION_LENGTH * (ANSWER_PAGE_LENGTH + SCRIBBLE_PAGE_LENGTH)
*/
 




// Exported functions

void UI_init(void);
void UI_handle_pressed_keys(Key_TypeDef* cur_pressed_keys);
void get_UI_state(UI_TypeDef* ui);



#endif

