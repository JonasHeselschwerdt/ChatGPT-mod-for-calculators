/*

ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

sidedisplay.h: Settings for sidedisplay (OLED-graphic display)

*/

/*

This Code uses the U8G2 library, to get it to compile paste the 'csrc' directory 
of https://github.com/olikraus/u8g2 into components/u8g2

*/

#ifndef SIDEDIS_H
#define SIDEDIS_H

// Includes

#include "AI_calc_battery.h"
#include "AI_calc_network.h"
#include "AI_calc_UI.h"



// I2C-Address

#define DEP128064_I2C_ADDR 0x3D




// Settings

#define SIDEDISPLAY_CONTRAST_INTERVAL 5
#define SIDEDISPLAY_MAX_CONTRAST 255


// DEP128064 Commands and Bitmasks for commands
// Commands that are only used once during init are not specifically defined

#define DEP128064_DISPLAYONOFF_CTRL 0xAE
#define DEP128064_ON 0x01
#define DEP128064_OFF 0x00

#define DEP128064_CONTRAST_SET 0x81





// Display restrictions

#define SIDE_DISPLAY_MAX_FRAMERATE 20       // in Hz
#define SIDE_DISPLAY_MIN_CYCLETIME_MS  ((1/SIDE_DISPLAY_MAX_FRAMERATE)*1000)

#define SIDE_DISPLAY_PIX_X 128
#define SIDE_DISPLAY_PIX_Y 64





// Screensaver

#define SCREENSAVER_ACTIVE_BIT BIT1

typedef struct{
    uint8_t x;
    uint8_t y;
    int8_t speed_x;
    int8_t speed_y;
    uint8_t size_x;
    uint8_t size_y;
}screensaver_TypeDef;




// Device status screen

typedef struct{
    uint8_t battery_bars;                       // 0...5
    char battery_percentage[5];                 // XXX%
    uint8_t status_charging;                    // boolean
    uint8_t warning_low_bat;                    // boolean
    char battery_time_left[9];                  // XXhYYmin
    uint8_t battery_connected;                  // boolean
    uint8_t wifi_signal_waves;                  // 0...3
    char wifi_name[19];                         // if longer put ... at back
    char ai_name[11];                           // if longer put ... at back
    char ai_model_version[11];                  // if longer put ... at back
    char storage_used_percentage[9];            // XXX/100%
    char camera_storage_pictures[8];            // XXX/YYY
    char uimode_title[10];
}status_screen_TypeDef;





// Exported functions

void dep128064_init(void);
void dep128064_set_contrast(uint8_t contrast);
void dep128064_power_ctrl(uint8_t display_on);
void dep128064_clear_screen(void);
void dep128064_start_screensaver(uint16_t advance_interval);
void dep128064_end_screensaver(void);
void dep128064_refresh_status_screen(bms_typeDef* bms,wifi_manager_TypeDef* wifi, UI_TypeDef* ui);
void dep128064_show_XBM(uint8_t* XBM_buf, uint8_t x_size, uint8_t y_size);
void dep128064_show_info(char* info_text);

#endif