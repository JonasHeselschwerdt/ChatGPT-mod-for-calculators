// ChatGPT Hardware Hack compatible with Casio FX Series

// UI.c -> Functions and variables to run the UI with the 4x20 DOGM204 LCD and the TCA8418

// © 2026 Jonas Heselschwerdt
// Licensed under CC BY-NC 4.0




#include "UI.h"
#include "config.h"
#include "keyset.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdint.h>
#include "esp_log.h"
#include "network.h"
#include <stdbool.h>




// TCA8418 base functions and variables


static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t tca8418_dev = NULL;

esp_err_t tca8418_write(uint8_t reg, uint8_t data) {

    // Write Data to register of tca8418

    uint8_t buf[2] = { reg, data };
    return i2c_master_transmit(tca8418_dev, buf, sizeof(buf), -1);
}

esp_err_t tca8418_read(uint8_t reg, uint8_t *data) {

    // Read from register of tca8418

    esp_err_t ret = i2c_master_transmit(tca8418_dev, &reg, 1, -1);
    if (ret != ESP_OK) return ret;
    return i2c_master_receive(tca8418_dev, data, 1, -1);
}








// DOGM204 base functions and variables


void bitBang(uint8_t rs,uint8_t rw,uint8_t data){

    // Helper Function which writes to addresses of the LCD

    gpio_set_level(RW,rw);
    gpio_set_level(RS,rs);

    gpio_set_level(D4, (data >> 4)& 0x01);
    gpio_set_level(D5, (data >> 5)& 0x01);
    gpio_set_level(D6, (data >> 6)& 0x01);
    gpio_set_level(D7, (data >> 7)& 0x01);

    gpio_set_level(E,1);
    esp_rom_delay_us(5);
    gpio_set_level(E,0);
    esp_rom_delay_us(50);

    // some writes take longer to process

    if (rs == 1){
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    gpio_set_level(D4, (data >> 0)& 0x01);
    gpio_set_level(D5, (data >> 1)& 0x01);
    gpio_set_level(D6, (data >> 2)& 0x01);
    gpio_set_level(D7, (data >> 3)& 0x01);

    gpio_set_level(E,1);
    esp_rom_delay_us(5);
    gpio_set_level(E,0);
    esp_rom_delay_us(50);

    // some writes take longer to process

    if (rs == 1){
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}








// TCA8418 advanced functions (use base functions, not externally used)


void tca8418_init(void) {

    // I2C Settings

    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &i2c_bus));

    i2c_device_config_t dev_cfg = {
        .device_address = TCA8418_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &tca8418_dev));

    // Set inverted reset (=enable) to HIGH

    gpio_set_level(KeyEn,1);

    tca8418_write(0x01,0x01); // Set Config Register

    tca8418_write(0x1D,0x1F);  // define Keypad Matrix Structure definition
    tca8418_write(0x1E,0xFF);
    tca8418_write(0x1F,0x01);

    tca8418_write(0x03,0x00);  // Unlock Keypad
}








// DOGM204 advanced functions (use base functions, not externally used)


uint8_t scribble_page[scribble_page_length];  // Holds the message the user typed in
static uint8_t current_page_scribble = 0;     // The current page in scribble mode

char answer_page[answer_page_length + 1];      // Holds the answer of ChatGPT (+1 for string terminator \0)
static uint8_t current_page_answer = 0;       // The current page in answer mode

char extracted[scribble_page_length+1];


// This message gets added to each prompt the user types in
const char *answer_instructions = "Note: Answer in a string of up to 4800 characters, use only 7-bit ASCII signs, answer in the language used before this note, if unclear use english. Do not refer to this note in your answer";

// Variable that keeps track of the menu the user is in
Menu menu = {'s',false,0,0,0};

// Those variables have to be changed for every menu

static uint8_t sub_menu_count = sub_menu_count_main;
static uint8_t menu_pages = menu_pages_main;

// More menu-specific variables

uint8_t prompt_copy[scribble_page_length];  // temporarily save the prompt when using the settings scribble mode

char proposed_ssid[32];
char proposed_pass[64];

// Auto off settings

uint8_t autooff_mins = 2;
uint32_t autooff_timer = 0;

// HTML viewing mode variables

uint16_t html_menu[9] = {0,0,0,0,0,0,0,0,0};        // current location of the html viewer within the 9 texts 
                                                    // see https server hosting tool

uint8_t html_text_passage = 0;              // 0-8 (Text 1 - 9)
char current_html_text[81];

// Keypad mode varriables

char keypad_orient = 'v';       // default 'v' (vertical) alternatively 'h' (horizontal)
char keypad_mode = 'u';          // deafult 'u' (USB) alternatively 'b' (bluetooth)      


void insert_scribble_header(char scribble_mode){

    if (scribble_mode == 'c'){

        // Header: >Write a prompt

        scribble_page[0] = lcd_charset[ArrowR];
        scribble_page[1] = lcd_charset['W'];
        scribble_page[2] = lcd_charset['r'];
        scribble_page[3] = lcd_charset['i'];
        scribble_page[4] = lcd_charset['t'];
        scribble_page[5] = lcd_charset['e'];
        scribble_page[6] = lcd_charset[' '];
        scribble_page[7] = lcd_charset['a'];
        scribble_page[8] = lcd_charset[' '];
        scribble_page[9] = lcd_charset['p'];
        scribble_page[10] = lcd_charset['r'];
        scribble_page[11] = lcd_charset['o'];
        scribble_page[12] = lcd_charset['m'];
        scribble_page[13] = lcd_charset['p'];
        scribble_page[14] = lcd_charset['t'];
        scribble_page[15] = lcd_charset[' '];
        scribble_page[16] = lcd_charset[' '];
        scribble_page[17] = lcd_charset[' '];
        scribble_page[18] = lcd_charset[' '];
        scribble_page[19] = lcd_charset[' '];
    }
    else if (scribble_mode == 'p'){

        // Header: >Enter WIFI password

        scribble_page[0] = lcd_charset[ArrowR];
        scribble_page[1] = lcd_charset['E'];
        scribble_page[2] = lcd_charset['n'];
        scribble_page[3] = lcd_charset['t'];
        scribble_page[4] = lcd_charset['e'];
        scribble_page[5] = lcd_charset['r'];
        scribble_page[6] = lcd_charset[' '];
        scribble_page[7] = lcd_charset['W'];
        scribble_page[8] = lcd_charset['I'];
        scribble_page[9] = lcd_charset['F'];
        scribble_page[10] = lcd_charset['I'];
        scribble_page[11] = lcd_charset[' '];
        scribble_page[12] = lcd_charset['p'];
        scribble_page[13] = lcd_charset['a'];
        scribble_page[14] = lcd_charset['s'];
        scribble_page[15] = lcd_charset['s'];
        scribble_page[16] = lcd_charset['w'];
        scribble_page[17] = lcd_charset['o'];
        scribble_page[18] = lcd_charset['r'];
        scribble_page[19] = lcd_charset['d'];
    }
    else if (scribble_mode == 's'){

        // Header: >Enter WIFI SSID

        scribble_page[0] = lcd_charset[ArrowR];
        scribble_page[1] = lcd_charset['E'];
        scribble_page[2] = lcd_charset['n'];
        scribble_page[3] = lcd_charset['t'];
        scribble_page[4] = lcd_charset['e'];
        scribble_page[5] = lcd_charset['r'];
        scribble_page[6] = lcd_charset[' '];
        scribble_page[7] = lcd_charset['W'];
        scribble_page[8] = lcd_charset['I'];
        scribble_page[9] = lcd_charset['F'];
        scribble_page[10] = lcd_charset['I'];
        scribble_page[11] = lcd_charset[' '];
        scribble_page[12] = lcd_charset['S'];
        scribble_page[13] = lcd_charset['S'];
        scribble_page[14] = lcd_charset['I'];
        scribble_page[15] = lcd_charset['D'];
        scribble_page[16] = lcd_charset[' '];
        scribble_page[17] = lcd_charset[' '];
        scribble_page[18] = lcd_charset[' '];
        scribble_page[19] = lcd_charset[' '];
    }
    else if (scribble_mode == 'k'){

        // Header: >Your OpenAI API Key

        scribble_page[0] = lcd_charset[ArrowR];
        scribble_page[1] = lcd_charset['Y'];
        scribble_page[2] = lcd_charset['o'];
        scribble_page[3] = lcd_charset['u'];
        scribble_page[4] = lcd_charset['r'];
        scribble_page[5] = lcd_charset[' '];
        scribble_page[6] = lcd_charset['O'];
        scribble_page[7] = lcd_charset['p'];
        scribble_page[8] = lcd_charset['e'];
        scribble_page[9] = lcd_charset['n'];
        scribble_page[10] = lcd_charset['A'];
        scribble_page[11] = lcd_charset['I'];
        scribble_page[12] = lcd_charset[' '];
        scribble_page[13] = lcd_charset['A'];
        scribble_page[14] = lcd_charset['P'];
        scribble_page[15] = lcd_charset['I'];
        scribble_page[16] = lcd_charset[' '];
        scribble_page[17] = lcd_charset['K'];
        scribble_page[18] = lcd_charset['e'];
        scribble_page[19] = lcd_charset['y'];
    }
}

