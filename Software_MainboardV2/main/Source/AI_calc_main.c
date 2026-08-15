/*

ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

main.c: App_main

*/

// Includes

#include <stdio.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "AI_calc_main.h"
#include "AI_calc_device.h"
#include "AI_calc_UI.h"
#include "AI_calc_keypad.h"
#include "AI_calc_maindisplay.h"
#include "AI_calc_battery.h"
#include "AI_calc_sidedisplay.h"




// The code below is only for reference on how to use functions and variables
// The UI is not ready yet

void app_main(void){

    device_init();

    if(device.debug_mode){
        dep128064_start_screensaver(100);
        // Debuggincode start Display
        char* debug_text[MAIN_DISPLAY_ROWS]={
            "                    ",
            SOFTWARE_VERSION_STRING,
            DEVICE_STATUS_STRING,
            "                    "    
        };
        dogm204_print_screen(debug_text);
        vTaskDelay(pdMS_TO_TICKS(2000));
        char *debug_message[MAIN_DISPLAY_ROWS] = {
            "====================",
            "    Created by      ",
            " @ElectrJonics on YT",
            "===================="
        };
        dogm204_print_screen(debug_message);
        vTaskDelay(pdMS_TO_TICKS(2000));
        char* legal_notice[MAIN_DISPLAY_ROWS] = {
            " Independant Mod:   ",
            " Not affiliated     ",
            " with Casio Computer",
            " Co.,LTD            "
        };
        dogm204_print_screen(legal_notice);
        vTaskDelay(pdMS_TO_TICKS(2000));
        dogm204_start_loading_screen(" Loading...         ", 150);
        vTaskDelay(pdMS_TO_TICKS(5000));
        dogm204_end_loading_screen();
        char* info_text2[MAIN_DISPLAY_ROWS] = {
            " You can find all   ",
            " design files for   ",
            " this on my GitHub @",
            " JonasHeselschwerdt "
        };
        dogm204_print_screen(info_text2);
        vTaskDelay(pdMS_TO_TICKS(2000));
        char* fancy_text[MAIN_DISPLAY_ROWS]={
            EMPTY_LINE,
            "  This is a fancy   ",
            " swiping animation  ",
            EMPTY_LINE
        };
        dogm204_print_screen_fancy(fancy_text,3000);
        dep128064_end_screensaver();
        dep128064_clear_screen();
        // Debuggingcode start Keypad
        while(1){
            vTaskDelay(pdMS_TO_TICKS(UI_LOOP_DELAYTIME));
            if(!gpio_get_level(TCA8418_N_INTERRUPT)){
                update_pressed_keys();
                // Check shutdown condition
                if ((cur_pressed_keys[0].special_function == KEY_SHIFT_SPECIAL_FUNC) && (cur_pressed_keys[1].special_function == KEY_MENU_SPECIAL_FUNC)){
                    vTaskDelay(pdMS_TO_TICKS(500));
                    powerlatch_shutdown();
                }
                // Press DEL Key to display battery stats
                if (cur_pressed_keys[0].special_function == KEY_BACK_SPECIAL_FUNC){
                    char line1[MAIN_DISPLAY_COLUMNS+1];
                    char line2[MAIN_DISPLAY_COLUMNS+1];
                    char line3[MAIN_DISPLAY_COLUMNS+1];
                    char line4[MAIN_DISPLAY_COLUMNS+1];
                    char* info_screen[MAIN_DISPLAY_ROWS] = {
                        line1,
                        line2,
                        line3,
                        line4
                    };
                    if (get_battery_info() == ESP_FAIL){
                        powerlatch_shutdown();
                    }
                    create_bms_info_screen(info_screen);
                    char* explanation[MAIN_DISPLAY_ROWS] = {
                        "====================",
                        " Printing battery   ",
                        " Information        ",
                        "===================="
                    };
                    dogm204_print_screen(explanation);
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    dep128064_refresh_status_screen();
                    dogm204_print_screen(info_screen);
                    vTaskDelay(pdMS_TO_TICKS(2000));
                }
                // Log pressed keys
                uint64_t cur_time = esp_timer_get_time() / 1000;
                for (uint8_t i=0; i<10;i++){
                    if (cur_pressed_keys[i].normal_meaning != '\0'){
                        ESP_LOGI("Keypad","Key %d: %c | pressed since %llu ms",(i+1),cur_pressed_keys[i].normal_meaning, (cur_time - cur_pressed_keys[i].press_timestamp));
                    }
                    if (cur_pressed_keys[i].special_function == KEY_NOT_DEFINED){
                        break;
                    }
                }
            }
        }
        // Debuggingcode end
    }
    else{
        // Device main loop
    }
}
