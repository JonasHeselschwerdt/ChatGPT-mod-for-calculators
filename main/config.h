// ChatGPT Hardware Hack compatible with Casio FX Series

// config.h -> Non-user-changeable Settings and Pin definitions for all .c files

// © 2026 Jonas Heselschwerdt
// Free for personal, research and educational use
// Commercial use requires written permission




#ifndef CONFIG_H
#define CONFIG_H

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

#define answer_page_length 480
#define max_pages_answer 5

// Menu Settings

#define menu_pages_main 1        
#define sub_menu_count_main 6    

#define menu_pages_wifi 1        
#define sub_menu_count_wifi 7    

#define menu_pages_models 0      
#define sub_menu_count_models 1

// General Network settings

#define WIFI_NAMESPACE "wifi"
#define MAX_WIFI_ENTRIES 8
#define API_NAMESPACE "app"
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64

#endif