void clear_display(void){

    bitBang(0,0,0x01); // Clear Display command of DOGM204

}

void moveCursor(char direction, Cursor *cursor){  // (in scribble mode)

    if (current_page_scribble != 0){

        // separate logics for up neccesary depending on which page because of scribble_page header!

        if (direction == 'u'){
            if (cursor->y > 0){
                cursor->y--;
            }
            else{

                if (current_page_scribble != 0){

                    current_page_scribble--;
                    cursor->y = 3; // Old page

                }
         }
        }
    }
    else{

        // separate logics for up neccesary depending on which page because of scribble_page header!
        
        if (direction == 'u'){
            if (cursor->y > 1){
                cursor->y--;
            }
        }
    }




    if (direction == 'd'){
        if (cursor->y < 3){
            cursor->y++;
        }
        else{

            if (current_page_scribble != max_pages_scribble){

                current_page_scribble++;
                cursor->y = 0; // New page
            }
        }
    }



    if (current_page_scribble != 0){

        // separate logics for left neccesary depending on which page because of scribble_page header!

        if (direction == 'l'){
        
            if (cursor->x > 0){
                cursor->x--;
            }
            else if ((cursor->x == 0) && (cursor->y > 0)){

                cursor->x = 19;
                cursor->y--;      // Jump back a line
            }
            else{

                if (current_page_scribble != 0){

                    current_page_scribble--;
                    cursor->x = 19;
                    cursor->y = 3;    // Jump back a Page

                }
            }
        }        
    }
    else{

        // separate logics for left neccesary depending on which page because of scribble_page header!

        if (direction == 'l'){
        
            if (cursor->x > 0){
                cursor->x--;
            }
            else if ((cursor->x == 0) && (cursor->y > 1)){

                cursor->x = 19;
                cursor->y--;      // Jump back a line
            }
        }  

    }




    if (direction == 'r'){
        if (cursor->x < 19){
            cursor->x++;
        }
        else if ((cursor->x == 19) && (cursor->y < 3)){

            cursor->x = 0;
            cursor->y++;      // Jump a line forwards
        }
        else{

            if (current_page_scribble != max_pages_scribble){

                current_page_scribble++;
                cursor->x = 0;
                cursor->y = 0;    // Jump a page forwards

            }
        }
    }

    // Set DDRAM Address (see DOGM204 and SSD1803A datasheet) to desired position

    uint8_t next_position = 0x00;
    next_position = next_position + (0x20 * cursor->y);
    next_position = next_position + (0x01 * cursor->x);
    bitBang(0,0,(next_position + 0x80));

}

void moveCursor_menu(char direction, Cursor *cursor){

    if (direction == 'u'){

        if (cursor->y > 0){

            cursor->y--;
        }
        else if ((cursor->y == 0) && menu.page > 0){

            menu.page--;
            cursor->y = 3;
        }

    }
    if (direction == 'd'){

        if ((cursor->y < 3) && menu.page < menu_pages){

            cursor->y++;
        }

        else if((cursor->y < (sub_menu_count % 4)) && menu.page == menu_pages){

            cursor->y++;
        }

        else if ((cursor->y == 3) && menu.page < menu_pages){

            menu.page++;
            cursor->y = 0;
        }

    }

    // Set DDRAM Address (see DOGM204 and SSD1803A datasheet) to desired position

    uint8_t next_position = 0x00;
    next_position = next_position + (0x20 * cursor->y);
    next_position = next_position + (0x01 * cursor->x);
    bitBang(0,0,(next_position + 0x80));

}

void initialize_cursor(Cursor* cursor){  

    cursor->x = 0;
    cursor->y = 1;              // because of scribble page header
    bitBang(0,0,0xA0);
}

void lcd_init(void){

    // Proper Reset on start

    gpio_set_level(Reset,1);
    vTaskDelay(pdMS_TO_TICKS(75));
    gpio_set_level(Reset,0);
    vTaskDelay(pdMS_TO_TICKS(12));
    gpio_set_level(Reset,1);
    vTaskDelay(pdMS_TO_TICKS(100)); 

    // Initializing Sequence from DOGM204 Datasheet (4-bit control)

    bitBang(0,0,0x33); 
    bitBang(0,0,0x32); 
    bitBang(0,0,0x2A); 
    bitBang(0,0,0x09); 
    bitBang(0,0,0x05); 
    bitBang(0,0,0x1E); 
    bitBang(0,0,0x29); 
    bitBang(0,0,0x1B); 
    bitBang(0,0,0x6E); 
    bitBang(0,0,0x57); 
    bitBang(0,0,0x72); 
    bitBang(0,0,0x28);
    bitBang(0,0,0x0C);

    // Choose ROM A

    bitBang(0,0,0x2A);
    bitBang(0,0,0x72);
    bitBang(1,0,0x00);
    bitBang(0,0,0x28);

    // Setup charset

    setup_charset();

    bitBang(0,0,0x01); // Clear Display
    vTaskDelay(pdMS_TO_TICKS(1000)); 
}

void shift_scribble_page(Cursor* cursor, char shift_direction){

    // Calculate from where to start shifting

    uint16_t shift_start = 0;
    shift_start = shift_start + (current_page_scribble * 80);
    shift_start = shift_start + (cursor->y * 20);
    shift_start = shift_start + (cursor->x);

    if (shift_direction == 'f'){    // forward shift

        // Moves everything to the right of shift_start to the right (print)

        uint8_t temporary_save = scribble_page[shift_start];
        uint8_t temporary_save2 = 0x00;

        for (int i = (shift_start + 1); i < scribble_page_length; i++){

            // Temporarily save overwritten List entry in scribble_page
            temporary_save2 = scribble_page[i];
            scribble_page[i] = temporary_save;
            temporary_save = temporary_save2;
        }

    }

    else if (shift_direction == 'b'){   // backwards shift

        // moves everything to the right of shift_start to the left (delete)

        uint8_t temporary_save = 0x20;
        uint8_t temporary_save2 = 0x00;

        for (int i = (scribble_page_length - 1); i > (shift_start -1 ); i--){

            temporary_save2 = scribble_page[i]; 
            scribble_page[i] = temporary_save;
            temporary_save = temporary_save2;
        }
    }

}

void print_direct(Cursor* cursor, uint8_t sign){

    // Calculate DDRAM Address

    uint8_t print_location = 0x00;

    if ((cursor->x < 20) && (cursor->y < 4)){

        print_location = print_location + (0x20 * cursor->y);
        print_location = print_location + (0x01 * cursor->x);

        // Select DDRAM Address and write to it

        bitBang(0,0,(print_location + 0x80));
        bitBang(1,0,lcd_charset[sign]);

        // Update Cursor with new DDRAM Address

        if ((cursor->x < 19)){
            cursor->x++;
        }
        else if ((cursor->x == 19)&& (cursor->y < 3)){
            cursor->x = 0;
            cursor->y++;
        }
        else if ((cursor->x == 19)&& (cursor->y == 3)){
            cursor->x = 0;
            cursor->y = 0;
        }

        // Move cursor to next position manually (would work automatically too)

        print_location = 0x00;
        print_location = print_location + (0x20 * cursor->y);
        print_location = print_location + (0x01 * cursor->x);
        bitBang(0,0,(print_location + 0x80));


    }

}

