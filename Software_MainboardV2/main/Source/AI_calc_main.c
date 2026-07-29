/*

ChatGPT Hardware Hack for caluclators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

main.c: App_main

*/

// Includes

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

// Includes only for debugging start
#include "esp_log.h"
#include "esp_timer.h"
// Includes only for debuggin end

#include "AI_calc_main.h"
#include "AI_calc_UI.h"
#include "AI_calc_keypad.h"







void app_main(void){

    gpios_init();
    free_gpios_init();
    gpio_set_level(ESP_N_POWERLATCH,0);     // Activate powerlatch, keep ESP32S3 Enable high
    gpios_set_default();
    i2c_bus_init();

    vTaskDelay(pdMS_TO_TICKS(100));         // Wait for stable signal on ESP32 Enable

    tca8418_init_keypad();
    tca8418_init_gpios();

    // Debuggincode start
    while(1){
        if(!gpio_get_level(TCA8418_N_INTERRUPT)){
            update_pressed_keys();
            // Check shutdown condition
            if ((cur_pressed_keys[0].special_function == KEY_SHIFT_SPECIAL_FUNC) && (cur_pressed_keys[1].special_function == KEY_MENU_SPECIAL_FUNC)){
                gpio_set_level(ESP_N_POWERLATCH,1);
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
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    // Debuggingcode end
}
