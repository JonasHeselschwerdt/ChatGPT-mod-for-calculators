/*

ChatGPT Hardware Hack for caluclators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

maindisplay.h: Functions and variables to control text LCDs via I2C

*/

// Includes

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

#include "AI_calc_main.h"
#include "AI_calc_device.h"
#include "AI_calc_UI.h"
#include "AI_calc_maindisplay.h"
#include "AI_calc_device.h"




// Static main display variables

static i2c_master_dev_handle_t dogm204_dev = NULL;

static uint8_t MainDisplay_LUT[256];

static EventGroupHandle_t loading_events;
static TaskHandle_t loading_task_handle;

static char loading_text[MAIN_DISPLAY_COLUMNS+1];
static char empty_line[MAIN_DISPLAY_COLUMNS+1] = EMPTY_LINE;
static char loading_progress_bar[MAIN_DISPLAY_COLUMNS+1] = {
    ' ',
    DOGM204_FULLBLOCK,
    DOGM204_FULLBLOCK,
    DOGM204_FULLBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    DOGM204_HOLLOWBLOCK,
    ' ',
    '\0'
};
static char* loading_screen[MAIN_DISPLAY_ROWS] = {
    empty_line,
    loading_text,
    loading_progress_bar,
    empty_line
}; 



// Global main display variables








// Static function declarations

static void dogm204_write_cmd(uint8_t cmd);
static void dogm204_write_data(uint8_t data);
static void dogm204_read_data(uint8_t* data);
static uint8_t dogm204_screen_text_valid(char** screen_text);
static void initialize_MainDisplayLUT(void);
static void dogm204_read_screen(uint8_t* screen_text);
static void dogm204_create_symbol(uint8_t* symbol_data, uint8_t cgram_addr);
static void define_custom_symbols(void);
static void loading_screen_task_init(void);
static void advance_loading_screen(void);
static void loading_screen_task(void* arg);



// Static main display functions

static void dogm204_write_cmd(uint8_t cmd){

    uint8_t buf[2] = {
        0x00,     
        cmd
    };
    ESP_ERROR_CHECK(i2c_master_transmit(dogm204_dev, buf, sizeof(buf), -1));
    // Give a little bit of time for command processing
    vTaskDelay(pdMS_TO_TICKS(5));
}

static void dogm204_write_data(uint8_t data){

    uint8_t buf[2] = {
        0x40,
        data
    };
    ESP_ERROR_CHECK(i2c_master_transmit(dogm204_dev, buf, sizeof(buf), -1));
}

static void dogm204_read_data(uint8_t* data){ 

    uint8_t command = 0x40;
    ESP_ERROR_CHECK(i2c_master_transmit(dogm204_dev, &command, 1, -1));
    uint8_t data_buf[3];
    ESP_ERROR_CHECK(i2c_master_receive(dogm204_dev, data_buf, 3, -1));
    *data = data_buf[2];
}

static uint8_t dogm204_screen_text_valid(char** screen_text){

    // Check if strings are valid (same length as MAIN_DISPLAY_COLUMNS)
    for (uint8_t i=0; i<MAIN_DISPLAY_ROWS; i++){
        if (strlen(screen_text[i]) < MAIN_DISPLAY_COLUMNS){
            dogm204_print_error_screen("STRING TO SHORT     ",ERROR_SOURCE_MAIN_DISPLAY);
            return 0;
        }
        if (strlen(screen_text[i]) > MAIN_DISPLAY_COLUMNS){
            dogm204_print_error_screen("STRING TO LONG      ",ERROR_SOURCE_MAIN_DISPLAY);
            return 0;
        }
    }
    return 1;
}

static void dogm204_read_screen(uint8_t* screen_text){

    dogm204_write_cmd(DOGM204_RETURN_HOME);
    dogm204_set_cursor_position(20,4);
    for (uint8_t i=1; i<=MAIN_DISPLAY_ROWS; i++){
        for (uint8_t j=1; j<=MAIN_DISPLAY_COLUMNS; j++){
            dogm204_set_cursor_position(j,i);
            dogm204_read_data(&screen_text[((i-1)*MAIN_DISPLAY_COLUMNS)+(j-1)]);
        }
    }
    /*
    For some reason, the cursor seems to get shifted before each DDRAM read, 
    because of this quirk of the SSD1803A the screen_text array needs to be 'rotated' by one
    */
    uint8_t temp_buf = screen_text[(MAIN_DISPLAY_COLUMNS*MAIN_DISPLAY_ROWS)-1];
    for (uint8_t i=((MAIN_DISPLAY_COLUMNS*MAIN_DISPLAY_ROWS)-1); i>0; i--){
        screen_text[i] = screen_text[i-1];
    }
    screen_text[0]=temp_buf;
}