void delete_direct(Cursor* cursor){

    // Works like print_direct, sets a coordinate to the space sign,
    // then moves cursor in the opposite direction as print_direct

    uint8_t delete_location = 0x00;

    if ((cursor->x < 20) && (cursor->y < 4)){

        if (cursor->x > 0){
            cursor->x--;
        }

        else if ((cursor->x == 0) && cursor->y > 0){
            cursor->x = 19;
            cursor->y--;
        }

        else if ((cursor->x == 0) && (cursor->y == 0)){
            cursor->x = 19;
            cursor->y = 3;
        }

        delete_location = delete_location + (0x20 * cursor->y);
        delete_location = delete_location + (0x01 * cursor->x);

        bitBang(0,0,(delete_location + 0x80));
        bitBang(1,0,lcd_charset[' ']);

        delete_location = 0x00;
        delete_location = delete_location + (0x20 * cursor->y);
        delete_location = delete_location + (0x01 * cursor->x);
        bitBang(0,0,(delete_location + 0x80));

    }

}

void print_line(const char* print_text, uint8_t line, Cursor* cursor){

    // Helper function to print menues and splatscreens
    // Doesnt affect the scribble page list!

    if ((strlen(print_text) <= 20) && (line < 4)){

        cursor->x = 0;
        cursor->y = line;

        for (int i=0; i < 20; i++){
            if (print_text[i] != '\0'){
                print_direct(cursor, print_text[i]);
            }
            else{
                print_direct(cursor,' ');
            }

        }
    }
}

void print_screen(const char* print_text, Cursor *cursor){

    // Prints out an 80 sign long text on the display
    
    char text_line1[21];
    char text_line2[21];
    char text_line3[21];
    char text_line4[21];

    memcpy(text_line1, print_text +  0, 20);
    memcpy(text_line2, print_text + 20, 20);
    memcpy(text_line3, print_text + 40, 20);
    memcpy(text_line4, print_text + 60, 20);

    text_line1[20] = '\0';
    text_line2[20] = '\0';
    text_line3[20] = '\0';
    text_line4[20] = '\0';

    print_line(text_line1,0,cursor);
    print_line(text_line2,1,cursor);
    print_line(text_line3,2,cursor);
    print_line(text_line4,3,cursor);

}

void print_scribble_page(void){

    // Function that prints the content from scribble_page out

    // find startpoint in scribble_page list depending on current_page

    uint16_t startpoint = 0;
    startpoint = startpoint + (current_page_scribble * 80);

    Cursor temporary_cursor = {0,0};
    uint8_t temporary_print_pos = 0x00;

    for (int i = 0; i < 80; i++){

        temporary_print_pos = temporary_print_pos + (0x20 * temporary_cursor.y);
        temporary_print_pos = temporary_print_pos + (0x01 * temporary_cursor.x);

        bitBang(0,0,(temporary_print_pos + 0x80));
        bitBang(1,0,scribble_page[startpoint + i]);

        if ((temporary_cursor.x < 19)){
            temporary_cursor.x++;
        }
        else if ((temporary_cursor.x == 19)&& (temporary_cursor.y < 3)){
            temporary_cursor.x = 0;
            temporary_cursor.y++;
        }
        else if ((temporary_cursor.x == 19)&& (temporary_cursor.y == 3)){
            temporary_cursor.x = 0;
            temporary_cursor.y = 0;
        }

        temporary_print_pos = 0x00;
    }
}

void print_answer_page(Cursor cursor)
{
    char line[21]; 

    int base_line = current_page_answer * 4;

    for (int row = 0; row < 4; row++) {

        int line_index = base_line + row;
        int src_offset = line_index * 20;

        strncpy(line, &answer_page[src_offset], 20);
        line[20] = '\0';

        print_line(line, row, &cursor);
    }
}

/*/
void print_answer_page(Cursor cursor){

    // split the answer page into ((max_pages_answer + 1) *4) strings, each with 20 characters!
    // Arrange them in a list!

    char splitted_answer_page[((max_pages_answer+1)*4)][21];

    for (int i = 0; i < ((max_pages_answer+1)*4); i++){

        strncpy(splitted_answer_page[i], &answer_page[i * 20], 20);
        splitted_answer_page[i][20] = '\0';
    }

    // Print out the 4 relevant lines of the answer page, depending on the current_answer_page

    for (int i = (current_page_answer*4); i < ((current_page_answer*4)+4); i++){

        print_line(splitted_answer_page[i],(i % 4),&cursor);
    }
    
}
/*/

void print_menu(Cursor cursor){

    if (menu.sub_menu == 0){

        // We are in the main menu

        if (menu.page == 0){

            print_line(" Edit WIFI List     ",0,&cursor);
            print_line(" Edit OpenAI API Key",1,&cursor);
            print_line(" GPT Model Selection",2,&cursor);
            print_line(" Enter Keypad Mode  ",3,&cursor);
        }
        else if (menu.page == 1){

            print_line(" Auto-off timer     ",0,&cursor);
            print_line(" Factory Reset      ",1,&cursor);
            print_line(" Firmware Info      ",2,&cursor);
            print_line("                    ",3,&cursor);

        }

    }

    else if ((menu.sub_menu == 1) && (menu.subsub_menu == 0)){

        // We look at the wifi list

        char ssid[32];
        char dummy[64];  // we dont care about passwords right now, so we use a dummy
        
        for (int i = (0 + (menu.page * 4)); i < (4 + (menu.page * 4)); i++){

            memset(ssid, 0, sizeof(ssid));
            if (wifi_load_credentials(i, ssid, sizeof(ssid), dummy, sizeof(dummy)) == ESP_OK){
                ssid[19] = '\0';

                // First sign = Space sign

                memmove(ssid + 1, ssid, 19);
                ssid[0] = ' ';
                ssid[20] = '\0';

                print_line(ssid,(i%4),&cursor);
            }
            else{
                print_line(" Add new wifi here  ",(i%4),&cursor);
            }
        }

    }

    else if ((menu.sub_menu == 4) && (menu.subsub_menu == 0)){

        print_line(" Start USB Keypad   ",0,&cursor);
        print_line(" Start BLE Keypad   ",1,&cursor);
        print_line(" Set Keypad Layout  ",2,&cursor);
        print_line("                    ",3,&cursor);       

    }

    else if ((menu.sub_menu == 4) && (menu.subsub_menu == 3)){

        print_line(" Vertical Keypad    ",0,&cursor);
        print_line(" Horizontal Keypad  ",1,&cursor);
        print_line("                    ",2,&cursor);
        print_line("                    ",3,&cursor);       

    }

    else if ((menu.sub_menu == 3) && (menu.subsub_menu == 0)){

        print_line(" GPT 3.5-Turbo      ",0,&cursor);
        print_line(" GPT 4o             ",1,&cursor);
        print_line("                    ",2,&cursor);
        print_line("                    ",3,&cursor);

    }

    else if ((menu.sub_menu == 5) && (menu.subsub_menu == 0)){

        print_line(" Auto off @  1 min  ",0,&cursor);
        print_line(" Auto off @  2 min  ",1,&cursor);
        print_line(" Auto off @  5 min  ",2,&cursor);
        print_line(" Auto off @ 15 min  ",3,&cursor);


    }

    else if ((menu.sub_menu == 6) && (menu.subsub_menu == 0)){

        print_line(" Yes, reset device  ",0,&cursor);
        print_line(" No, cancel reset   ",1,&cursor);
        print_line("                    ",2,&cursor);
        print_line("                    ",3,&cursor);
    }

    else{

        print_line(" Menu not found     ",0,&cursor);
        print_line("                    ",1,&cursor);
        print_line("                    ",2,&cursor);
        print_line("                    ",3,&cursor);


    }





}

void setcursor(bool cursor, bool blink){

    // Toggles the cursorfunction of the lcd

    uint8_t cmd = 0x0C;

    if (cursor == 1){

        cmd = cmd + 0x03;
    }
    if (blink == 0){

        cmd = cmd - 0x01;
    }

    bitBang(0,0,cmd);
}

