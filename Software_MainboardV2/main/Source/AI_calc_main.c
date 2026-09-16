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



void app_main(void){
    
    // Device Init
    device_init();
    UI_init();
    Key_TypeDef cur_pressed_keys[10];
    for (uint8_t i=0; i<10; i++){
        cur_pressed_keys[i] = no_key;
    }
    // Device main loop
    while(1){
        vTaskDelay(pdMS_TO_TICKS(UI_LOOP_DELAYTIME));
        if(!gpio_get_level(TCA8418_N_INTERRUPT)){
            update_pressed_keys(cur_pressed_keys);
            // The entire UI is implemented in:
            UI_handle_pressed_keys(cur_pressed_keys);
            /*
            Parallel FreeRTOS Tasks:
            - wifi_manager in network.c
            - battery monitoring in battery.c
            - autooff timer monitoring and side display refreshing in UI.c
            */
        }
    }
}
