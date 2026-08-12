/*

ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

device.c: GPIO, I2C, and other connectivity

*/




// Includes

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"


#include "AI_calc_main.h"
#include "AI_calc_device.h"
#include "AI_calc_maindisplay.h"
#include "AI_calc_keypad.h"
#include "AI_calc_battery.h"
    




// Static variables

char* shutdown_text[MAIN_DISPLAY_ROWS] = {
    "====================",
    "  Device is         ",
    "  shuting down      ",
    "===================="
};






// Generic global variables

i2c_master_bus_handle_t i2c_bus = NULL;

device_TypeDef device = {
    // default values
    .debug_mode = 1,        
};






// Static function declarations
static void i2c_bus_init(void);

static void gpios_init(void);
static void gpios_set_default(void);
static void free_gpios_init(void);




// Static functions I2C

static void i2c_bus_init(void){

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






// Static functions GPIO

static void gpios_init(void){

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

    uint64_t disabled = 0;
    disabled |= (1ULL << BMS_NTC_VOLTAGE_DIV_ACT);
    gpio_config_t disable_config = {
        .pin_bit_mask = disabled,
        .mode = GPIO_MODE_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&disable_config);
}

static void gpios_set_default(void){

    // Turns off both displays, TCA8418, camera and turn on ESP powerlatch circuit
    gpio_set_level(ESP_N_POWERLATCH,0);
    gpio_set_level(TCA8418_N_RESET,0);
    gpio_set_level(SIDE_DISPLAY_N_RESET,0);
    gpio_set_level(MAIN_DISPLAY_N_RESET,0);
    gpio_set_level(CAMERA_POWER_ENABLE,0);
}

static void free_gpios_init(void){

    // Not used by default, configure as Inputs with internal Pullups
    uint64_t freegpios = 0;
    freegpios |= (1ULL << FREEGPIO_4);
    freegpios |= (1ULL << FREEGPIO_5);
    freegpios |= (1ULL << FREEGPIO_6);
    freegpios |= (1ULL << FREEGPIO_7);
    freegpios |= (1ULL << FREEGPIO_8);

    gpio_config_t freegpio_config = {
        .pin_bit_mask = freegpios,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&freegpio_config);
}






// Exported functions

void powerlatch_shutdown(void){

    // turns off device and informs user
    dogm204_print_screen(shutdown_text);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_set_level(ESP_N_POWERLATCH,1);
}

void powerlatch_shutdown_immediately(void){

    // turns off device immediately and without warning
    gpio_set_level(ESP_N_POWERLATCH,1);
}

void device_init(void){

    // GPIOs
    gpios_init();
    free_gpios_init();
    gpios_set_default();
    // ADC and I2C
    bms_temp_adc_init();
    i2c_bus_init();
    // Keypad and more GPIOs
    tca8418_init_keypad();
    tca8418_init_gpios();
    // BMS
    max17048init();
    // Main display
    dogm204_init();
    // Check battery condition after initializing components)
    vTaskDelay(pdMS_TO_TICKS(100));
    if (!battery_boot_ok()){
        powerlatch_shutdown();
    }
}