char* extract_scribble_page(){      // extract what the user typed in

    for (int i= (scribble_page_length-1);i>=0;i--){
        for (int j = 255; j>=0; j--){
            if (lcd_charset[j] == scribble_page[i]){
                extracted[i] = (char)j;
                break;
            }
        }
    }
    extracted[scribble_page_length] = '\0';

    return extracted;
}









// Various other functions and variables


void device_shutdown(Cursor cursor){

    setcursor(0,1);
    bitBang(0,0,0x01);              // Clear Display DDRAM

    // Print shutdown info screen

    print_line(" Device is shutting ",1,&cursor);
    print_line("       down         ",2,&cursor);
    
    gpio_set_level(KeyEn,0);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(PE,0);

}

char UI_mode = 's';     // The mode in which the UI currently is

// s = scribble
// a = answer
// m = menu
// h = html viewer
// c = calculator (stealth mode)
// k = keypad mode (HID)

char url[64] = "http://192.168.";








// Externally used functions (in main.c)

void UI_init(void){

    tca8418_init();
    define_keyset();
    lcd_init();
    setup_hid_LUT();
    setup_special_hid_LUT('v');

}

void update_keyregister(uint8_t* registerpointer, uint8_t* registerpointer_old){

    // Important function in main loop

    // Copy old Keyregistercontents into a save file
    // for later comparisons

    for (int i = 0; i<10; i++){

        registerpointer_old[i] = registerpointer[i];
    }

    // Is there a keypadinterrupt?
    uint8_t interrupt;
    if(tca8418_read(0x02,&interrupt) == ESP_OK){
        if (interrupt &0x01){

            // How many events occured?
            uint8_t event_counter;
            if(tca8418_read(0x03,&event_counter) == ESP_OK){
                event_counter = event_counter & 0x0F;

                for (int i = 0; i < event_counter && i < 10; i++){

                    // Check all Keyevents, edit Keyregister (add (pressed) or delete (released) keys)

                    uint8_t key_event;
                    if (tca8418_read(0x04,&key_event) == ESP_OK){

                        if ((key_event >> 7) == 1){

                            for (int j = 0; j<10; j++ ){
                                if (registerpointer[j] == 0x00){
                                    registerpointer[j] = key_event;
                                    break;
                                }
                                
                            }
                        }

                        else if ((key_event >> 7) == 0){
                            
                            for (int j = 0; j<10; j++ ){
                                if (registerpointer[j] == (key_event | 0x80)){
                                    registerpointer[j] = 0x00;
                                    break;
                                }

                            }

                        }
                    } 

                }

                tca8418_write(0x02,0x01);  // Clear Key interrupt events if possible
            }
        }
    }

}

void print_start_screen(Cursor cursor){

    print_line("  CASIO FX-SERIES   ",1,&cursor);
    print_line("  HARDWARE HACK     ",2,&cursor);

    vTaskDelay(pdMS_TO_TICKS(1500));
    clear_display();

    //

    print_line("   DEVELOPED  BY    ",1,&cursor);
    print_line("JONAS HESELSCHWERDT ",2,&cursor);

    vTaskDelay(pdMS_TO_TICKS(2500));
    clear_display();
    
}

void print_firmware_info(Cursor cursor){

    clear_display();
    setcursor(0,1);
    print_line("Installed Firmware: ",1,&cursor);
    print_line(firmware_version,2,&cursor);
    vTaskDelay(pdMS_TO_TICKS(3000));
    setcursor(1,0);
    clear_display();

}


