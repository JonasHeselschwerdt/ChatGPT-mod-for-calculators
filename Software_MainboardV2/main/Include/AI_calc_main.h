/*

ChatGPT Hardware Hack for caluclators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

main.h: Project config settings

*/

#ifndef MAIN_H
#define MAIN_H





// Includes






// GPIO-Defines and GPIO-state-defines

#define ESP_N_POWERLATCH 4

#define MAIN_DISPLAY_N_RESET 8

#define TCA8418_N_INTERRUPT 16
#define TCA8418_N_RESET 17

#define SIDE_DISPLAY_N_RESET 18

#define CAMERA_POWER_ENABLE 38

#define FREEGPIO_9 39
#define FREEGPIO_8 40
#define FREEGPIO_7 41
#define FREEGPIO_6 42
#define FREEGPIO_5 43
#define FREEGPIO_4 44






// I2C-Defines

#define SDA 7
#define SCL 15
#define I2C_FREQ 400000             // in Hz, supported by all peripherals




// Software- Version Information

#define SOFTWARE_VERSION_STRING " Prototype v.2.0.0  "      // Should have the same length as MAIN_DISPLAY_COLUMNS!









#endif