// ChatGPT Hardware Hack compatible with Casio FX Series

// Main.c -> Main Program file

// © 2026 Jonas Heselschwerdt
// Licensed under CC BY-NC 4.0




#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "config.h"
#include "UI.h"
#include "network.h"
#include "secret.h"

void app_main(void){

    // GPIO Setup (see schematic)

    uint64_t pin_mask = 0;
    pin_mask |= (1ULL << Reset);
    pin_mask |= (1ULL << RS);
    pin_mask |= (1ULL << RW);
    pin_mask |= (1ULL << E);
    pin_mask |= (1ULL << D4);
    pin_mask |= (1ULL << D5);
    pin_mask |= (1ULL << D6);
    pin_mask |= (1ULL << D7);
    pin_mask |= (1ULL << PE);
    pin_mask |= (1ULL << KeyEn);
    pin_mask |= (1ULL << diagnose);

    gpio_config_t io_conf = {
        .pin_bit_mask = pin_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Power latch and flash diagnostics LED

    gpio_set_level(PE,1);
    gpio_set_level(diagnose,1);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(diagnose,0);

    // Initialize Hardware

    UI_init();
    setcursor(0,1);

    uint8_t keyregister[10];
    uint8_t keyregister_old[10];
    for (int i = 0; i<10 ;i++) keyregister[i] = 0x00;
    for (int i = 0; i<10 ;i++) keyregister_old[i] = 0x00;

    Cursor cursor = {0,0};

    print_start_screen(cursor);

    nvs_flash_init();
    wifi_credentials_nvs_init();
    preset_test_credentials();
    wifi_manager_start();

    // Initialize scribble page, set UI_mode to scribble mode
    // First 20 characters show the scribble_mode header

    UI_mode = 's';

    for (int i=20;i < scribble_page_length; i++){

        scribble_page[i] = 0x20;  // Space characters everywhere
    }

    insert_scribble_header('c'); // c = command (edit prompt mode)

    // Initialize answer page (without printing)

    for (int i=0;i < answer_page_length; i++){

        answer_page[i] = ' ';   // Space characters everywhere
    }
    answer_page[answer_page_length] = '\0';  // Terminate string

    print_scribble_page();

    // Make sure the cursor starts in line 1! not in line 0 (reserved for scribble page header)

    initialize_cursor(&cursor);
    setcursor(1,1);
    
    // Main Loop:

    while (true){  

        while(UI_mode == 's'){

            // Scribble Mode main loop

            vTaskDelay(pdMS_TO_TICKS(10));

            if (gpio_get_level(KeyInt) == 0){

                update_keyregister(keyregister,keyregister_old);
                handle_key_register_scribble(keyregister,keyregister_old,&cursor);
                
                /*/ Debugging: Menu Logging

                ESP_LOGI("Menustate", "Menu state:");
                ESP_LOGI("Menustate", "  opened_from = %c", menu.opened_from);
                ESP_LOGI("Menustate", "  main_menu   = %s", menu.main_menu ? "true" : "false");
                ESP_LOGI("Menustate", "  sub_menu    = %u", menu.sub_menu);
                ESP_LOGI("Menustate", "  subsub_menu = %u", menu.subsub_menu);
                ESP_LOGI("Menustate", "  page        = %u", menu.page);

                /*/

            }
        }

        while(UI_mode == 'a'){

            // Answer Mode main loop

            vTaskDelay(pdMS_TO_TICKS(10));

            if(gpio_get_level(KeyInt) == 0){

                update_keyregister(keyregister,keyregister_old);
                handle_keyregister_answer(keyregister,keyregister_old,&cursor);

                /*/ Debugging: Menu Logging

                ESP_LOGI("Menustate", "Menu state:");
                ESP_LOGI("Menustate", "  opened_from = %c", menu.opened_from);
                ESP_LOGI("Menustate", "  main_menu   = %s", menu.main_menu ? "true" : "false");
                ESP_LOGI("Menustate", "  sub_menu    = %u", menu.sub_menu);
                ESP_LOGI("Menustate", "  subsub_menu = %u", menu.subsub_menu);
                ESP_LOGI("Menustate", "  page        = %u", menu.page);

                /*/
            }
        }
        
        while(UI_mode == 'm'){

            // Menu Mode main loop
            // Simple menus where you can choose between options

            vTaskDelay(pdMS_TO_TICKS(10));

            if(gpio_get_level(KeyInt) == 0){

                update_keyregister(keyregister,keyregister_old);
                handle_keyregister_menu(keyregister,keyregister_old,&cursor); 

                /*/ Debugging: Menu Logging

                ESP_LOGI("Menustate", "Menu state:");
                ESP_LOGI("Menustate", "  opened_from = %c", menu.opened_from);
                ESP_LOGI("Menustate", "  main_menu   = %s", menu.main_menu ? "true" : "false");
                ESP_LOGI("Menustate", "  sub_menu    = %u", menu.sub_menu);
                ESP_LOGI("Menustate", "  subsub_menu = %u", menu.subsub_menu);
                ESP_LOGI("Menustate", "  page        = %u", menu.page);

                /*/
            }

            

        }
        
    }

}