void handle_key_register_scribble(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor){

    // Checks the keyregister for content and starts the correct functions

    if ((keyset[registerpointer[0]].event_key) == EVENT_NONE){

        // Handles normal keys with no special event assigned
        // Shift or alpha havent been presses beforehand, normal values are printed
        // at the position of the cursor

        if (registerpointer[0] != 0){

            // Ignore Keyreleases

            if (registerpointer[0] != registerpointer_old[0]){

                // Inject printed char in scribble_page List!

                uint16_t injection_point = 0;
                injection_point = injection_point + (current_page_scribble * 80);
                injection_point = injection_point + (cursor->y * 20);
                injection_point = injection_point + (cursor->x);
                
                // Only print if this key was not in the register before
                // Another keypress might have triggered the interrupt of the tca8418 as well
                

                shift_scribble_page(cursor,'f');
                scribble_page[injection_point] = lcd_charset[keyset[registerpointer[0]].normal_value];

                // Move cursor

                print_scribble_page();
                moveCursor('r',cursor);

                /*/ Debugging

                ESP_LOGI("Update:","Shifted forward");

                for (int i = 0; i < scribble_page_length; i++){
                    ESP_LOGI("Scribble Page","@%d: 0x%02X",i,scribble_page[i]);
                }

                /*/

                }
            }
        
    }

    else if (((keyset[registerpointer[0]].event_key) == SHIFT)){

        if (registerpointer[1] != 0x00){

            if (((keyset[registerpointer[1]].event_key) == EVENT_NONE)){

                // Check if second pressed key is a normal one!

                if (registerpointer[1] != registerpointer_old[1]){

                    uint16_t injection_point = 0;
                    injection_point = injection_point + (current_page_scribble * 80);
                    injection_point = injection_point + (cursor->y * 20);
                    injection_point = injection_point + (cursor->x);

                    shift_scribble_page(cursor,'f');
                    scribble_page[injection_point] = lcd_charset[keyset[registerpointer[1]].shift_value]; 

                    print_scribble_page();
                    moveCursor('r',cursor);

                    /*/ Debugging

                    ESP_LOGI("Update:","Shifted forward");

                    for (int i = 0; i < scribble_page_length; i++){
                        ESP_LOGI("Scribble Page","@%d: 0x%02X",i,scribble_page[i]);
                    }
                    /*/

                }
            }
            else if (((keyset[registerpointer[1]].event_key) == MENU)){

                // SHIFT + MENU = Shutdown system
                device_shutdown(*cursor);
            }
        }
    }

    else if (((keyset[registerpointer[0]].event_key) == ALPHA)){

        if (registerpointer[1] != 0x00){

            if (registerpointer[1] != registerpointer_old[1]){

                uint16_t injection_point = 0;
                injection_point = injection_point + (current_page_scribble * 80);
                injection_point = injection_point + (cursor->y * 20);
                injection_point = injection_point + (cursor->x);


                shift_scribble_page(cursor,'f');
                scribble_page[injection_point] = lcd_charset[keyset[registerpointer[1]].alpha_value]; 

                print_scribble_page();
                moveCursor('r',cursor);

                /*/ Debugging

                ESP_LOGI("Update:","Shifted forward");

                for (int i = 0; i < scribble_page_length; i++){
                    ESP_LOGI("Scribble Page","@%d: 0x%02X",i,scribble_page[i]);
                }
                /*/

            }
        }
    }

    else if (

            (keyset[registerpointer[0]].event_key) == UP ||
            (keyset[registerpointer[0]].event_key) == DOWN || 
            (keyset[registerpointer[0]].event_key) == RIGHT ||
            (keyset[registerpointer[0]].event_key) == LEFT)
            
            {
                // Handle cursor movements

                print_scribble_page();
                moveCursor((keyset[registerpointer[0]].normal_value), cursor);

            }
    
    else if(((keyset[registerpointer[0]].event_key) == BACK)){

        // Inject a Space character into scribble page

        if (registerpointer[0] != 0){

            // Ignore Keyreleases

            if (registerpointer[0] != registerpointer_old[0]){

                uint16_t injection_point = 0;
                injection_point = injection_point + (current_page_scribble * 80);
                injection_point = injection_point + (cursor->y * 20);
                injection_point = injection_point + (cursor->x);

                scribble_page[injection_point] = 0x20; // Scribble Page contains lcd_charset codes

                shift_scribble_page(cursor,'b');

                print_scribble_page();
                moveCursor('l',cursor);

                /*/ Debugging

                ESP_LOGI("Update","Shifted backwards");

                for (int i = 0; i < scribble_page_length; i++){
                    ESP_LOGI("Scribble Page","@%d: 0x%02X",i,scribble_page[i]);
                }
                /*/
            }
        }
    }

    else if(((keyset[registerpointer[0]].event_key) == ENTER)){

        // ENTER allows you to leave scribble mode to get into the answer mode (or html viewer mode)!

        if (registerpointer[0] != 0){

            // Ignore Keyreleases

            if (registerpointer[0] != registerpointer_old[0]){

                if (menu.main_menu == false){

                    extract_scribble_page();

                    if(extracted[20] == '@'){

                        // User wants to view a HTML file @ a specific IPv4 / locally or wants to enter game mode

                        if (strncmp(&extracted[21], "esp", 3) == 0){

                            // Look at Local file

                            clear_display();

                            UI_mode = 'h';

                            // Go to text passage 0 everywhere

                            for (int i = 0; i <= 8; i++){

                                html_menu[i] = 0;
                            }

                            html_text_passage = 0;

                            setcursor(0,1);

                            display_from_local_html(html_menu, html_text_passage, current_html_text,cursor);

                            ESP_LOGI("HTML","%s",current_html_text);

                        }

                        else if (strncmp(&extracted[21], "game", 4) == 0){

                            // Enter game mode
                        }

                        else{

                            // Download html file from local server

                            // Check: does server exist?

                            strcpy(url, "http://192.168.");
                            char url_ending[16]; 

                            memcpy(url_ending, &extracted[21], 15);
                            url_ending[15] = '\0';

                            strcat(url,url_ending);

                            if (check_for_http_server(url)){

                                ESP_LOGI("Success","Connected to HTTP :)");

                                // Reset Scribble page, clear display

                                for (int i = (scribble_page_length-1); i>=20; i--){
                                    scribble_page[i] = 0x20;
                                }
                                extracted[scribble_page_length] = '\0';

                                clear_display();

                                // Enter HTML Mode

                                UI_mode = 'h';
                                
                                // Go to text passage 0 everywhere

                                for (int i = 0; i <= 8; i++){

                                    html_menu[i] = 0;
                                }

                                html_text_passage = 0;

                                setcursor(0,1);

                                // Important: Save entire Text into LittleFS file system once
                                // -> no internet communication necessary later! (save power)

                                read_from_http(url);

                                // Display first section

                                display_from_local_html(html_menu, html_text_passage, current_html_text,cursor);

                                // Debugging

                                ESP_LOGI("HTML","%s",current_html_text);
                                
                            }

                        }

                    }

                    else{

                        // User typed in a ChatGPT Prompt

                        UI_mode = 'a';   // set UI_mode to a -> change into answer mode after completing this iteration
                                         // of handle_keyregister_scribble

                        current_page_answer = 0;
                
                        setcursor(0,1);   // turn cursor off in answer mode

                        // Remove all the spaces at the end of scribble page

                        for (int i = strlen(extracted) - 1; i >= 0; i--) {
                            if (extracted[i] == ' ') {
                                extracted[i] = '\0';   
                            } else {
                                break; 
                            }
                        }

                        // Add prompt engineering suffix to make sure all the answers are not to
                        // long and contain only valid ASCII signs

                        strncat(extracted, answer_instructions, sizeof(extracted) - strlen(extracted) - 1);

                        // Put the answer of ChatGpt into the answer page, ignore the first 20 signs in extracted (header)

                        strncpy(answer_page, handle_openai_chat(extracted + 20), answer_page_length);
                        answer_page[answer_page_length] = '\0'; 

                        // reset extracted

                        for (int i = (scribble_page_length-1); i>=0; i--){
                            extracted[i] = '0';
                        }

                        // reset scribble_page, leave the header of the page

                        for (int i = (scribble_page_length-1); i>=20; i--){
                            scribble_page[i] = 0x20;
                        }
                        extracted[scribble_page_length] = '\0';

                    }

                }

                else if(menu.sub_menu == 2){

                    // The API Key has been typed in, needs to be saved now
                    // leave menu, reset scribble page header

                    extract_scribble_page();

                    // Remove all the spaces at the end

                    for (int i = strlen(extracted) - 1; i >= 0; i--) {
                        if (extracted[i] == ' ') {
                            extracted[i] = '\0';   
                        } else {
                            break; 
                        }
                    }

                    // Change NVS saved data of API Key

                    api_save_key(extracted+20);

                    // Empty Scribble page again

                    for (int i = (scribble_page_length-1); i>=20; i--){
                        scribble_page[i] = 0x20;
                    }

                    // Reset extracted

                    for (int i = (scribble_page_length-1); i>=0; i--){
                        extracted[i] = '0';
                    }

                    // Print success screen

                    clear_display();
                    setcursor(0,1);

                    print_line("   API Key saved   ",1,cursor);
                    print_line("   to the device   ",2,cursor);
                    vTaskDelay(pdMS_TO_TICKS(2000));

                    setcursor(1,1);
                    
                    // Change scribble header back to command mode

                    insert_scribble_header('c');
                    print_scribble_page();
                    initialize_cursor(cursor);

                    // Leave the menu again (set default values again)

                    menu = (Menu){'s',false,0,0,0}; 

                }

                else if ((menu.subsub_menu != 0) && (menu.page == 0)){

                    // The SSID has been typed in, temporarily save it and ask for the password next
                    // set menu.subsub_menu = 1

                    extract_scribble_page();

                    // Remove all the spaces at the end

                    for (int i = strlen(extracted) - 1; i >= 0; i--) {
                        if (extracted[i] == ' ') {
                            extracted[i] = '\0';   
                        } else {
                            break; 
                        }
                    }

                    // Temporarily save ssid

                    memset(proposed_ssid, 0, sizeof(proposed_ssid));
                    strncpy(proposed_ssid, extracted+20, sizeof(proposed_ssid));
                    proposed_ssid[sizeof(proposed_ssid)-1] = '\0';

                    // Empty Scribble page again

                    for (int i = (scribble_page_length-1); i>=20; i--){
                        scribble_page[i] = 0x20;
                    }

                    // Reset extracted

                    for (int i = (scribble_page_length-1); i>=0; i--){
                        extracted[i] = '0';
                    }

                    // Change scribble header to password mode

                    insert_scribble_header('p');

                    menu.page = 1;
                    print_scribble_page();

                    initialize_cursor(cursor);
                }

                else if ((menu.subsub_menu != 0) && (menu.page == 1)){

                    // The password was typed in, save to nvs storage
                    // leave menu, reset scribble page header

                    extract_scribble_page();

                    // Remove all the spaces at the end

                    for (int i = strlen(extracted) - 1; i >= 0; i--) {
                        if (extracted[i] == ' ') {
                            extracted[i] = '\0';   
                        } else {
                            break; 
                        }
                    }

                    // Temporarily save password

                    memset(proposed_pass, 0, sizeof(proposed_pass));
                    strncpy(proposed_pass, extracted+20, sizeof(proposed_pass));
                    proposed_pass[sizeof(proposed_pass)-1] = '\0';

                    wifi_clear_credentials((menu.subsub_menu-1));
                    wifi_save_credentials((menu.subsub_menu-1),proposed_ssid,proposed_pass);

                    ESP_LOGI("NVS","Saved SSID: %s",proposed_ssid);

                    // Empty Scribble page again

                    for (int i = (scribble_page_length-1); i>=20; i--){
                        scribble_page[i] = 0x20;
                    }

                    // Reset extracted

                    for (int i = (scribble_page_length-1); i>=0; i--){
                        extracted[i] = '0';
                    }


                    // Print success screen

                    clear_display();
                    setcursor(0,1);

                    print_line("   New Wifi saved  ",1,cursor);
                    print_line("   to the device   ",2,cursor);
                    vTaskDelay(pdMS_TO_TICKS(2000));

                    setcursor(1,1);

                    // Change scribble header back to command mode

                    insert_scribble_header('c');

                    // Leave the menu again (set default values again)

                    menu = (Menu){'s',false,0,0,0}; 

                    memset(proposed_ssid, 0, sizeof(proposed_ssid));
                    memset(proposed_pass, 0, sizeof(proposed_pass));

                    print_scribble_page();
                    initialize_cursor(cursor);
                    

                }



            }
        }
    }

    else if(((keyset[registerpointer[0]].event_key) == MENU)){

        if (registerpointer[0] != 0){

            if (registerpointer[0] != registerpointer_old[0]){

                if (menu.main_menu == false){

                    UI_mode = 'm';   // Set UI_mode to m -> change into menu mode

                    setcursor(1,0);
                    initialize_cursor(cursor);
                    moveCursor_menu('u',cursor);

                    // Reset temporary save variables of ssid / password

                    memset(proposed_ssid, 0, sizeof(proposed_ssid));
                    memset(proposed_pass, 0, sizeof(proposed_pass));

                    menu = (Menu){'s',true,0,0,0};
                    print_menu(*cursor);

                    // do not reset the scribble_page, do not extract scribble page

                    menu_pages = menu_pages_main;
                    sub_menu_count = sub_menu_count_main;

                }

                else if (menu.main_menu == true){

                    // Go back to prompt mode

                    clear_display();
                    setcursor(0,1);

                    print_line("  Operation aborted ",1,cursor);
                    print_line("  by user           ",2,cursor);

                    clear_display();
                    setcursor(1,0);

                    menu = (Menu){'s',false,0,0,0};
                    initialize_cursor(cursor);
                    insert_scribble_header('c');

                }

            }

        }

    }

}


