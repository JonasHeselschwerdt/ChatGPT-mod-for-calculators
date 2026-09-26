// ChatGPT Hardware Hack compatible with Casio FX Series

// config.h -> Non-user-changeable Settings and Pin definitions for all .c files

// © 2026 Jonas Heselschwerdt
// Licensed under CC BY-NC 4.0




#ifndef CONFIG_H
#define CONFIG_H

// Define firmware version:

#define firmware_version " Prototype v.1.1.0  "     // Make sure this string has exactly 20 chars!

// Main loop delay time with no user activity

#define LOOP_DELAYTIME 10    // in ms
#define LOOPS_PER_MINUTE ((1000 / LOOP_DELAYTIME) * 60)

// Define GPIO Pin Numbers

#define Reset 6
#define RS 7
#define RW 15
#define E 16
#define D4 17
#define D5 18
#define D6 14
#define D7 21

#define PE 4
#define diagnose 5

#define KeyInt 47
#define KeyEn 48
#define I2C_MASTER_SDA_IO  8
#define I2C_MASTER_SCL_IO  9

// Define I2C Settings

#define TCA8418_ADDR       0x34
#define I2C_MASTER_FREQ_HZ 100000

// Define Special Signs (use for lcd_charset as list index)

#define ArrowR 128
#define ArrowL 129
#define Backarrow 130
#define Fullblock 131
#define Circle 132
#define scharfS 133

// Settings for how much the user can write in scribble mode
// max_pages_scribble has to be (scribble_page_length / 80) - 1)

#define scribble_page_length 480
#define max_pages_scribble 5

// Settings for limitations in answer mode
// max_pages_answer = (answer_page_length / 80) - 1

#define answer_page_length 4800
#define max_pages_answer 59

// Menu Settings

#define menu_pages_main 1        
#define sub_menu_count_main 6    

#define menu_pages_wifi 1        
#define sub_menu_count_wifi 7    

#define menu_pages_models 0      
#define sub_menu_count_models 1

#define menu_pages_autooff 0
#define sub_menu_count_autooff 3

#define menu_pages_keypad 0
#define sub_menu_count_keypad 2

#define menu_pages_keypad_orient 0
#define sub_menu_count_keypad_orient 1

#define menu_pages_reset 0
#define sub_menu_count_reset 1

// HTML Viewer settings

#define max_sections_per_passage_html 1000
#define HTML_FILE "/littlefs/index.html"

// Stealth mode settings

#define UNLCK1 'e'  // Unlock key 1
#define UNLCK2 'r'  // Unlock key 2

// Modifier Bytes USB HID

#define MOD_SHIFT 0x02
#define MOD_ALTGR 0x40
#define MOD_NONE  0x00

// General Network settings

#define WIFI_NAMESPACE "wifi"
#define MAX_WIFI_ENTRIES 8
#define API_NAMESPACE "app"
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64

#define SETTINGS_NAMESPACE "settings"

#define BLE_NAME "MODFX-87-TEST-0"
#define BLE_NAME_LABELED "Name:" BLE_NAME

#endif

