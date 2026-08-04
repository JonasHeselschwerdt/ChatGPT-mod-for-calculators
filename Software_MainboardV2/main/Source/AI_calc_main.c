/*

ChatGPT Hardware Hack for caluclators: Software V2

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







void app_main(void){

    device_init();

    if(device.debug_mode){
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
        dogm204_print_message(debug_message,2000);
        vTaskDelay(pdMS_TO_TICKS(2000));
        dogm204_start_loading_screen(" Loading...         ", 150);
        vTaskDelay(pdMS_TO_TICKS(5000));
        dogm204_end_loading_screen();
        dogm204_print_screen(debug_text);       
        vTaskDelay(pdMS_TO_TICKS(2000));
        char* fancy_text[MAIN_DISPLAY_ROWS]={
            EMPTY_LINE,
            "  This is a fancy   ",
            " swiping animation  ",
            EMPTY_LINE
        };
        dogm204_print_screen_fancy(fancy_text,3000);
        // Debuggingcode start Keypad
        while(1){
            if(!gpio_get_level(TCA8418_N_INTERRUPT)){
                update_pressed_keys();
                // Check shutdown condition
                if ((cur_pressed_keys[0].special_function == KEY_SHIFT_SPECIAL_FUNC) && (cur_pressed_keys[1].special_function == KEY_MENU_SPECIAL_FUNC)){
                    powerlatch_shutdown();
                }
                // Press DEL Key to show GPI states
                if (cur_pressed_keys[0].special_function == KEY_BACK_SPECIAL_FUNC){
                    ESP_LOGI("GPI-States","STAT1: %d | STAT2: %d | PG: %d | ALRT: %d",tca8418_gpi_get_level(BMS_STAT1),tca8418_gpi_get_level(BMS_STAT2),tca8418_gpi_get_level(BMS_PG),tca8418_gpi_get_level(BMS_ALRT));
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
            vTaskDelay(pdMS_TO_TICKS(UI_LOOP_DELAYTIME));
        }
        // Debuggingcode end
    }
    else{
        // Device main loop
    }
}