void handle_keyregister_answer(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor){

    if(((keyset[registerpointer[0]].event_key) == ENTER)){

        if (registerpointer[0] != 0){

            if (registerpointer[0] != registerpointer_old[0]){

                // Allows you to go back to scribble mode

                UI_mode = 's';

                setcursor(1,1);

                // Empty the answer_page again!

                for (int i=0;i < answer_page_length; i++){

                    answer_page[i] = ' ';  // Space characters everywhere
                }
                answer_page[answer_page_length] = '\0';

                print_scribble_page();
                initialize_cursor(cursor);

                return;  // Leave this function immediately, dont print the answer_page!
            }
        }
    }

    else if (((keyset[registerpointer[0]].event_key) == UP) && current_page_answer > 0){

        current_page_answer--;
    }

    else if (((keyset[registerpointer[0]].event_key) == DOWN) && current_page_answer < max_pages_answer){

        current_page_answer++;
    }

    else if (((keyset[registerpointer[0]].event_key) == SHIFT)){

        if (((keyset[registerpointer[1]].event_key) == MENU)){

            // Turn the device off

            device_shutdown(*cursor);
            return;    // do not print the answer_page while the shutdown is performed  

        }

    }
    else if (((keyset[registerpointer[0]].event_key) == MENU)){

        // Enter menu from answer page

        if (registerpointer[0] != 0){

            if (registerpointer[0] != registerpointer_old[0]){

                UI_mode = 'm';
                setcursor(1,0);
                initialize_cursor(cursor);
                moveCursor_menu('u',cursor);
                menu = (Menu){'a',true,0,0,0};
                print_menu(*cursor);
                return;
            }
        }

    }

    print_answer_page(*cursor);  // print the answer page (it most probably changed!)
         
}

void handle_keyregister_menu(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor *cursor){

    if (((keyset[registerpointer[0]].event_key) == UP) || ((keyset[registerpointer[0]].event_key) == DOWN)){

        // Allows you to move around in the menu

        moveCursor_menu((keyset[registerpointer[0]].normal_value),cursor);

        uint8_t temp_x = cursor->x;
        uint8_t temp_y = cursor->y;
        
        print_menu(*cursor);

        // Return cursor to where it was before

        cursor->x = temp_x;
        cursor->y = temp_y;

        uint8_t next_position = 0x00;
        next_position = next_position + (0x20 * cursor->y);
        next_position = next_position + (0x01 * cursor->x);
        bitBang(0,0,(next_position + 0x80));

    }

    else if (((keyset[registerpointer[0]].event_key) == MENU)){

        // Leave the menu again!

        UI_mode = menu.opened_from;
        menu = (Menu){'s',false,0,0,0}; // Reset menu when leaving

        if (UI_mode == 's'){

            // Go back to scribble page

            setcursor(1,1);
            print_scribble_page();
            initialize_cursor(cursor);

        }
        else if (UI_mode == 'a'){

            // Go back to answer page

            current_page_answer = 0;
            setcursor(0,1);

        }

        else if (UI_mode == 'h'){

            // Back to HTML viewer

            setcursor(0,1);
            clear_display();
            display_from_local_html(html_menu, html_text_passage, current_html_text,cursor);

        }
    }

    else if (((keyset[registerpointer[0]].event_key) == SHIFT)){

        if (((keyset[registerpointer[1]].event_key) == MENU)){

            // Turn the device off

            device_shutdown(*cursor);
            return;    // do not print the answer_page while the shutdown is performed  

        }

    }

    else if (((keyset[registerpointer[0]].event_key) == ENTER)){

        // Enter submenu from main menu (if in main menu at beginning)

        if (menu.sub_menu == 0){
            
            menu.sub_menu = (cursor->y + 1) + (menu.page * 4);

            // refresh the menu restrictions
            // leave menu mode if neccesary

            if (menu.sub_menu == 1){
                menu_pages = menu_pages_wifi;
                sub_menu_count = sub_menu_count_wifi;
                menu.page = 0;
            }
            else if (menu.sub_menu == 3){
                menu_pages = menu_pages_models;
                sub_menu_count = sub_menu_count_models;
            }
            else if (menu.sub_menu == 4){
                menu_pages = menu_pages_keypad;
                sub_menu_count = sub_menu_count_keypad;
            }
            else if (menu.sub_menu == 5){
                menu_pages = menu_pages_autooff;
                sub_menu_count = sub_menu_count_autooff;
            }
            else if (menu.sub_menu == 6){
                
                // Special case: First display a little warning:

                clear_display();
                setcursor(0,1);

                print_line("Resetting the device",0,cursor);
                print_line("will delete ALL user",1,cursor);
                print_line("information from the",2,cursor);
                print_line("device IRREVERSIBLY ",3,cursor);

                vTaskDelay(pdMS_TO_TICKS(5000));
                setcursor(1,0);

                clear_display();

                menu_pages = menu_pages_reset;
                sub_menu_count = sub_menu_count_reset;
            }
            else if (menu.sub_menu == 7){
                
                // Special case: Only show the firmware version, return to main menu

                print_firmware_info(*cursor);

                menu.sub_menu = 0;
                menu.page = 0;
                menu_pages = menu_pages_main;
                sub_menu_count = sub_menu_count_main;
                cursor->y = 0;

            }
            else if (menu.sub_menu == 2){

                // Special case: enter settings scribble mode!
                UI_mode = 's';
                insert_scribble_header('k');

                // Temporarily save the scribble page
                // Empty the scribble page, print the scribble page

                memcpy(prompt_copy, scribble_page, scribble_page_length);

                for (int i=20;i < scribble_page_length; i++){

                    scribble_page[i] = 0x20;  // Space characters everywhere
                }

                setcursor(1,1);
                print_scribble_page();
                initialize_cursor(cursor);

                return;

            }

            // print the menu

            print_menu(*cursor);

            // Set cursor back to 0!
            
            cursor->y = 0;
            menu.page = 0;

        }

        else if (menu.sub_menu == 5){

            clear_display();
            setcursor(0,1);

            if (cursor->y == 0){
                print_line(" 1 Minute           ",3,cursor);
                settings_set_str("autooff","1");
                autooff_mins = 1;
            }
            else if (cursor->y == 1){
                print_line(" 2 Minutes          ",3,cursor);
                settings_set_str("autooff","2");
                autooff_mins = 2;
            }
            else if (cursor->y == 2){
                print_line(" 5 Minutes          ",3,cursor);
                settings_set_str("autooff","5");
                autooff_mins = 5;
            }
            else{
                print_line(" 15 Minutes         ",3,cursor);
                settings_set_str("autooff","15");
                autooff_mins = 15;
            }

            print_line(" Switched Auto off  ",0,cursor);
            print_line(" Timer to           ",1,cursor);

            vTaskDelay(pdMS_TO_TICKS(3000));
            clear_display();
            setcursor(1,0);

            cursor->y = 0;
            menu.sub_menu = 0;
            menu.page = 0;

            menu_pages = menu_pages_main;
            sub_menu_count = sub_menu_count_main;

            print_menu(*cursor);

        }

        else if ((menu.sub_menu == 4) && (menu.subsub_menu == 0)){

            if (cursor->y == 0){

                // start USB HID mode

                setcursor(0,1);
                clear_display();

                print_line("     REBOOTING      ",1,cursor);
                print_line("    Please Wait     ",2,cursor);

                vTaskDelay(pdMS_TO_TICKS(2000));

                clear_display();

                toggle_usb_mode(*cursor);      // restarts the ESP32 in usb hid mode

            }

            else if (cursor->y == 1){

                // start BLE HID mode

                /*/
                UI_mode = 'k';
                keypad_mode = 'b';
                setcursor(0,1);

                menu.main_menu = false;
                menu.sub_menu = 0;

                clear_display();
                print_line("Device in BLE Keypad",0,cursor);
                print_line("mode, [Menu] = Leave",1,cursor);
                print_line(BLE_NAME_LABELED,2,cursor);
                print_line("State: Disconnected ",3,cursor);

                /*/

                clear_display();
                setcursor(0,1);

                print_line("     Error: Not     ",1,cursor);
                print_line("   implemented yet  ",2,cursor);

                vTaskDelay(pdMS_TO_TICKS(2000));

                setcursor(1,0);

                menu.sub_menu = 0;
                menu.page = 0;
                cursor->y = 0;
                menu_pages = menu_pages_main;
                sub_menu_count = sub_menu_count_main;

                print_menu(*cursor);

            }
            else{

                menu.subsub_menu = (cursor->y + 1);
                menu.page = 0;

                menu_pages = menu_pages_keypad_orient;
                sub_menu_count = sub_menu_count_keypad_orient;

                cursor->y = 0;
                print_menu(*cursor);
            }
 
        }

        else if ((menu.sub_menu == 4) && (menu.subsub_menu == 3)){

            if (cursor->y == 0){

                keypad_orient = 'v';
                setup_special_hid_LUT(keypad_orient);

                // Print success screen

                clear_display();
                setcursor(0,1);

                print_line("Switched to vertical",1,cursor);
                print_line("    Keypad Layout   ",2,cursor);
                vTaskDelay(pdMS_TO_TICKS(2000));

                menu.subsub_menu = 0;
                menu_pages = menu_pages_keypad;
                sub_menu_count = sub_menu_count_keypad;
                cursor->y = 0;
                print_menu(*cursor);
                setcursor(1,0);

            }
            else{

                keypad_orient = 'h';
                setup_special_hid_LUT(keypad_orient);

                clear_display();
                setcursor(0,1);

                print_line(" Switched to horiz. ",1,cursor);
                print_line("    Keypad Layout   ",2,cursor);
                vTaskDelay(pdMS_TO_TICKS(2000));

                menu.subsub_menu = 0;
                menu_pages = menu_pages_keypad;
                sub_menu_count = sub_menu_count_keypad;
                cursor->y = 0;
                print_menu(*cursor);
                setcursor(1,0);

            }

        }

        else if (menu.sub_menu == 3){

            setcursor(0,1);
            clear_display();
            
            if (cursor->y == 0){
                settings_set_str("gpt_model", "gpt-3.5-turbo");
                print_line("GPT-3.5-turbo       ",2,cursor);
            }
            else{
                settings_set_str("gpt_model", "gpt-4o");
                print_line("GPT-4o              ",2,cursor);
            }

            print_line("Switched model to:  ",1,cursor);

            vTaskDelay(pdMS_TO_TICKS(3000));

            // Back to main menu

            menu.sub_menu = 0;
            menu.page = 0;
            cursor->y = 0;

            menu_pages = menu_pages_main;
            sub_menu_count = sub_menu_count_main;

            setcursor(1,0);
            print_menu(*cursor);
            
        }

        else if (menu.sub_menu == 6){

            if (cursor->y == 0){

                // Empty WIFI List + API Key + LittleFS index.html + Autooff Timer(default) + Model Settings(default)

                for (int i=0; i<=7; i++){
                    wifi_clear_credentials(i);
                }
                api_save_key("");                       
                settings_set_str("autooff","2");                    // default value
                settings_set_str("gpt_model","gpt-4o");             // default setting
                clear_html();
                

                clear_display();
                setcursor(0,1);

                print_line("All user information",1,cursor);
                print_line(" has been deleted!  ",2,cursor);

                vTaskDelay(pdMS_TO_TICKS(3000));

                clear_display();
                setcursor(1,0);

                menu.sub_menu = 0;
                menu.page = 0;
                cursor->y = 0;

                menu_pages = menu_pages_main;
                sub_menu_count = sub_menu_count_main;

                print_menu(*cursor);

            }
            
            else{

                clear_display();
                setcursor(0,1);

                print_line("  Operation aborted ",1,cursor);
                print_line("  by user           ",2,cursor);

                vTaskDelay(pdMS_TO_TICKS(3000));

                clear_display();
                setcursor(1,0);

                menu.sub_menu = 0;
                menu.page = 0;
                cursor->y = 0;

                menu_pages = menu_pages_main;
                sub_menu_count = sub_menu_count_main;

                print_menu(*cursor);

            }
        }

        else if (menu.sub_menu == 1){

            // Special case: enter settings scribble mode
            // the wifi that is edited is saved in subsub_menu

            menu.subsub_menu = ((cursor->y + 1) + (menu.page * 4));
            menu.page = 0;

            UI_mode = 's';
            insert_scribble_header('s');

            // Temporarily save the scribble page 
            // Empty the scribble page, print the scribble page

            memcpy(prompt_copy, scribble_page, scribble_page_length);

            for (int i=20;i < scribble_page_length; i++){

                scribble_page[i] = 0x20;  // Space characters everywhere
            }

            setcursor(1,1);
            print_scribble_page();
            initialize_cursor(cursor);

            return;

        }


    }

}