static void initialize_MainDisplayLUT(void){

    for (uint16_t i=0; i<256; i++){
        MainDisplay_LUT[i] = 0x20;
    }

    // Big letters

    MainDisplay_LUT['A'] = 0x41;
    MainDisplay_LUT['B'] = 0x42;
    MainDisplay_LUT['C'] = 0x43;
    MainDisplay_LUT['D'] = 0x44;
    MainDisplay_LUT['E'] = 0x45;
    MainDisplay_LUT['F'] = 0x46;
    MainDisplay_LUT['G'] = 0x47;
    MainDisplay_LUT['H'] = 0x48;
    MainDisplay_LUT['I'] = 0x49;
    MainDisplay_LUT['J'] = 0x4A;
    MainDisplay_LUT['K'] = 0x4B;
    MainDisplay_LUT['L'] = 0x4C;
    MainDisplay_LUT['M'] = 0x4D;
    MainDisplay_LUT['N'] = 0x4E;
    MainDisplay_LUT['O'] = 0x4F;
    MainDisplay_LUT['P'] = 0x50;
    MainDisplay_LUT['Q'] = 0x51;
    MainDisplay_LUT['R'] = 0x52;
    MainDisplay_LUT['S'] = 0x53;
    MainDisplay_LUT['T'] = 0x54;
    MainDisplay_LUT['U'] = 0x55;
    MainDisplay_LUT['V'] = 0x56;
    MainDisplay_LUT['W'] = 0x57;
    MainDisplay_LUT['X'] = 0x58;
    MainDisplay_LUT['Y'] = 0x59;
    MainDisplay_LUT['Z'] = 0x5A;

    // Small letters

    MainDisplay_LUT['a'] = 0x61;
    MainDisplay_LUT['b'] = 0x62;
    MainDisplay_LUT['c'] = 0x63;
    MainDisplay_LUT['d'] = 0x64;
    MainDisplay_LUT['e'] = 0x65;
    MainDisplay_LUT['f'] = 0x66;
    MainDisplay_LUT['g'] = 0x67;
    MainDisplay_LUT['h'] = 0x68;
    MainDisplay_LUT['i'] = 0x69;
    MainDisplay_LUT['j'] = 0x6A;
    MainDisplay_LUT['k'] = 0x6B;
    MainDisplay_LUT['l'] = 0x6C;
    MainDisplay_LUT['m'] = 0x6D;
    MainDisplay_LUT['n'] = 0x6E;
    MainDisplay_LUT['o'] = 0x6F;
    MainDisplay_LUT['p'] = 0x70;
    MainDisplay_LUT['q'] = 0x71;
    MainDisplay_LUT['r'] = 0x72;
    MainDisplay_LUT['s'] = 0x73;
    MainDisplay_LUT['t'] = 0x74;
    MainDisplay_LUT['u'] = 0x75;
    MainDisplay_LUT['v'] = 0x76;
    MainDisplay_LUT['w'] = 0x77;
    MainDisplay_LUT['x'] = 0x78;
    MainDisplay_LUT['y'] = 0x79;
    MainDisplay_LUT['z'] = 0x7A;

    // Other ASCII Symbols

    MainDisplay_LUT['!'] = 0x21;
    MainDisplay_LUT['"'] = 0x22;
    MainDisplay_LUT['#'] = 0x23;
    MainDisplay_LUT['$'] = 0xA2;
    MainDisplay_LUT['%'] = 0x25;
    MainDisplay_LUT['&'] = 0x26;
    MainDisplay_LUT['\''] = 0x27;
    MainDisplay_LUT['('] = 0x28;
    MainDisplay_LUT[')'] = 0x29;
    MainDisplay_LUT['*'] = 0x2A;
    MainDisplay_LUT['+'] = 0x2B;
    MainDisplay_LUT[','] = 0x2C;
    MainDisplay_LUT['-'] = 0x2D;
    MainDisplay_LUT['.'] = 0x2E;
    MainDisplay_LUT['/'] = 0x2F;
    MainDisplay_LUT[':'] = 0x3A;
    MainDisplay_LUT[';'] = 0x3B;
    MainDisplay_LUT['<'] = 0x3C;
    MainDisplay_LUT['='] = 0x3D;
    MainDisplay_LUT['>'] = 0x3E;
    MainDisplay_LUT['?'] = 0x3F;
    MainDisplay_LUT['@'] = 0xA0;
    MainDisplay_LUT['['] = 0xFA;
    MainDisplay_LUT['\\'] = 0xFB;
    MainDisplay_LUT[']'] = 0xFC;
    MainDisplay_LUT['^'] = 0x1D;
    MainDisplay_LUT['_'] = 0xC4;
    MainDisplay_LUT['{'] = 0xFD;
    MainDisplay_LUT['}'] = 0xFF;
    MainDisplay_LUT['~'] = 0xDE;
    MainDisplay_LUT[' '] = 0x20;

    // Numbers

    MainDisplay_LUT['0'] = 0x30;
    MainDisplay_LUT['1'] = 0x31;
    MainDisplay_LUT['2'] = 0x32;
    MainDisplay_LUT['3'] = 0x33;
    MainDisplay_LUT['4'] = 0x34;
    MainDisplay_LUT['5'] = 0x35;
    MainDisplay_LUT['6'] = 0x36;
    MainDisplay_LUT['7'] = 0x37;
    MainDisplay_LUT['8'] = 0x38;
    MainDisplay_LUT['9'] = 0x39;

    // Other non-ASCII symbols

    MainDisplay_LUT[DOGM204_FULLBLOCK] = 0x1F;
}

