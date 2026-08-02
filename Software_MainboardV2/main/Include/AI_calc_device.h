/*

ChatGPT Hardware Hack for caluclators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

device.h: Hardware settings

*/

#ifndef DEVICE_H
#define DEVICE_H

// Includes

#include "driver/i2c_master.h"





// Device Typedef, used to store settings and information related to the hardware

typedef struct{
    uint8_t debug_mode;
    uint8_t main_display_contrast;
    uint16_t bms_cell_millivolts;
    uint8_t bms_cell_temp;
}device_TypeDef;



// Extern device variables

extern i2c_master_bus_handle_t i2c_bus;

extern device_TypeDef device;



// GPIO-Defines and GPIO-state-defines

#define ESP_N_POWERLATCH 4

#define MAIN_DISPLAY_N_RESET 8

#define TCA8418_N_INTERRUPT 16
#define TCA8418_N_RESET 17

#define SIDE_DISPLAY_N_RESET 18

#define CAMERA_POWER_ENABLE 38

#define BMS_ADC_THERM 39

#define FREEGPIO_8 40
#define FREEGPIO_7 41
#define FREEGPIO_6 42
#define FREEGPIO_5 43
#define FREEGPIO_4 44





// I2C-Defines

#define SDA 7
#define SCL 15
#define I2C_FREQ 400000             // in Hz, supported by all peripherals






// Exported functions

void powerlatch_shutdown(void);
void device_init(void);
void get_battery_info(void);

#endif