void handle_keyregister_html_view(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor){

    static uint8_t passage_selection_keys[9] = {'1','2','3','4','5','6','7','8','9'};

    if(((keyset[registerpointer[0]].event_key) == ENTER)){

        // Go back to scribble page

        UI_mode = 's';

        setcursor(1,1);

        print_scribble_page();
        initialize_cursor(cursor);

    }

    else if(((keyset[registerpointer[0]].event_key) == MENU)){

        // Go to menu

        UI_mode = 'm';
        setcursor(1,0);
        initialize_cursor(cursor);
        moveCursor_menu('u',cursor);
        menu = (Menu){'h',true,0,0,0};
        print_menu(*cursor);
        return;

    }

    else if (((keyset[registerpointer[0]].event_key) == UP) && (html_menu[html_text_passage] > 0 )){

        // Scroll upwards (back)

        html_menu[html_text_passage]--;

        // Print text at new position

        display_from_local_html(html_menu, html_text_passage, current_html_text,cursor);

    }

    else if (((keyset[registerpointer[0]].event_key) == DOWN) && (html_menu[html_text_passage] < max_sections_per_passage_html )){

        // Scroll downwards (forward)

        html_menu[html_text_passage]++;

        // Print text at new position

        display_from_local_html(html_menu, html_text_passage, current_html_text,cursor);

    }

    else if (((keyset[registerpointer[0]].event_key) == SHIFT)){

        if (((keyset[registerpointer[1]].event_key) == MENU)){

            // Turn the device off

            device_shutdown(*cursor);
            return;     

        }
    }

    else if ((keyset[registerpointer[0]].event_key) == EVENT_NONE){

        if (registerpointer[0] != 0){

            // Ignore Keyereleases 

            if (registerpointer[0] != registerpointer_old[0]){

                // Check if one of the number keys was pressed

                for (int i = 0; i <= 8; i++){

                    if ((keyset[registerpointer[0]].alpha_value) == passage_selection_keys[i]){

                        html_text_passage = i;

                        // Print text at new location

                        display_from_local_html(html_menu, html_text_passage, current_html_text,cursor);

                    }
                }

            }
        }

    }

}

// Stealth mode variables

bool stealth_shift_state = false;                                   // toggles everytime shift is pressed
char stealth_allowed_keys[17] = {'0','1','2','3','4','5','6','7','8','9','+','-','*','/','.','a','='};

char stealth_operation_line[21] = "                    ";           // only one line for operations
uint8_t stealth_cursor = 0;                                         // 1 dimensional (only one line)
float stealth_ans = 0.0f;
uint8_t stealth_ans_error = 0;

void print_stealth_operation_line(Cursor* cursor){

    // Print out the operation line after it got updated

    clear_display();
    print_line(stealth_operation_line,0,cursor);
    
}

