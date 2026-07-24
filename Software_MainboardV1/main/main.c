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
#include "driver/rtc_io.h"

#include "config.h"
#include "UI.h"
#include "network.h"
#include "secret.h"
#include "keyset.h"

void app_main(void){

    // GPIO Setup (see schematic)

    uint64_t pin_mask = 0;
    pin_mask |= (1ULL << Reset);
    pin_mask |= (1ULL << RS);
    pin_mask |= (1ULL << RW);
    pin_mask |= (1ULL << E);
    pin_mask |= (1ULL << PE);
    pin_mask |= (1ULL << D4);
    pin_mask |= (1ULL << D5);
    pin_mask |= (1ULL << D6);
    pin_mask |= (1ULL << D7);
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

    // Power Latch

    gpio_set_level(PE,1);

    nvs_flash_init();

    // Did the device reboot to start HID Mode?
    // If yes initialize USB HID device (needs to be done quickly after booting)

    char boot_mode_buf[16];
    settings_get_str("Boot_Mode",boot_mode_buf,sizeof(boot_mode_buf));

    if (boot_mode_buf[0] == 'u'){
        hid_mode_start();
    }

    // Initialize Hardware

    UI_init();

    uint8_t keyregister[10];
    uint8_t keyregister_old[10];
    for (int i = 0; i<10 ;i++) keyregister[i] = 0x00;
    for (int i = 0; i<10 ;i++) keyregister_old[i] = 0x00;

    Cursor cursor = {0,0};
    initialize_cursor(&cursor);
    setcursor(1,1);

    set_display_cursor(0, 0);       // Initial curser coordinates in stealth mode

    wifi_credentials_nvs_init();
    preset_test_credentials();
    wifi_manager_start();

    littlefs_init();

    if (boot_mode_buf[0] == 'u'){

        UI_mode = 'k';
        clear_display();
        print_line("Device in USB Keypad",1,&cursor);
        print_line("mode, [Menu] = Leave",2,&cursor);
        setcursor(0,1);
        keypad_mode = 'u';

    }
    else{
        UI_mode = 'c';      // Start out in stealth mode
    }


    // Initialize scribble page without printing

    for (int i=20;i < scribble_page_length; i++){
        scribble_page[i] = 0x20;  // Space characters everywhere
    }

    // Initialize answer page (without printing)

    for (int i=0;i < answer_page_length; i++){
        answer_page[i] = ' ';   // Space characters everywhere
    }
    answer_page[answer_page_length] = '\0';  // Terminate string

    // Load auto save setting from NVS

    char autooff_char_buf[8];
    if (settings_get_str("autooff",autooff_char_buf,sizeof(autooff_char_buf)) != ESP_OK){
        autooff_mins = 2;
    }
    else{
        autooff_mins = (uint8_t)atoi(autooff_char_buf);
        if (autooff_mins == 0){
            autooff_mins = 2;
        }
    }

    // Main Loop: (USB HID mode)

    while(boot_mode_buf[0] == 'u'){

        // Device stuck in USB HID Mode until shutdown or restart back to normal mode!

        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAYTIME));

        if (autooff_timer > (LOOPS_PER_MINUTE * autooff_mins)){

            device_shutdown(cursor);
        }

        if (gpio_get_level(KeyInt) == 0){

            autooff_timer = 0;
            update_keyregister(keyregister,keyregister_old);
            handle_keyregister_keypad(keyregister,keyregister_old,&cursor);
        }

        else{

            autooff_timer++;
        }

    }

    // Main Loop (normal mode):

    while (boot_mode_buf[0] != 'u'){  

        while(UI_mode == 'c'){

            // Stealth mode main loop

            vTaskDelay(pdMS_TO_TICKS(LOOP_DELAYTIME));

            if (autooff_timer > (LOOPS_PER_MINUTE * autooff_mins)){
                device_shutdown(cursor);
            }

            if (gpio_get_level(KeyInt) == 0){

                autooff_timer = 0;
                update_keyregister(keyregister,keyregister_old);
                handle_key_register_stealth(keyregister,keyregister_old,&cursor);
            }
            else{
                autooff_timer++;
            }

        }

        while(UI_mode == 's'){

            // Scribble Mode main loop

            vTaskDelay(pdMS_TO_TICKS(LOOP_DELAYTIME));

            if (autooff_timer > (LOOPS_PER_MINUTE * autooff_mins)){

                device_shutdown(cursor);
                
            }

            if (gpio_get_level(KeyInt) == 0){

                autooff_timer = 0;
                update_keyregister(keyregister,keyregister_old);
                handle_key_register_scribble(keyregister,keyregister_old,&cursor);

            }
            else{
                autooff_timer++;
            }
        }

        while(UI_mode == 'a'){

            // Answer Mode main loop

            vTaskDelay(pdMS_TO_TICKS(LOOP_DELAYTIME));

            if (autooff_timer > (LOOPS_PER_MINUTE * autooff_mins)){

                device_shutdown(cursor);
                
            }

            if(gpio_get_level(KeyInt) == 0){

                autooff_timer = 0;
                update_keyregister(keyregister,keyregister_old);
                handle_keyregister_answer(keyregister,keyregister_old,&cursor);
            }
            else{
                autooff_timer++;
            }

        }
        
        while(UI_mode == 'm'){

            // Menu Mode main loop
            // Simple menus where you can choose between options

            vTaskDelay(pdMS_TO_TICKS(LOOP_DELAYTIME));

            if (autooff_timer > (LOOPS_PER_MINUTE * autooff_mins)){

                device_shutdown(cursor);
                
            }

            if(gpio_get_level(KeyInt) == 0){

                autooff_timer = 0;
                update_keyregister(keyregister,keyregister_old);
                handle_keyregister_menu(keyregister,keyregister_old,&cursor); 
            }
            else{
                autooff_timer++;
            }

        }

        while(UI_mode == 'h'){

            // HTML viewer mode main loop

            vTaskDelay(pdMS_TO_TICKS(LOOP_DELAYTIME));

            if (autooff_timer > (LOOPS_PER_MINUTE * autooff_mins)){

                device_shutdown(cursor);
                
            }

            if(gpio_get_level(KeyInt) == 0){

                autooff_timer = 0;
                update_keyregister(keyregister,keyregister_old);
                handle_keyregister_html_view(keyregister,keyregister_old,&cursor); 
            }
            else{
                autooff_timer++;
            }
        
        
        }

        while(UI_mode == 'k'){

            // Keypad mode main loop (HERE BLUETOOTH ONLY, USB IN SEPERATE LOOP)

            vTaskDelay(pdMS_TO_TICKS(10));

            if (autooff_timer > (LOOPS_PER_MINUTE * autooff_mins)){

                device_shutdown(cursor);
            }

            if (gpio_get_level(KeyInt) == 0){

                autooff_timer = 0;
                update_keyregister(keyregister,keyregister_old);
                handle_keyregister_keypad(keyregister,keyregister_old,&cursor);
            }

            else{

                autooff_timer++;
            }
        }

    }

}