static void dogm204_create_symbol(uint8_t* symbol_data, uint8_t cgram_addr){

    if (cgram_addr > 7){
        // Invalid CGRAM-ADDR
        dogm204_print_error_screen("INVALID CGRAM       ",ERROR_SOURCE_MAIN_DISPLAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
    }
    dogm204_write_cmd(DOGM204_FUNCTION_SET);
    for (uint8_t i=0; i<MAIN_DISPLAY_CHARACTERSIZE_Y; i++){
        dogm204_write_cmd(DOGM204_SET_CGRAM_ADDR|(cgram_addr<<3)|i);
        dogm204_write_data(symbol_data[i]);
    }

}

static void define_custom_symbols(void){

    // Create custom symbol (MAIN_DISPLAY_CHARACTERSIZE_X * MAIN_DISPLAY_CHARACTERSIZE_Y) 
    // for display progress bar
    uint8_t empty_prog_bar[8] = {
        // Bits 5-7 are irrelevant              Looks like:
        0b00011111,                     //      0   0   0   0   0    
        0b00010001,                     //      0               0
        0b00010001,                     //      0               0
        0b00010001,                     //      0               0
        0b00010001,                     //      0               0
        0b00010001,                     //      0               0
        0b00010001,                     //      0               0
        0b00011111                      //      0   0   0   0   0
    };
    MainDisplay_LUT[DOGM204_HOLLOWBLOCK] = 0x00;
    dogm204_create_symbol(empty_prog_bar,MainDisplay_LUT[DOGM204_HOLLOWBLOCK]);    
}

static void loading_screen_task_init(void){

    loading_events = xEventGroupCreate();
    xTaskCreate(loading_screen_task,"LoadingScreen",2048,NULL,2,&loading_task_handle);
}

static void loading_screen_task(void* arg){

    // Shows a loading screen while not blocking app_main
    while(1){
        xEventGroupWaitBits(loading_events,LOADING_ACTIVE_BIT,pdFALSE,pdTRUE,portMAX_DELAY);
        advance_loading_screen();
        vTaskDelay(pdMS_TO_TICKS(LOADING_SCREEN_ADVANCE_INTERVAL_MS));
    }
}

static void advance_loading_screen(void){

    uint8_t progress_bar_DDRAM_codes[MAIN_DISPLAY_COLUMNS-2];
    // Read current state of progress bar
    for (uint8_t i=2; i<MAIN_DISPLAY_COLUMNS; i++){
        // Set cursor x one cursor position in front of progressbar,
        // see dogm204_read_screen above for the reason
        dogm204_set_cursor_position(i-1,3);
        dogm204_read_data(&progress_bar_DDRAM_codes[i-2]);
    }
    // Rotate values of progress bar
    uint8_t temp_buf = progress_bar_DDRAM_codes[MAIN_DISPLAY_COLUMNS-3];
    for (uint8_t i=(MAIN_DISPLAY_COLUMNS - 3); i >= 1; i--) {
        progress_bar_DDRAM_codes[i] = progress_bar_DDRAM_codes[i - 1];
    }
    progress_bar_DDRAM_codes[0]=temp_buf;
    // Clear progress bar and update it
    dogm204_set_cursor_position(2,3);
    for (uint8_t i=0; i<(MAIN_DISPLAY_COLUMNS-2); i++){
        dogm204_write_data(MainDisplay_LUT[' ']);
    }
    dogm204_set_cursor_position(2,3);
    for (uint8_t i=0; i<(MAIN_DISPLAY_COLUMNS-2); i++){
        dogm204_write_data(progress_bar_DDRAM_codes[i]);
    }
}





// Exported main display functions

void dogm204_init(void){

    i2c_device_config_t dogm204_config = {
        .device_address = DOGM204_I2C_ADDR,
        .scl_speed_hz = I2C_FREQ
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus,&dogm204_config,&dogm204_dev));

    gpio_set_level(MAIN_DISPLAY_N_RESET,1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Init-Sequence start
    dogm204_write_cmd(DOGM204_FUNCTION_SET|DOGM204_RE_BIT);
    dogm204_write_cmd(DOGM204_EXT_FUNCTION_SET|DOGM204_FANCY_CURSOR_BIT);
    dogm204_write_cmd(DOGM204_ENTRY_MODE_SET|DOGM204_TOP_VIEW_BIT);
    dogm204_write_cmd(DOGM204_BIAS_SET);
    dogm204_write_cmd(DOGM204_FUNCTION_SET|DOGM204_IS_BIT);
    dogm204_write_cmd(DOGM204_INTERNAL_OSC);
    dogm204_write_cmd(DOGM204_FOLLOWER_CONTROL);
    dogm204_set_contrast(device.main_display_contrast);
    dogm204_write_cmd(DOGM204_FUNCTION_SET);
    // clear display befor turning display on, so no 'Junk data' shows up
    dogm204_clear_screen();
    dogm204_display_control(DOGM204_CURSOR_OFF_BIT|DOGM204_CURSOR_NO_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
    // Default cursor shift direction -> right
    dogm204_set_cursor_shift_dir(DOGM204_CURSOR_RIGHT_SHIFT);
    dogm204_write_cmd(DOGM204_FUNCTION_SET|DOGM204_RE_BIT);
    // Dual byte command start
    dogm204_write_cmd(DOGM204_ROM_SELECT);
    dogm204_write_data(DOGM204_ROM_SETTINGS|DOGM204_ROM_A_BIT);
    // Dual byte command end
    dogm204_write_cmd(DOGM204_FUNCTION_SET);
    // Init-Sequence end

    initialize_MainDisplayLUT();
    define_custom_symbols();

    loading_screen_task_init();
}







void dogm204_set_contrast(uint8_t contrast){

    // Contrast can be set from 0-63
    // IS Bit = 1, RE Bit = 0
    if (contrast > 0x3F){
        // contrast-value to high, set to max
        dogm204_write_cmd(DOGM204_FUNCTION_SET|DOGM204_IS_BIT);
        dogm204_write_cmd(DOGM204_POWER_CTRL|0x03);
        dogm204_write_cmd(DOGM204_CONTRAST_SET|0x0F);
    }
    else{
        dogm204_write_cmd(DOGM204_FUNCTION_SET|DOGM204_IS_BIT);
        dogm204_write_cmd(DOGM204_POWER_CTRL|(contrast >> 4));
        dogm204_write_cmd(DOGM204_CONTRAST_SET|(contrast & 0x0F));
    }
    // IS and RE Bits back to default
    dogm204_write_cmd(DOGM204_FUNCTION_SET); 
}

void dogm204_display_control(uint8_t display_settings){

    // IS Bit = 0, RE Bit = 0
    dogm204_write_cmd(DOGM204_FUNCTION_SET);
    dogm204_write_cmd(DOGM204_DISPLAY_ONOFF_CTRL|display_settings);
}

void dogm204_set_cursor_shift_dir(uint8_t dir){

    // IS Bit = 0, RE Bit = 0
    dogm204_write_cmd(DOGM204_FUNCTION_SET);
    dogm204_write_cmd(DOGM204_ENTRY_MODE_SET|dir);
    
}

void dogm204_set_cursor_position(uint8_t x, uint8_t y){

    /*
    Cursor coordinates are 1-based!
    x = 1 & y = 1 -> Top left corner of display
    x = MAIN_DISPLAY_COLUMNS & y = MAIN_DISPLAY_ROWS -> Bottom right corner of display
    */
    uint8_t display_ram_coords = 0x00;
    display_ram_coords = display_ram_coords + (0x20 * (y-1));
    display_ram_coords = display_ram_coords + (0x01 * (x-1));
    dogm204_write_cmd(DOGM204_SET_DDRAM_ADDR + display_ram_coords);
}









void dogm204_clear_screen(void){

    dogm204_write_cmd(DOGM204_CLEAR_SCREEN);
}

void dogm204_print_error_screen(char* error_message_string, char* error_source_string){

    dogm204_write_cmd(DOGM204_FUNCTION_SET);
    dogm204_write_cmd(DOGM204_CLEAR_SCREEN);
    dogm204_set_cursor_shift_dir(DOGM204_CURSOR_RIGHT_SHIFT);
    for (uint8_t i=0; i<MAIN_DISPLAY_COLUMNS; i++){
        dogm204_write_data(MainDisplay_LUT[(uint8_t)error_source_string[i]]);
    }
    for (uint8_t i=0; i<MAIN_DISPLAY_COLUMNS; i++){
        dogm204_write_data(MainDisplay_LUT[(uint8_t) error_message_string[i]]);
    }
}

void dogm204_print_screen(char** screen_text){

    dogm204_write_cmd(DOGM204_FUNCTION_SET);
    dogm204_clear_screen();
    if (!dogm204_screen_text_valid(screen_text)){
        return;
    }
    for (uint8_t i=0; i<MAIN_DISPLAY_ROWS; i++){
        for (uint8_t j=0; j<MAIN_DISPLAY_COLUMNS; j++){
            dogm204_write_data(MainDisplay_LUT[(uint8_t)screen_text[i][j]]);
        }
    }
}

void dogm204_print_message(char** screen_text, uint8_t display_time){

    // Prints message, then goes back to screen data before
    // Displaytime in s
    dogm204_write_cmd(DOGM204_FUNCTION_SET);
    uint8_t current_screen_data[MAIN_DISPLAY_COLUMNS*MAIN_DISPLAY_ROWS];
    dogm204_read_screen(current_screen_data);
    if (!dogm204_screen_text_valid(screen_text)){
        return;
    }
    dogm204_clear_screen();
    vTaskDelay(pdMS_TO_TICKS(500));
    for (uint8_t i=0; i<MAIN_DISPLAY_ROWS; i++){
        for (uint8_t j=0; j<MAIN_DISPLAY_COLUMNS; j++){
            dogm204_write_data(MainDisplay_LUT[(uint8_t)screen_text[i][j]]);
        }
    }
    vTaskDelay(pdMS_TO_TICKS(display_time*1000));
    dogm204_clear_screen();
    vTaskDelay(pdMS_TO_TICKS(500));
    for (uint8_t i=0; i<(MAIN_DISPLAY_COLUMNS*MAIN_DISPLAY_ROWS);i++){
        dogm204_write_data(current_screen_data[i]);
    }
}

void dogm204_print_screen_fancy(char** screen_text){

}

void dogm204_start_loading_screen(char* loading_screen_string){

    strncpy(loading_text,loading_screen_string,20);
    dogm204_print_screen(loading_screen);
    vTaskDelay(pdMS_TO_TICKS(500));
    xEventGroupSetBits(loading_events,LOADING_ACTIVE_BIT);

}

void dogm204_end_loading_screen(void){

    dogm204_clear_screen();
    vTaskDelay(pdMS_TO_TICKS(500));
    xEventGroupClearBits(loading_events,LOADING_ACTIVE_BIT);

}

