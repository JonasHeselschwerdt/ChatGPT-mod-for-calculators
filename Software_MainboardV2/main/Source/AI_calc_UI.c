/*

ChatGPT Hardware Hack for caluclators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

UI.c: UI-Functions and Variables

*/

// Includes

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "AI_calc_main.h"
#include "esp_log.h"

#include "AI_calc_UI.h"





// Generic variables

i2c_master_bus_handle_t i2c_bus = NULL;




// Scribble Mode variables







// Answer Mode variables







// Menu Mode UI variables and constants

const char *main_menu[7] = {
    " Edit WIFI List     ",
    " Edit OpenAI API Key",
    " GPT-Model Selection",
    " Enter Keypad Mode  ",
    " Autooff-Timer      ",
    " Factory Reset      ",
    " Firmware Info      "
};







// Generic extern UI-Functions

void i2c_bus_init(void){

    i2c_master_bus_config_t i2c_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA,
        .scl_io_num = SCL,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false       // external resistors R27:28 on board
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, &i2c_bus));
}

void gpios_init(void){

    uint64_t outputs = 0;
    outputs |= (1ULL << ESP_N_POWERLATCH);
    outputs |= (1ULL << MAIN_DISPLAY_N_RESET);
    outputs |= (1ULL << TCA8418_N_RESET);
    outputs |= (1ULL << SIDE_DISPLAY_N_RESET);
    outputs |= (1ULL << CAMERA_POWER_ENABLE);

    gpio_config_t output_config = {
        .pin_bit_mask = outputs,
        .mode = GPIO_MODE_OUTPUT,
        // Pullups/Pulldowns externally on board (R35:37 + R19)
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&output_config);

    uint64_t inputs = 0;
    inputs |= (1ULL << TCA8418_N_INTERRUPT);

    gpio_config_t input_config = {
        .pin_bit_mask = inputs,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&input_config);
}

void gpios_set_default(void){

    // Turns off both displays, TCA8418 and camera
    gpio_set_level(TCA8418_N_RESET,0);
    gpio_set_level(SIDE_DISPLAY_N_RESET,0);
    gpio_set_level(MAIN_DISPLAY_N_RESET,0);
    gpio_set_level(CAMERA_POWER_ENABLE,0);
}

void free_gpios_init(void){

    // Not used by default, configure as Inputs with internal Pullups
    uint64_t freegpios = 0;
    freegpios |= (1ULL << FREEGPIO_4);
    freegpios |= (1ULL << FREEGPIO_5);
    freegpios |= (1ULL << FREEGPIO_6);
    freegpios |= (1ULL << FREEGPIO_7);
    freegpios |= (1ULL << FREEGPIO_8);
    freegpios |= (1ULL << FREEGPIO_9);

    gpio_config_t freegpio_config = {
        .pin_bit_mask = freegpios,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&freegpio_config);

}





// Scribble Mode functions







// Answer Mode functions







// Menu Mode functions