void set_display_cursor(uint8_t x, uint8_t y){

    // Set DDRAM Address of DOGM204 to a specific location

    uint8_t print_location = 0x00;

    if ((x < 20) && (y < 4)){

        print_location = print_location + (0x20 * y);
        print_location = print_location + (0x01 * x);

        // Select DDRAM Address and write to it

        bitBang(0,0,(print_location + 0x80));
    }

}

void stealth_print_solution(Cursor cursor){

    // Examines the stealth operation line,
    // Calculates the solution if possible and prints it

    char operand1[21] = {0};
    char operand2[21] = {0};
    char operator = 0;
    const char *s = stealth_operation_line;
    int i = 0, j = 0;
    int found_operator = 0;

    // Find operator and operand 1 within operation line

    while (*s && !found_operator) {

        if (*s == ' ') {
            s++;
            continue;
        }

        // Check if its a sign or an operator (negative number)

        if ((*s == '+' || *s == '-' || *s == '*' || *s == '/') &&
            i > 0) {

            operator = *s;
            found_operator = 1;
            s++;
            break;
        }

        if (i < (int)sizeof(operand1) - 1) {
            operand1[i++] = *s;
        }
        s++;
    }

    operand1[i] = '\0';

    // Everything behind the operator sign is operand2

    while (*s) {

        if (*s == ' ') {
            s++;
            continue;
        }

        if (j < (int)sizeof(operand2) - 1) {
            operand2[j++] = *s;
        }
        s++;
    }

    operand2[j] = '\0';

    ESP_LOGI("Calc","Operand1: %s   Operand2: %s   Operator: %c", operand1, operand2, operator);

    // Strings to floats:

    char *prblm;
    char stealth_ans_string[21];
    
    float op_1 = strtof(operand1,&prblm);

    if (operand1 == prblm){

        if (operand1[0] == 'a'){

            op_1 = stealth_ans;     // previous answer

        }
        else{

            stealth_ans_error = 1;

        }

    }

    float op_2 = strtof(operand2,&prblm);

    if (operand2 == prblm){

        if(operand2[0] == 'a'){

            op_2 = stealth_ans;
        }

        else{

            stealth_ans_error = 1;

        }
    }

    // Make calculation, scan for errors

    if (!stealth_ans_error){

        switch (operator){

            case '+': stealth_ans = op_1 + op_2; break;
            case '-': stealth_ans = op_1 - op_2; break;
            case '*': stealth_ans = op_1 * op_2; break;
            case '/':
                if (op_2 == 0.0f){
                    stealth_ans_error = 1;
                }
                else{
                    stealth_ans = op_1 / op_2;
                }
                break;
            default: stealth_ans_error = 1;
        }
    }

    // If no error: print the solution

    char formatted_ans[21];

    if (!stealth_ans_error){

        snprintf(stealth_ans_string, sizeof(stealth_ans_string), "%.6g", stealth_ans);
        snprintf(formatted_ans, sizeof(formatted_ans), "%20s", stealth_ans_string);

    }

    else{

        strcpy(formatted_ans, "  Mathematical Error");
    }

    // Fill up answer string to 20 chars

    ESP_LOGI("Result","%s",formatted_ans);


    stealth_ans_error = 0;

    // Print solution or error message

    print_line(formatted_ans,3,&cursor);

}


void handle_key_register_stealth(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor){

    // Stealth mode: The device imitates the original calculator interface (only basic calculations possible)
    // To activate all the features the user has to hit a specific key combination!

    if ((keyset[registerpointer[0]].event_key) == EVENT_NONE){

        if (registerpointer[0] != 0){
            
            // Normal key has been pressed, only accept number keys or operand keys

            for (int i = 0; i < 16; i++){

                if ((keyset[registerpointer[0]].stealth_value == stealth_allowed_keys[i])){

                    // It was a number or an operator

                    stealth_operation_line[stealth_cursor] = keyset[registerpointer[0]].stealth_value;

                    print_stealth_operation_line(cursor);

                    if (stealth_cursor < 19){

                        stealth_cursor++;
                    }

                    set_display_cursor(stealth_cursor,0);

                }

            }

            if((keyset[registerpointer[0]].stealth_value == stealth_allowed_keys[16])){

                // User wants the solution displayed

                clear_display();
                stealth_print_solution(*cursor);
                stealth_cursor = 0;
                strcpy(stealth_operation_line,"                    ");
                set_display_cursor(stealth_cursor,0);

            }

            if (keyset[registerpointer[0]].normal_value == UNLCK1){

                if (keyset[registerpointer[1]].normal_value == UNLCK2){

                    // Correct unlock sequence
                    // Initialize scribble mode

                    setcursor(0,1);
                    clear_display();
                    print_start_screen(*cursor);
                    UI_mode = 's';
                    insert_scribble_header('c');
                    print_scribble_page();
                    initialize_cursor(cursor);
                    setcursor(1,1);

                }

            }

        }

    }

    else if (

            (keyset[registerpointer[0]].event_key) == RIGHT ||
            (keyset[registerpointer[0]].event_key) == LEFT)
            
            {
                // Allow navigation in stealth mode, only within already written signs

                if (((keyset[registerpointer[0]].normal_value) == 'r') && (stealth_cursor < 19) && (stealth_operation_line[stealth_cursor+2] != ' ')){
                    stealth_cursor++;
                }
                else if (((keyset[registerpointer[0]].normal_value) == 'l') && (stealth_cursor > 0) && (stealth_operation_line[stealth_cursor-1] != ' ')){
                    stealth_cursor--;
                }

                set_display_cursor(stealth_cursor,0);
            }

    else if(((keyset[registerpointer[0]].event_key) == ENTER)){

        // delete all numbers and operator signs
        // or shutdown (depending on shift_state)

        if (stealth_shift_state == true){

            device_shutdown(*cursor);
        }

        else{

            clear_display();
            stealth_cursor = 0;
            set_display_cursor(stealth_cursor,0);
            strcpy(stealth_operation_line,"                    ");
        }

    }

    else if (((keyset[registerpointer[0]].event_key) == SHIFT)){

        // toggle shift_state
        stealth_shift_state = !stealth_shift_state;

    }


}

void handle_keyregister_keypad(uint8_t* registerpointer, uint8_t* registerpointer_old, Cursor* cursor){

    if ((keyset[registerpointer[0]].event_key) == EVENT_NONE){

        if (registerpointer[0] != 0){

            if (keypad_mode == 'u'){

                // Normal value of key gets sent via USB HID interface
                hid_send_char(keyset[registerpointer[0]].normal_value);

            }
            
        }
    }
    else if ((keyset[registerpointer[0]].event_key) == SHIFT){

        if (registerpointer[1] != 0){

            if (keypad_mode == 'u'){

                hid_send_char(keyset[registerpointer[1]].shift_value);
            }
        }

    }
    else if ((keyset[registerpointer[0]].event_key) == ALPHA){

        if (registerpointer[1] != 0){

            if (keypad_mode == 'u'){

                hid_send_char(keyset[registerpointer[1]].alpha_value);
            }
        }
    }
    else if ((keyset[registerpointer[0]].event_key) == MENU){

        if (keypad_mode == 'u'){

            setcursor(0,1);
            clear_display();

            print_line("     REBOOTING      ",1,cursor);
            print_line("    Please Wait     ",2,cursor);

            vTaskDelay(pdMS_TO_TICKS(2000));

            clear_display();

            toggle_usb_mode(*cursor);      // restart in normal mode again!

        }


    }

    else if (

            (keyset[registerpointer[0]].event_key) == UP ||
            (keyset[registerpointer[0]].event_key) == DOWN || 
            (keyset[registerpointer[0]].event_key) == RIGHT ||
            (keyset[registerpointer[0]].event_key) == LEFT){

                hid_send_special_key(special_hid_LUT[keyset[registerpointer[0]].normal_value]);
            }



    else if ((keyset[registerpointer[0]].event_key) == ENTER){

        hid_send_special_key(special_hid_LUT[keyset[registerpointer[0]].normal_value]);

    }

    else if ((keyset[registerpointer[0]].event_key) == BACK){

        hid_send_special_key(special_hid_LUT[keyset[registerpointer[0]].normal_value]);

    }

}