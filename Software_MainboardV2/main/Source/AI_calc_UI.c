/*

ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

UI.c: UI-Functions and Variables

*/

// #region Includes

/*
#########################################################################################
##                                                                                     ##
##   Includes                                                                          ##
##                                                                                     ##
#########################################################################################
*/

#include "esp_littlefs.h"
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <math.h>
#include "freertos/FreeRTOS.h"

#include "AI_calc_UI.h"
#include "AI_calc_LLMs.h"
#include "AI_calc_device.h"
#include "AI_calc_battery.h"
#include "AI_calc_network.h"
#include "AI_calc_maindisplay.h"
#include "AI_calc_sidedisplay.h"

// #endregion

// #region Generic static UI Variables

/*
#########################################################################################
##                                                                                     ##
##   Generic static UI Variables                                                       ##
##                                                                                     ##
#########################################################################################
*/

static UI_TypeDef UI;
static nvs_handle_t ui_settings_handle;

static TaskHandle_t ui_sidetask_handle;

// Needed for autooff timer in sidetask
static uint8_t user_activity_sensed = 0;

// Needed when saving wifi credentials
char wifi_ssid_temp[WIFI_MAX_SSID_LENGTH+1];
char wifi_pass_temp[WIFI_MAX_PASSW_LENGTH+1];
uint8_t wifi_index_temp;

// Needed when saving API key
ai_model_TypeDef api_key_for_temp;

// A handful of often-used info texts
static char* ui_unlock_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    "     Starting AI    ",
    "    Calculator UI   ",
    "===================="
};
static char* feature_missing_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    "   This feature     ",
    " is not implemented ",
    "===================="
};
static char* ai_model_not_supported_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    "  Please select a   ",
    " different AI model ",
    "===================="
};
static char* taking_pic_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    "      Taking        ",
    "      Picture       ",
    "===================="
};
static char* chatview_mode_enter_fail_instead_scribble_enter_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    " Can't go to chat   ",
    " No chat files saved",
    "===================="
};
static char* wifi_saved_success_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    "  Wifi credentials  ",
    " saved successfully ",
    "===================="
};
static char* wifi_saved_fail_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    " Error while trying ",
    "    to save wifi    ",
    "===================="    
};
static char* apikey_saved_success_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    "   API Key saved    ",
    "   successfully     ",
    "===================="
};
static char* apikey_saved_fail_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    " Error while trying ",
    "  to save API Key   ",
    "===================="
};
static char* pref_wifi_saved_success_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    " Set preferred wifi ",
    "   successfully     ",
    "===================="
};
static char* pref_wifi_saved_fail_msg[MAIN_DISPLAY_ROWS] = {
    "====================",
    " Error while trying ",
    " set preferred wifi ",
    "===================="    
};


// #endregion

// #region Calculatormode static variables

/*
#########################################################################################
##                                                                                     ##
##   Calculatormode static variables                                                   ##
##                                                                                     ##
#########################################################################################
*/

static calculator_mode_TypeDef calculatormode;

// #endregion

// #region Scribblemode static variables

/*
#########################################################################################
##                                                                                     ##
##   Scribblemode static variables                                                     ##
##                                                                                     ##
#########################################################################################
*/

static scribble_mode_typeDef scribblemode;

// #endregion

// #region Textinputmode static variables

/*
#########################################################################################
##                                                                                     ##
##   Textinputmode static variables                                                    ##
##                                                                                     ##
#########################################################################################
*/

static text_input_mode_typeDef textinputmode;

// Textinput callbacks forward declarations
void textinput_cb_1(void);
void textinput_cb_2(void);
void textinput_cb_3(void);

// Textinput pages forward declarations
textinput_page_TypeDef enter_ssid_page;
textinput_page_TypeDef enter_passw_page;
textinput_page_TypeDef enter_api_key_page;

// #endregion

// #region Chatviewmode static variables

/*
#########################################################################################
##                                                                                     ##
##   Chatviewmode static variables                                                     ##
##                                                                                     ##
#########################################################################################
*/

static chatview_mode_typeDef chatviewmode;

// #endregion

// #region Menumode static variables

/*
#########################################################################################
##                                                                                     ##
##   Menumode static variables                                                         ##
##                                                                                     ##
#########################################################################################
*/

static menu_mode_typeDef menumode;

// Menues forward declarations (pre prompt menues)
menu_entry_TypeDef pre_prompt_settings_menu[];
menu_entry_TypeDef pre_prompt_cancel_menu[];
menu_entry_TypeDef pre_prompt_camera_menu[];
menu_entry_TypeDef pre_prompt_confirm_menu[];

// Menu callbacks forward declarations (pre prompt related)
void menu_cb_1(void);
void menu_cb_2(void);
void menu_cb_3(void);
void menu_cb_4(void);
void menu_cb_5(void);
void menu_cb_6(void);
void menu_cb_7(void);
void menu_cb_8(void);
void menu_cb_9(void);

// Menues forward declarations (main menu tree)
menu_entry_TypeDef main_menu[];
menu_entry_TypeDef wifi_main_menu[];
menu_entry_TypeDef ai_main_menu[];
menu_entry_TypeDef filesys_main_menu[];
menu_entry_TypeDef ui_main_menu[];
menu_entry_TypeDef device_main_menu[];
menu_entry_TypeDef debugging_main_menu[];

menu_entry_TypeDef wifis_edit_menu[WIFI_MAX_STORED_LOGINDATA+1];
menu_entry_TypeDef wifis_man_choose_menu[WIFI_MAX_STORED_LOGINDATA+1];

menu_entry_TypeDef chose_ai_menu[];
menu_entry_TypeDef choose_openai_ver_menu[];
menu_entry_TypeDef api_select_ai_menu[];

menu_entry_TypeDef autooff_settings_menu[];
menu_entry_TypeDef unlck_change_conf_menu[];

menu_entry_TypeDef display_contr_set_menu[];
menu_entry_TypeDef camera_settings_menu[];
menu_entry_TypeDef set_framesize_menu[];

menu_entry_TypeDef factory_rst_conf_menu[];
menu_entry_TypeDef enter_debug_conf_menu[];

// Menu callback forward declarations (main menu related)
void menu_cb_10(void);
void menu_cb_11(void);
void menu_cb_12(void);
void menu_cb_13(void);
void menu_cb_14(void);
void menu_cb_15(void);
void menu_cb_16(void);
void menu_cb_17(void);
void menu_cb_18(void);
void menu_cb_19(void);
void menu_cb_20(void);
void menu_cb_21(void);
void menu_cb_22(void);
void menu_cb_23(void);
void menu_cb_24(void);
void menu_cb_25(void);
void menu_cb_26(void);
void menu_cb_27(void);
void menu_cb_28(void);
void menu_cb_29(void);
void menu_cb_30(void);
void menu_cb_31(void);
void menu_cb_32(void);
void menu_cb_33(void);
void menu_cb_34(void);
void menu_cb_35(void);
void menu_cb_36(void);
void menu_cb_37(void);
void menu_cb_38(void);
void menu_cb_39(void);
void menu_cb_40(void);
void menu_cb_41(void);
void menu_cb_42(void);
void menu_cb_43(void);
void menu_cb_44(void);
void menu_cb_45(void);
void menu_cb_46(void);
void menu_cb_47(void);
void menu_cb_48(void);
void menu_cb_49(void);
void menu_cb_50(void);


// #endregion

// #region Fileviewmode static variables

/*
#########################################################################################
##                                                                                     ##
##   Fileview mode static variables                                                    ##
##                                                                                     ##
#########################################################################################
*/

// #endregion

// #region Generic static function declarations

/*
#########################################################################################
##                                                                                     ##
##   Generic static function declarations                                              ##
##                                                                                     ##
#########################################################################################
*/

static void nvs_get_ui_infos(void);
static void nvs_save_ui_infos(void);

static void UI_sidetask(void* arg);
static esp_err_t UI_send_prompt(char* scribble_page_copy, char (*pictures)[64]);

// #endregion

// #region Calculatormode static function declarations

/*
#########################################################################################
##                                                                                     ##
##   Calculatormode static function declarations                                       ##
##                                                                                     ##
#########################################################################################
*/

static void calculatormode_init(calculator_mode_TypeDef* calculator);
static void handle_pressed_keys_calc(Key_TypeDef* cur_pressed_keys);
static void calcmode_calculate(char* equation,char* solution,float* solution_float,uint8_t* ans_valid,float prev_ans);

// #endregion

// #region Scribblemode static function declarations

/*
#########################################################################################
##                                                                                     ##
##   Scribblemode static function declarations                                         ##
##                                                                                     ##
#########################################################################################
*/

static void scribble_page_inject_at_cursor_pos(char sign);
static void scribble_page_delete_at_cursor_pos(void);
static void scribble_mode_reset(scribble_mode_typeDef* scribble);
static void scribble_page_print_at_cursor_pos(void);
static void scribble_page_adjust_display_cursor(void);
static void scribble_mode_move_cursor(Key_TypeDef* navigation_key);
static void handle_pressed_keys_scribble(Key_TypeDef* cur_pressed_keys);

// #endregion

// #region Textinputmode static function declarations

/*
#########################################################################################
##                                                                                     ##
##   Textinputmode static function declarations                                        ##
##                                                                                     ##
#########################################################################################
*/

static void enter_textinput_mode(textinput_page_TypeDef* goto_page);
static void print_textinput_page_at_cursor(void);
static void textinput_page_inject_at_cursor(char sign);
static void textinput_page_delete_at_cursor(void);
static void handle_pressed_keys_textinput(Key_TypeDef* cur_pressed_keys);

// #endregion

// #region Chatviewmode static function declarations

/*
#########################################################################################
##                                                                                     ##
##   Chatviewmode static function declarations                                         ##
##                                                                                     ##
#########################################################################################
*/

static esp_err_t maybe_enter_chatview(void);
static void chatview_load_and_format_response(void);
static void chatview_load_and_format_prompt(void);
static void chatview_print_chatitem_at_cursor(void);
static void handle_pressed_keys_chatview(Key_TypeDef* cur_pressed_keys);

// #endregion

// #region Menumode static function declarations

/*
#########################################################################################
##                                                                                     ##
##   Menumode static function declarations                                             ##
##                                                                                     ##
#########################################################################################
*/

static void enter_menu(menu_entry_TypeDef* menu, uint8_t menu_key_to_leave_allowed, UI_mode_TypeDef opened_from);
static void menu_mode_print_at_cursor_pos(void);
static void handle_pressed_keys_menu(Key_TypeDef* cur_pressed_keys);

// #endregion

// #region Fileviewmode static function declarations

/*
#########################################################################################
##                                                                                     ##
##   Fileviewmode static function declarations                                         ##
##                                                                                     ##
#########################################################################################
*/

static void handle_pressed_keys_fileview(Key_TypeDef* cur_pressed_keys);

// #endregion

// #region LittleFS static function declarations

/*
#########################################################################################
##                                                                                     ##
##   LittleFS static function declarations                                             ##
##                                                                                     ##
#########################################################################################
*/

static void littleFS_init(void);

// #endregion

// #region Generic static UI functions

/*
#########################################################################################
##                                                                                     ##
##   Generic static UI functions                                                       ##
##                                                                                     ##
#########################################################################################
*/

static void nvs_get_ui_infos(void){

    // Called during UI init
    ESP_ERROR_CHECK(nvs_open("ui",NVS_READWRITE,&ui_settings_handle));
    // Get Minutes until auto shutdown
    if (nvs_get_u8(ui_settings_handle,"autooff_mins",&UI.autooff_tresh_mins) != ESP_OK){
        UI.autooff_tresh_mins = UI_DEFAULT_AUTOOFFMINS;
    }
    // Get Current AI model
    uint8_t model_buf;
    if (nvs_get_u8(ui_settings_handle,"ai_model",&model_buf) != ESP_OK){
        UI.current_ai_model = UI_DEFAULT_AI_MODEL;
    }
    else{
        UI.current_ai_model = (ai_model_TypeDef)model_buf;
    }
    // Get "Delete images after sending" setting
    if (nvs_get_u8(ui_settings_handle,"delete_imgs",&UI.delete_imgs_after_sending) != ESP_OK){
        UI.delete_imgs_after_sending = UI_DEFAULT_DELETE_IMGS_AFTER_SENDING;
    }
    // Get AI model versions
    size_t version_name_length = sizeof(UI.openai_model_version);
    if (nvs_get_str(ui_settings_handle,"openai_ver",UI.openai_model_version,&version_name_length)!=ESP_OK){
        strcpy(UI.openai_model_version,UI_DEFAULT_OPENAI_MODEL);
    }
    version_name_length = sizeof(UI.gemini_model_version);
    if (nvs_get_str(ui_settings_handle,"gemini_ver",UI.gemini_model_version,&version_name_length)!=ESP_OK){
        strcpy(UI.gemini_model_version,UI_DEFAULT_GEMINI_MODEL);
    }
    version_name_length = sizeof(UI.claude_model_version);
    if (nvs_get_str(ui_settings_handle,"claude_ver",UI.claude_model_version,&version_name_length)!=ESP_OK){
        strcpy(UI.claude_model_version,UI_DEFAULT_CLAUDE_MODEL);
    }
    // Get UI unlock code
    size_t unlock_code_length = sizeof(UI.unlock_code);
    if (nvs_get_str(ui_settings_handle,"unlock_code",UI.unlock_code,&unlock_code_length)!=ESP_OK){
        strcpy(UI.unlock_code,UI_DEFAULT_UNLOCK_CODE);
    }
}

static void nvs_save_ui_infos(void){

    // Called during shutdown, potential errors not checked, default values are
    // selected in nvs_get_ui_infos in case of an error
    nvs_set_u8(ui_settings_handle,"autooff_mins",(uint8_t)UI.autooff_tresh_mins);
    nvs_set_u8(ui_settings_handle,"ai_model",UI.current_ai_model);
    nvs_set_u8(ui_settings_handle,"delete_imgs",UI.delete_imgs_after_sending);
    nvs_set_str(ui_settings_handle,"openai_ver",UI.openai_model_version);
    nvs_set_str(ui_settings_handle,"gemini_ver",UI.gemini_model_version);
    nvs_set_str(ui_settings_handle,"claude_ver",UI.claude_model_version);
    nvs_set_str(ui_settings_handle,"unlock_code",UI.unlock_code);
    nvs_commit(ui_settings_handle);
}

static void UI_sidetask(void* arg){

    /* 
    This task runs parallel to the app_main, for UI related actions that
    have to be performed every once in a while / asynchronous to keypad:
    - increment autooff timer
    - refresh battery information / wifi informations / flash information...
    */
    uint16_t seconds_since_last_user_activity = 0;
    while(1){
        vTaskDelay(pdMS_TO_TICKS(UI_SIDETASK_LOOP_DELAYTIME));
        if (user_activity_sensed){
            // Reset autooff timer
            user_activity_sensed = 0;
            seconds_since_last_user_activity = 0;
        }
        else{
            // Increment autoofftimer
            seconds_since_last_user_activity += (UI_SIDETASK_LOOP_DELAYTIME/1000);
        }
        if (seconds_since_last_user_activity > (uint16_t)(UI.autooff_tresh_mins*60)){
            // Shutdown device due to user inactivity
            nvs_save_ui_infos();
            powerlatch_shutdown();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        // Status screen refreshing
        bms_typeDef battery_info;
        get_bms_state(&battery_info);
        wifi_manager_TypeDef wifi_info;
        get_wifi_state(&wifi_info);
        UI_TypeDef ui;
        get_UI_state(&ui);
        dep128064_refresh_status_screen(&battery_info, &wifi_info,&ui);
    }
}

static esp_err_t UI_send_prompt(char* scribble_page_copy, char (*pictures)[64]){

    // Central function for sending prompts
    // Returns ESP_FAIL if AI model is not supported, in case of other errors, an error message is in littleFS response
    char answer_buf[ANSWER_PAGE_LENGTH+1];
    switch (UI.current_ai_model){
        case AI_MODEL_OPENAI:
            dogm204_start_loading_screen(" Waiting for OpenAI ",150);
            if (scribblemode.start_new_chat){
                start_openai_conversation(scribble_page_copy,answer_buf,sizeof(answer_buf),pictures,UI.openai_model_version);
            }
            else{
                get_openai_response(scribble_page_copy,answer_buf,sizeof(answer_buf),pictures,UI.openai_model_version);
            }
            // ESP_LOGI("UI-Test","OpenAI response: %s", answer_buf);
            dogm204_end_loading_screen();
            return ESP_OK;
        case AI_MODEL_GEMINI:
            // Not implemented yet, go back to scribblemode
            return ESP_FAIL;
        case AI_MODEL_CLAUDE:
            // Not implemented yet, go back to scribblemode
            return ESP_FAIL;
    }
    return ESP_FAIL;
}


// #endregion

// #region Calculatormode static functions

/*
#########################################################################################
##                                                                                     ##
##    Calculatormode static functions                                                  ##
##                                                                                     ##
#########################################################################################
*/

static void calculatormode_init(calculator_mode_TypeDef* calculator){

    // Initial values upon entering (only upon device start)
    strcpy(calculator->equation_page,EMPTY_LINE);
    strcpy(calculator->solution,EMPTY_LINE);
    calculator->calc_cursor_pos = 0;
    calculator->shift_active = 0;
    calculator->equation_len = 0;
    calculator->previous_result_valid = 0;
    // Cursor on in maindisplay
    dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
}

static void handle_pressed_keys_calc(Key_TypeDef* cur_pressed_keys){

    if (cur_pressed_keys[0].special_function == KEY_SHIFT_SPECIAL_FUNC){
        // toggle shift just like in original calculator
        calculatormode.shift_active = (calculatormode.shift_active == 0);
    }
    else if (cur_pressed_keys[0].special_function == KEY_ENTER_SPECIAL_FUNC){
        if (calculatormode.shift_active){
            // turn off device
            dogm204_clear_screen();
            nvs_save_ui_infos();
            powerlatch_shutdown_immediately();      // no shutdown message
            return;
        }
        else{
            // delete equations and solution, reset cursor (AC-Key)
            strcpy(calculatormode.equation_page,EMPTY_LINE);
            strcpy(calculatormode.solution,EMPTY_LINE);
            calculatormode.calc_cursor_pos = 0;
            calculatormode.equation_len = 0;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_BACK_SPECIAL_FUNC){
        // delete from equation, shift everything to the left
        for (uint8_t i=calculatormode.calc_cursor_pos; i<(MAIN_DISPLAY_COLUMNS-1); i++){
            calculatormode.equation_page[i] = calculatormode.equation_page[i+1];
        }
        calculatormode.equation_page[MAIN_DISPLAY_COLUMNS-1] = ' ';
        if (calculatormode.equation_len > 0){
            calculatormode.equation_len--;
        }
        if (calculatormode.calc_cursor_pos > 0){
            calculatormode.calc_cursor_pos--;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_RIGHT_SPECIAL_FUNC){
        // navigate in equation
        if (calculatormode.calc_cursor_pos <= calculatormode.equation_len){
            calculatormode.calc_cursor_pos++;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_LEFT_SPECIAL_FUNC){
        // navigate in equation
        if (calculatormode.calc_cursor_pos > 0){
            calculatormode.calc_cursor_pos--;
        }
    }
    else if (cur_pressed_keys[0].calculatormode_meaning == '='){
        // check first if UI got unlocked
        if (!strcmp(calculatormode.equation_page,UI.unlock_code)){
            // UI got unlocked, switch to scribble mode
            dogm204_display_control(DOGM204_CURSOR_OFF_BIT|DOGM204_CURSOR_NO_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
            dogm204_print_screen(ui_unlock_msg);
            vTaskDelay(pdMS_TO_TICKS(1000));
            dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
            UI.UI_mode = UI_MODE_SCRIBBLE;
            scribble_mode_reset(&scribblemode);
            scribble_page_print_at_cursor_pos();
            // Turn on sidedisplay
            device.side_display_on = 1;
            dep128064_power_ctrl(device.side_display_on);
            return;
        }
        // Terminate equation string
        calculatormode.equation_page[calculatormode.equation_len] = '\0';
        // calculate solution
        float previous_answer = calculatormode.previous_result;
        // ESP_LOGI("Calculatormode","Calculating: |%s|",calculatormode.equation_page);
        calcmode_calculate( calculatormode.equation_page,
                            calculatormode.solution,
                            &calculatormode.previous_result,
                            &calculatormode.previous_result_valid,
                            previous_answer);
        // delete equation page
        strcpy(calculatormode.equation_page,EMPTY_LINE);
        calculatormode.equation_len = 0;
        calculatormode.calc_cursor_pos = 0;
    }
    else if (cur_pressed_keys[0].calculatormode_meaning != '\0'){
        // shift everything to the right
        for (uint8_t i=(MAIN_DISPLAY_COLUMNS-1); i>calculatormode.calc_cursor_pos; i--){
            calculatormode.equation_page[i] = calculatormode.equation_page[i-1];
        }
        // add (valid) sign to equation
        calculatormode.equation_page[calculatormode.calc_cursor_pos] = cur_pressed_keys[0].calculatormode_meaning;
        if (calculatormode.calc_cursor_pos < (MAIN_DISPLAY_COLUMNS-1)){
            calculatormode.calc_cursor_pos++;
            calculatormode.equation_len++;
        }
    }
    // At the end, refresh the main display
    char* calc_screen[MAIN_DISPLAY_ROWS] = {
        calculatormode.equation_page,
        EMPTY_LINE,
        EMPTY_LINE,
        calculatormode.solution
    };
    dogm204_print_screen(calc_screen);
    dogm204_set_cursor_position((calculatormode.calc_cursor_pos+1),1);
}

static void calcmode_calculate(char* equation,char* solution,float* solution_float,uint8_t* ans_valid,float prev_ans){

    float values[MAIN_DISPLAY_COLUMNS + 1];
    char operators[MAIN_DISPLAY_COLUMNS + 1];
    uint8_t value_count = 0;
    uint8_t operator_count = 0;
    char* p = equation;
    // Parse numbers and operators
    while (*p != '\0' && value_count < MAIN_DISPLAY_COLUMNS){
        char number_str[MAIN_DISPLAY_COLUMNS + 1];
        uint8_t number_len = 0;
        // Read number / previous answer
        if (*p == 'a'){
            if (!(*ans_valid)){
                *ans_valid = 0;
                strcpy(solution,"         Math. error");
                *solution_float = 0.0f;
                return;
            }
            values[value_count++] = prev_ans;
            p++;
        }
        else{
            // Number may contain digits and comma
            while ((*p >= '0' && *p <= '9') || *p == ','){
                if (number_len < MAIN_DISPLAY_COLUMNS){
                    number_str[number_len++] = *p;
                }
                p++;
            }
            number_str[number_len] = '\0';
            if (number_len == 0){
                *ans_valid = 0;
                strcpy(solution,"         Math. error");
                *solution_float = 0.0f;
                return;
            }
            // strtof expects '.' as decimal separator
            for (uint8_t i = 0; i < number_len; i++){
                if (number_str[i] == ',')
                    number_str[i] = '.';
            }
            char* endptr;
            float value = strtof(number_str, &endptr);
            if (*endptr != '\0'){
                *ans_valid = 0;
                strcpy(solution,"         Math. error");
                *solution_float = 0.0f;
                return;
            }
            values[value_count++] = value;
        }
        // Read operator
        if (*p != '\0'){
            if (*p != '+' && *p != '-' && *p != '*' && *p != '/'){
                *ans_valid = 0;
                strcpy(solution,"         Math. error");
                *solution_float = 0.0f;
                return;
            }
            operators[operator_count++] = *p;
            p++;
        }
    }
    if (value_count == 0 || value_count != operator_count + 1){
        *ans_valid = 0;
        strcpy(solution,"         Math. error");
        *solution_float = 0.0f;
        return;
    }
    // First resolve * and /
    for (int i = 0; i < operator_count; i++){
        if (operators[i] == '*' || operators[i] == '/'){
            if (operators[i] == '/'){
                if (values[i + 1] == 0.0f){
                    *ans_valid = 0;
                    strcpy(solution,"         Math. error");
                    *solution_float = 0.0f;
                    return;
                }
                values[i] /= values[i + 1];
            }
            else{
                values[i] *= values[i + 1];
            }
            // Remove value i+1 and operator i
            for (int j = i + 1; j < value_count - 1; j++){
                values[j] = values[j + 1];
            }
            for (int j = i; j < operator_count - 1; j++){
                operators[j] = operators[j + 1];
            }
            value_count--;
            operator_count--;
            i--;
        }
    }
    // Resolve + and -
    float result = values[0];
    for (int i = 0; i < operator_count; i++)
    {
        if (operators[i] == '+')
            result += values[i + 1];
        else
            result -= values[i + 1];
    }
    *solution_float = result;
    // Check for invalid floating point result
    if (!isfinite(result)){
        *ans_valid = 0;
        strcpy(solution,"         Math. error");
        *solution_float = 0.0f;
        return;
    }
    // Convert result to string
    // %.7g gives a compact representation without unnecessary zeros.
    char result_string[MAIN_DISPLAY_COLUMNS + 1];
    snprintf(result_string, sizeof(result_string), "%.7g", result);
    // Replace decimal point with comma
    for (uint8_t i = 0; result_string[i] != '\0'; i++){
        if (result_string[i] == '.') {
            result_string[i] = ',';
        }
    }
    // Convert to right-aligned string
    snprintf(solution, MAIN_DISPLAY_COLUMNS + 1,"%*s", MAIN_DISPLAY_COLUMNS, result_string);
}

// #endregion

// #region Scribblemode static functions

/*
#########################################################################################
##                                                                                     ##
##   Scribblemode static functions                                                     ##
##                                                                                     ##
#########################################################################################
*/

static void scribble_mode_reset(scribble_mode_typeDef* scribble){

    for(uint16_t i=0; i<SCRIBBLE_PAGE_LENGTH; i++){
        scribblemode.scribble_page[i] = ' ';
    }
    scribblemode.scribble_page_length = 0;
    scribblemode.scribble_cursor_pos = 0;
}

static void scribble_page_inject_at_cursor_pos(char sign){

    if (scribblemode.scribble_page_length < SCRIBBLE_PAGE_LENGTH){
        // Shift everything to the right behind injection point
        for (uint16_t i=(SCRIBBLE_PAGE_LENGTH-1); i>scribblemode.scribble_cursor_pos;i--){
            scribblemode.scribble_page[i]=scribblemode.scribble_page[i-1];
        }
        scribblemode.scribble_page_length++;
    }
    // Print sign at cursor_pos
    scribblemode.scribble_page[scribblemode.scribble_cursor_pos] = sign;
    if (scribblemode.scribble_cursor_pos < (SCRIBBLE_PAGE_LENGTH-1)){
        scribblemode.scribble_cursor_pos++;
    }
}

static void scribble_page_delete_at_cursor_pos(void){
        
    scribblemode.scribble_page[scribblemode.scribble_cursor_pos] = ' ';
    // Shift everything to the right of injection point to the left
    for (uint16_t i=scribblemode.scribble_cursor_pos; i<(SCRIBBLE_PAGE_LENGTH-1); i++){
        scribblemode.scribble_page[i]=scribblemode.scribble_page[i+1];
    }
    scribblemode.scribble_page[SCRIBBLE_PAGE_LENGTH-1]=' ';
    if (scribblemode.scribble_cursor_pos > 0){
        scribblemode.scribble_cursor_pos--;
    }
    if (scribblemode.scribble_page_length > 0){
        scribblemode.scribble_page_length--;
    }
}

static void scribble_page_print_at_cursor_pos(void){

    uint16_t print_segment = scribblemode.scribble_cursor_pos / MAIN_DISPLAY_CHARACTERS;
    char line1[MAIN_DISPLAY_COLUMNS+1];
    char line2[MAIN_DISPLAY_COLUMNS+1];
    char line3[MAIN_DISPLAY_COLUMNS+1];
    char line4[MAIN_DISPLAY_COLUMNS+1];
    char* new_screen[MAIN_DISPLAY_ROWS] = {line1,line2,line3,line4};
    for (uint8_t i=0; i<MAIN_DISPLAY_ROWS; i++){
        strncpy(new_screen[i],
                (scribblemode.scribble_page+(print_segment*MAIN_DISPLAY_CHARACTERS)+(MAIN_DISPLAY_COLUMNS*i)),
                MAIN_DISPLAY_COLUMNS);
        new_screen[i][MAIN_DISPLAY_COLUMNS] = '\0';
    }
    dogm204_print_screen(new_screen);
    // Adjust cursor on display
    scribble_page_adjust_display_cursor();
}

static void scribble_page_adjust_display_cursor(void){

    // Sets the display cursor correctly
    uint16_t print_segment = scribblemode.scribble_cursor_pos / MAIN_DISPLAY_CHARACTERS;
    uint8_t cursor_on_screen_pos = scribblemode.scribble_cursor_pos - (print_segment*MAIN_DISPLAY_CHARACTERS);
    uint8_t cursor_line = cursor_on_screen_pos / MAIN_DISPLAY_COLUMNS;
    uint8_t cursor_column = cursor_on_screen_pos - (cursor_line * MAIN_DISPLAY_COLUMNS);
    dogm204_set_cursor_position(cursor_column+1,cursor_line+1);     // +1 because arguments are 1-based
}

static void scribble_mode_move_cursor(Key_TypeDef* navigation_key){

    switch (navigation_key->special_function){
        case KEY_UP_SPECIAL_FUNC:
            if (scribblemode.scribble_cursor_pos >= MAIN_DISPLAY_COLUMNS){
                scribblemode.scribble_cursor_pos -= MAIN_DISPLAY_COLUMNS;
            }
            break;
        case KEY_DOWN_SPECIAL_FUNC:
            if ((scribblemode.scribble_cursor_pos+MAIN_DISPLAY_COLUMNS) <= scribblemode.scribble_page_length){
                scribblemode.scribble_cursor_pos += MAIN_DISPLAY_COLUMNS;
            }
            break;
        case KEY_RIGHT_SPECIAL_FUNC:
            if (scribblemode.scribble_cursor_pos < scribblemode.scribble_page_length){
                scribblemode.scribble_cursor_pos++;
            }
            break;
        case KEY_LEFT_SPECIAL_FUNC:
            if (scribblemode.scribble_cursor_pos > 0){
                scribblemode.scribble_cursor_pos--;
            }
            break;
        default:
            break;
    }
}

static void handle_pressed_keys_scribble(Key_TypeDef* cur_pressed_keys){

    if (cur_pressed_keys[0].special_function == KEY_SHIFT_SPECIAL_FUNC){
        if (cur_pressed_keys[1].special_function == KEY_MENU_SPECIAL_FUNC){
            // Shutdown
            nvs_save_ui_infos();
            powerlatch_shutdown();
            vTaskDelay(pdMS_TO_TICKS(1000));
            return;
        }
        else if (cur_pressed_keys[1].special_function == KEY_NO_SPECIAL_FUNC){
            scribble_page_inject_at_cursor_pos(cur_pressed_keys[1].shift_meaning);
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_MENU_SPECIAL_FUNC){
        // Enter menu mode (main menu)
        UI.UI_mode = UI_MODE_MENU;
        dogm204_clear_screen();
        enter_menu(main_menu, LEAVE_WITH_MENU_KEY_ALLOWED, UI_MODE_SCRIBBLE);
        return;
    }
    else if (cur_pressed_keys[0].special_function == KEY_ENTER_SPECIAL_FUNC){
        /*
        Enter menu mode, ask user: Want to append pictures? New conversation or
        stay in previous chat?
        After that enter chatview or go back to scribble mode
        */
        UI.UI_mode = UI_MODE_MENU;
        dogm204_clear_screen();
        enter_menu(pre_prompt_confirm_menu, LEAVE_WITH_MENU_KEY_DISABLED, UI_MODE_SCRIBBLE);
        return;
    }
    else if (cur_pressed_keys[0].special_function == KEY_BACK_SPECIAL_FUNC){
        scribble_page_delete_at_cursor_pos();
    }
    else if (cur_pressed_keys[0].special_function == KEY_ALT_SPECIAL_FUNC){
        if (cur_pressed_keys[1].special_function == KEY_NO_SPECIAL_FUNC){
            scribble_page_inject_at_cursor_pos(cur_pressed_keys[1].alt_meaning);
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_CUSTOM_1_SPECIAL_FUNC){
        // Take a picture
        dogm204_display_control(DOGM204_CURSOR_OFF_BIT|DOGM204_CURSOR_NO_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
        dogm204_print_screen(taking_pic_msg);
        camera_take_picture();
        get_saved_pics(&UI.cam_img_cnt);
        dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
    }
    else if (   
        (cur_pressed_keys[0].special_function == KEY_UP_SPECIAL_FUNC) ||
        (cur_pressed_keys[0].special_function == KEY_DOWN_SPECIAL_FUNC) ||
        (cur_pressed_keys[0].special_function == KEY_RIGHT_SPECIAL_FUNC) ||
        (cur_pressed_keys[0].special_function == KEY_LEFT_SPECIAL_FUNC)
            ){
        // Navigate with cursor
        scribble_mode_move_cursor(&cur_pressed_keys[0]);
    }
    else if (cur_pressed_keys[0].special_function == KEY_NO_SPECIAL_FUNC){
        scribble_page_inject_at_cursor_pos(cur_pressed_keys[0].normal_meaning);
    }
    // At the end, print scribble_page
    scribble_page_print_at_cursor_pos();
}

// #endregion

// #region Textinputmode static functions

/*
#########################################################################################
##                                                                                     ##
##   Textinputmode static functions                                                    ##
##                                                                                     ##
#########################################################################################
*/

static void enter_textinput_mode(textinput_page_TypeDef* goto_page){

    // Resets textinputmode, changes settings
    textinputmode.textinput_cursor_pos = 0;
    textinputmode.textinput_page_length = 0;
    for (uint16_t i=0; i<TEXTINPUT_PAGE_LENGTH; i++){
        textinputmode.textinput_page[i] = ' ';
    }
    textinputmode.textinput_page[TEXTINPUT_PAGE_LENGTH] = '\0';
    textinputmode.current_page = goto_page;
    // Turn on cursor
    dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
}

static void print_textinput_page_at_cursor(void){

    // Similar concept as scribble_page_print_at_cursor_pos()
    char line2[MAIN_DISPLAY_COLUMNS+1];
    strcpy(line2,textinputmode.current_page->command_prompt);
    // Find segment of textinput page to print
    uint16_t segment_to_print = textinputmode.textinput_cursor_pos / MAIN_DISPLAY_COLUMNS;
    char line3[MAIN_DISPLAY_COLUMNS+1];
    // Text visible
    strncpy(line3,textinputmode.textinput_page+(segment_to_print*MAIN_DISPLAY_COLUMNS),MAIN_DISPLAY_COLUMNS);
    // Replace characters with * if needed
    if (textinputmode.current_page->sensitive_information){
        uint8_t end_of_str_found = 0;
        for (int8_t i=(MAIN_DISPLAY_COLUMNS-1);i>=0; i--){
            if (end_of_str_found){
                line3[i] = '*';
                continue;
            }
            if (line3[i]!=' '){
                end_of_str_found = 1;
                line3[i] = '*';
            }
        }
    }
    line3[MAIN_DISPLAY_COLUMNS] = '\0';
    char* new_screen[MAIN_DISPLAY_ROWS]={
        EMPTY_LINE,
        line2,
        line3,
        EMPTY_LINE
    };
    dogm204_print_screen(new_screen);
    // Set the cursor correctly
    dogm204_set_cursor_position(((textinputmode.textinput_cursor_pos%MAIN_DISPLAY_COLUMNS)+1),3);
}

static void textinput_page_inject_at_cursor(char sign){

    // Essentially the same concept as scribble_page_inject_at_cursor_pos()
    if (textinputmode.textinput_page_length < TEXTINPUT_PAGE_LENGTH){
        // Shift everything to the righ of the cursor to the righr
        for (uint16_t i=(TEXTINPUT_PAGE_LENGTH-1); i>textinputmode.textinput_cursor_pos; i--){
            textinputmode.textinput_page[i] = textinputmode.textinput_page[i-1];
        }
        textinputmode.textinput_page_length++;
    }
    textinputmode.textinput_page[textinputmode.textinput_cursor_pos] = sign;
    if (textinputmode.textinput_cursor_pos < (TEXTINPUT_PAGE_LENGTH-1)){
        textinputmode.textinput_cursor_pos++;
    }
}

static void textinput_page_delete_at_cursor(void){

    // Nothing to delete
    if ((textinputmode.textinput_cursor_pos == 0) || (textinputmode.textinput_page_length == 0)){
        return;
    }
    // Move cursor one position to the left
    textinputmode.textinput_cursor_pos--;
    // Shift everything left
    for (uint16_t i=textinputmode.textinput_cursor_pos;i<(TEXTINPUT_PAGE_LENGTH-1);i++){
        textinputmode.textinput_page[i] = textinputmode.textinput_page[i+1];
    }
    // Clear last position
    textinputmode.textinput_page[TEXTINPUT_PAGE_LENGTH-1] = ' ';
    // One character less
    textinputmode.textinput_page_length--;
}

static void handle_pressed_keys_textinput(Key_TypeDef* cur_pressed_keys){

    if (cur_pressed_keys[0].special_function == KEY_SHIFT_SPECIAL_FUNC){
        if (cur_pressed_keys[1].special_function == KEY_MENU_SPECIAL_FUNC){
            // Shutdown
            nvs_save_ui_infos();
            powerlatch_shutdown();
            vTaskDelay(pdMS_TO_TICKS(1000));
            return;
        }
        else if (cur_pressed_keys[1].special_function == KEY_NO_SPECIAL_FUNC){
            // Inject to textinput page
            textinput_page_inject_at_cursor(cur_pressed_keys[1].shift_meaning);
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_MENU_SPECIAL_FUNC){
        // If there is a menu to return to, go there, do not perform callback function
        if (textinputmode.current_page->return_to_menu != NO_RETURN_TO_MENU){
            // Enter menu mode again (cancel textinput)
            UI.UI_mode = UI_MODE_MENU;
            dogm204_clear_screen();
            // Enter menu at return_to_menu, no other changes to menumode
            enter_menu(textinputmode.current_page->return_to_menu,menumode.leave_directly_permitted,menumode.opened_from_UI_mode);
            return;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_RIGHT_SPECIAL_FUNC){
        // Navigate in textinput page
        if (textinputmode.textinput_cursor_pos <= textinputmode.textinput_page_length){
            textinputmode.textinput_cursor_pos++;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_LEFT_SPECIAL_FUNC){
        // Navigate in textinput page
        if (textinputmode.textinput_cursor_pos > 0){
            textinputmode.textinput_cursor_pos--;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_ENTER_SPECIAL_FUNC){
        // First perform callback function if one exists
        if (textinputmode.current_page->enter_callback != NO_ENTER_CB){
            textinputmode.current_page->enter_callback();
        }
        // Goto the next textinput_page if one exists
        if (textinputmode.current_page->goto_page != NO_GOTO_PAGE){
            enter_textinput_mode(textinputmode.current_page->goto_page);
            print_textinput_page_at_cursor();
            return;
        }
        // If no textinput_page exists go back to menu mode
        if (textinputmode.current_page->return_to_menu != NO_RETURN_TO_MENU){
            UI.UI_mode = UI_MODE_MENU;
            dogm204_clear_screen();
            // No other changes to menu!
            enter_menu(textinputmode.current_page->return_to_menu,menumode.leave_directly_permitted,menumode.opened_from_UI_mode);
            return;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_ALT_SPECIAL_FUNC){
        if (cur_pressed_keys[1].special_function == KEY_NO_SPECIAL_FUNC){
            // Inject to textinput page
            textinput_page_inject_at_cursor(cur_pressed_keys[1].alt_meaning);
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_BACK_SPECIAL_FUNC){
        // Delete at cursor position
        textinput_page_delete_at_cursor();
    }
    else if (cur_pressed_keys[0].special_function == KEY_NO_SPECIAL_FUNC){
        // Inject at textinput page
        textinput_page_inject_at_cursor(cur_pressed_keys[0].normal_meaning);
    }
    // Lastly, print textinput page
    print_textinput_page_at_cursor();
}

// #endregion

// #region Textinput mode page definitions

/*
#########################################################################################
##                                                                                     ##
##   Textinput page definitions                                                        ##
##                                                                                     ##
#########################################################################################
*/

// Page                                       Input prompt        Sen. info           Go back to          Next page          Enter cb  
textinput_page_TypeDef enter_ssid_page =  {">Enter Wifi SSID    ",NO_SENSITIVE_INFO,  wifi_main_menu,     &enter_passw_page, textinput_cb_1};
textinput_page_TypeDef enter_passw_page=  {">Enter password     ",SENSITIVE_INFO,     wifi_main_menu,     NO_GOTO_PAGE,      textinput_cb_2};
textinput_page_TypeDef enter_api_key_page={">Enter API Key      ",NO_SENSITIVE_INFO,  api_select_ai_menu, NO_GOTO_PAGE,      textinput_cb_3};

// #endregion

// #region Textinputmode callbacks

/*
#########################################################################################
##                                                                                     ##
##   Textinput callbacks                                                               ##
##                                                                                     ##
#########################################################################################
*/

void textinput_cb_1(void){

    // Save WIFI ssid in buffer
    // Make sure string is not too long for buf
    textinputmode.textinput_page[WIFI_MAX_SSID_LENGTH]='\0';
    // Terminate string at the end of user input
    textinputmode.textinput_page[textinputmode.textinput_page_length] = '\0';
    strcpy(wifi_ssid_temp,textinputmode.textinput_page);
}

void textinput_cb_2(void){

    // Save Wifi SSID and passwort to wifi manager
    textinputmode.textinput_page[WIFI_MAX_PASSW_LENGTH]='\0';
    // Terminate string at the end of user input
    textinputmode.textinput_page[textinputmode.textinput_page_length] = '\0';
    strcpy(wifi_pass_temp,textinputmode.textinput_page);
    // Add to wifi manager
    // ESP_LOGI("Saving WIFI:","|%s|%s|",wifi_ssid_temp,wifi_pass_temp);
    esp_err_t ret = wifi_add_login_credentials(wifi_ssid_temp,wifi_pass_temp,wifi_index_temp);
    // Reset temporary buffers and textinputpage
    strcpy(wifi_ssid_temp,"");
    strcpy(wifi_pass_temp,"");
    for (uint16_t i=0; i<(TEXTINPUT_PAGE_LENGTH-1); i++){
        textinputmode.textinput_page[i]=' ';
    }
    // Display success message
    dogm204_display_control(DOGM204_CURSOR_OFF_BIT|DOGM204_CURSOR_NO_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
    if (ret == ESP_OK){
        dogm204_print_screen(wifi_saved_success_msg);
    }
    else{
        dogm204_print_screen(wifi_saved_fail_msg);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void textinput_cb_3(void){

    // Save API key
    ESP_LOGI("Test","Scribble(Pre): |%.80s|",scribblemode.scribble_page);
    textinputmode.textinput_page[(API_KEY_MAX_LENGTH>TEXTINPUT_PAGE_LENGTH)?TEXTINPUT_PAGE_LENGTH:API_KEY_MAX_LENGTH] = '\0';
    // Terminate string at the end of user input
    textinputmode.textinput_page[textinputmode.textinput_page_length] = '\0';
    // ESP_LOGI("UI","Saving API Key: |%s|",textinputmode.textinput_page);
    esp_err_t ret = save_API_Key(api_key_for_temp,textinputmode.textinput_page);
    // Reset textinput_page
    for (uint16_t i=0; i<(TEXTINPUT_PAGE_LENGTH-1); i++){
        textinputmode.textinput_page[i]=' ';
    }
    dogm204_display_control(DOGM204_CURSOR_OFF_BIT|DOGM204_CURSOR_NO_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
    if (ret == ESP_OK){
        dogm204_print_screen(apikey_saved_success_msg);
    }
    else{
        dogm204_print_screen(apikey_saved_fail_msg);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// #endregion

// #region Chatviewmode static functions

/*
#########################################################################################
##                                                                                     ##
##   Chatviewmode static functions                                                     ##
##                                                                                     ##
#########################################################################################
*/

static esp_err_t maybe_enter_chatview(void){

    // Returns ESP_FAIL if no text exchanges are saved atm, this needs to be dealt with externally!
    // Find out highest chat_position and go there
    uint16_t text_exchanges = openai_chat_dir_text_exchanges();
    if (text_exchanges == 0){
        return ESP_FAIL;
    }
    chatviewmode.highest_chat_position = (text_exchanges-1);
    chatviewmode.current_chat_position = chatviewmode.highest_chat_position;
    // Show response by defualt upon entering
    chatviewmode.show_response = 1;
    chatview_load_and_format_response();
    return ESP_OK;
}

static void chatview_load_and_format_response(void){

    // Load the text exchange where the user is atm
    load_prev_openai_response(chatviewmode.current_chat_position,chatviewmode.response_page,sizeof(chatviewmode.response_page));
    // ESP_LOGI("Test","Loaded response: %s",chatviewmode.response_page);
    chatviewmode.current_item_section = 0;
    // Find out how big the loaded chat item is
    uint16_t cur_item_len = strlen(chatviewmode.response_page);
    if (cur_item_len > 0){
        chatviewmode.current_item_highest_section =
            (cur_item_len - 1) / MAIN_DISPLAY_CHARACTERS;
    }
    else{
        chatviewmode.current_item_highest_section = 0;
    }
    uint8_t rest = (cur_item_len % MAIN_DISPLAY_CHARACTERS);
    if (rest != 0){
        // Fill up last text section with spaces so it fills up the entire screen
        for (uint16_t i=cur_item_len; i<((chatviewmode.current_item_highest_section+1)*MAIN_DISPLAY_CHARACTERS); i++){
            chatviewmode.response_page[i] = ' ';
        }
        // String terminate
        chatviewmode.response_page[(chatviewmode.current_item_highest_section+1)*MAIN_DISPLAY_CHARACTERS] = '\0';
    }
}

static void chatview_load_and_format_prompt(void){

    // Load text exchange prompt
    load_prev_openai_prompt(chatviewmode.current_chat_position,chatviewmode.prompt_page,sizeof(chatviewmode.prompt_page));
    // ESP_LOGI("Test","Loaded prompt: %s",chatviewmode.prompt_page);
    chatviewmode.current_item_section = 0;
    // Find out how big the loaded chat item is
    uint16_t cur_item_len = strlen(chatviewmode.prompt_page);
    if (cur_item_len > 0){
        chatviewmode.current_item_highest_section =
            (cur_item_len - 1) / MAIN_DISPLAY_CHARACTERS;
    }
    else{
        chatviewmode.current_item_highest_section = 0;
    }
    uint8_t rest = (cur_item_len % MAIN_DISPLAY_CHARACTERS);
    if (rest != 0){
        // Fill up last text section with spaces so it fills up the entire screen
        for (uint16_t i=cur_item_len; i<((chatviewmode.current_item_highest_section+1)*MAIN_DISPLAY_CHARACTERS); i++){
            chatviewmode.prompt_page[i] = ' ';
        }
        // String terminate
        chatviewmode.prompt_page[(chatviewmode.current_item_highest_section+1)*MAIN_DISPLAY_CHARACTERS] = '\0';
    }
}

static void chatview_print_chatitem_at_cursor(void){

    char line1[MAIN_DISPLAY_COLUMNS+1];
    char line2[MAIN_DISPLAY_COLUMNS+1];
    char line3[MAIN_DISPLAY_COLUMNS+1];
    char line4[MAIN_DISPLAY_COLUMNS+1];
    char* new_screen[MAIN_DISPLAY_ROWS] = {line1,line2,line3,line4};
    for (uint8_t i=0; i<MAIN_DISPLAY_ROWS; i++){
        if (chatviewmode.show_response){
            strncpy(new_screen[i],
                    (chatviewmode.response_page+(chatviewmode.current_item_section*MAIN_DISPLAY_CHARACTERS)+(i*MAIN_DISPLAY_COLUMNS)),
                    MAIN_DISPLAY_COLUMNS);
        }
        else{
            strncpy(new_screen[i],
                    (chatviewmode.prompt_page+(chatviewmode.current_item_section*MAIN_DISPLAY_CHARACTERS)+(i*MAIN_DISPLAY_COLUMNS)),
                    MAIN_DISPLAY_COLUMNS);
        }
        new_screen[i][MAIN_DISPLAY_COLUMNS] = '\0';
    }
    dogm204_print_screen(new_screen);
}

static void handle_pressed_keys_chatview(Key_TypeDef* cur_pressed_keys){

    if (cur_pressed_keys[0].special_function == KEY_SHIFT_SPECIAL_FUNC){
        if (cur_pressed_keys[1].special_function == KEY_MENU_SPECIAL_FUNC){
            // Shutdown
            nvs_save_ui_infos();
            powerlatch_shutdown();
            vTaskDelay(pdMS_TO_TICKS(1000));
            return;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_UP_SPECIAL_FUNC){
        // Navigate within current chat item
        if (chatviewmode.current_item_section > 0){
            chatviewmode.current_item_section--;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_DOWN_SPECIAL_FUNC){
        // Navigate within current chat item
        if (chatviewmode.current_item_section < chatviewmode.current_item_highest_section){
            chatviewmode.current_item_section++;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_LEFT_SPECIAL_FUNC){
        // Navigate between chat items
        if ((chatviewmode.current_chat_position > 0) && (!chatviewmode.show_response)){
            chatviewmode.current_chat_position--;
            chatviewmode.show_response = 1;
            chatview_load_and_format_response();
        }
        else if (chatviewmode.show_response){
            chatviewmode.show_response = 0;
            chatview_load_and_format_prompt();
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_RIGHT_SPECIAL_FUNC){
        // Navigate between chat items
        if ((chatviewmode.current_chat_position < chatviewmode.highest_chat_position) && (chatviewmode.show_response)){
            chatviewmode.current_chat_position++;
            chatviewmode.show_response = 0;
            chatview_load_and_format_prompt();
        }
        else if (!chatviewmode.show_response){
            chatviewmode.show_response = 1;
            chatview_load_and_format_response();
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_BACK_SPECIAL_FUNC){
        // Go back to scribble mode
        UI.UI_mode = UI_MODE_SCRIBBLE;
        dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
        scribble_page_print_at_cursor_pos();
        // Do not print in chatviewmode
        return;
    }
    else if (cur_pressed_keys[0].special_function == KEY_ENTER_SPECIAL_FUNC){
        // For toggling side display (can render equations too, not implemented yet)
    }
    else if (cur_pressed_keys[0].special_function == KEY_MENU_SPECIAL_FUNC){
        // Go to menu mode
        UI.UI_mode = UI_MODE_MENU;
        dogm204_clear_screen();
        enter_menu(main_menu, LEAVE_WITH_MENU_KEY_ALLOWED, UI_MODE_CHATVIEW);
        return;
    }
    // Lastly, print screen
    chatview_print_chatitem_at_cursor();
}

// #endregion

// #region Menumode static functions

/*
#########################################################################################
##                                                                                     ##
##   Menumode static functions                                                         ##
##                                                                                     ##
#########################################################################################
*/

static void enter_menu(menu_entry_TypeDef* menu, uint8_t menu_key_to_leave_allowed, UI_mode_TypeDef opened_from){

    // Do not use from within menu mode, only to enter
    menumode.current_menu = menu;
    menumode.menu_cursor_pos = 0;
    menumode.opened_from_UI_mode = opened_from;
    menumode.leave_directly_permitted = menu_key_to_leave_allowed;
    dogm204_display_control(DOGM204_CURSOR_OFF_BIT|DOGM204_CURSOR_NO_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
    menu_mode_print_at_cursor_pos();
}

static void menu_mode_print_at_cursor_pos(void){

    char menu_header[MAIN_DISPLAY_COLUMNS+1];
    // Menu title has to have MAIN_DISPLAY_COLUMS-1 letters
    snprintf(menu_header,sizeof(menu_header)," %s",menumode.current_menu->menu_title);
    // Insert style elements
    menu_header[0]=DOGM204_BALL_SIGN;
    // Choose needed entry texts for printing
    uint8_t menu_segment = menumode.menu_cursor_pos / (MAIN_DISPLAY_ROWS-1);
    char menu_entry_to_print1[MAIN_DISPLAY_COLUMNS+1];
    char menu_entry_to_print2[MAIN_DISPLAY_COLUMNS+1];
    char menu_entry_to_print3[MAIN_DISPLAY_COLUMNS+1];
    char* new_screen[MAIN_DISPLAY_ROWS] = {menu_header,menu_entry_to_print1,menu_entry_to_print2,menu_entry_to_print3};
    for (uint8_t i=1; i<MAIN_DISPLAY_ROWS; i++){
        new_screen[i][0] = ' ';
        strcpy(new_screen[i]+1,(menumode.current_menu+(menu_segment*(MAIN_DISPLAY_ROWS-1))+(i-1))->entry_text);
    }
    // Handle cases where not the entire screen is used
    for (uint i=1; i<MAIN_DISPLAY_ROWS; i++){
        if (strcmp(new_screen[i]," ")==0){
            for (uint8_t j=i; j<MAIN_DISPLAY_ROWS; j++){
                strcpy(new_screen[j],EMPTY_LINE);
            }
            break;
        }
    }
    // Set cursor
    new_screen[(menumode.menu_cursor_pos % (MAIN_DISPLAY_ROWS-1))+1][0] = (char)DOGM204_MENU_POINTER_SIGN;
    // At the end, print screen
    dogm204_print_screen(new_screen);
}

static void handle_pressed_keys_menu(Key_TypeDef* cur_pressed_keys){

    if (cur_pressed_keys[0].special_function == KEY_MENU_SPECIAL_FUNC){
        if (menumode.leave_directly_permitted == LEAVE_WITH_MENU_KEY_ALLOWED){
            // Leave menu and go back to the previous UI mode
            if (menumode.opened_from_UI_mode == UI_MODE_SCRIBBLE){
                // Enter scribble mode without resetting it
                UI.UI_mode = UI_MODE_SCRIBBLE;
                dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
                scribble_page_print_at_cursor_pos();
                return;
            }
            if (menumode.opened_from_UI_mode == UI_MODE_CHATVIEW){
                // Go to the latest text exchange in chatviewmode if possible
                if (maybe_enter_chatview() != ESP_OK){
                    dogm204_print_screen(chatview_mode_enter_fail_instead_scribble_enter_msg);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    UI.UI_mode = UI_MODE_SCRIBBLE;
                    dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
                    scribble_page_print_at_cursor_pos();
                }
                else{
                    UI.UI_mode = UI_MODE_CHATVIEW;
                    chatview_print_chatitem_at_cursor();
                }
                return;
            }
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_SHIFT_SPECIAL_FUNC){
        if (cur_pressed_keys[1].special_function == KEY_MENU_SPECIAL_FUNC){
            // Shutdown
            nvs_save_ui_infos();
            powerlatch_shutdown();
            vTaskDelay(pdMS_TO_TICKS(1000));
            return;    
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_ENTER_SPECIAL_FUNC){
        // First perform callback of the menu_entry_TypeDef where the cursor points at
        menu_entry_TypeDef* entry_ptr = menumode.current_menu + menumode.menu_cursor_pos;
        if (entry_ptr->enter_callback != NO_ENTER_CB){
            entry_ptr->enter_callback();
        }
        // Leave this function without printing screen if the UI Mode changed during enter_callback
        if (UI.UI_mode != UI_MODE_MENU){
            return;
        }
        // If a child menu exists, jump to that one
        if ((menumode.current_menu + menumode.menu_cursor_pos)->menu_child != NO_CHILD){
        menumode.current_menu = (menumode.current_menu + menumode.menu_cursor_pos)->menu_child;
        menumode.menu_cursor_pos = 0;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_BACK_SPECIAL_FUNC){
        // If parent menu exists, go back to that one
        if ((menumode.current_menu + menumode.menu_cursor_pos)->menu_parent != NO_PARENT){
            menumode.current_menu = (menumode.current_menu + menumode.menu_cursor_pos)->menu_parent;
            menumode.menu_cursor_pos = 0;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_UP_SPECIAL_FUNC){
        // Navigate through menu directory
        if (menumode.menu_cursor_pos > 0){
            menumode.menu_cursor_pos--;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_DOWN_SPECIAL_FUNC){
        // Navigate through menu directory
        if (((menumode.current_menu+1+menumode.menu_cursor_pos)->menu_title[0] != '\0') && 
            ((menumode.current_menu+1+menumode.menu_cursor_pos)->entry_text[0] != '\0')){
            // Only when next entry is not MENU_END (see UI.h and menu structure at the top of this file)
            menumode.menu_cursor_pos++;
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_LEFT_SPECIAL_FUNC){
        // Perform left_callback if it exists
        menu_entry_TypeDef* entry_ptr = menumode.current_menu + menumode.menu_cursor_pos;
        if (entry_ptr->left_callback != NO_LEFT_CB){
            entry_ptr->left_callback();
        }
    }
    else if (cur_pressed_keys[0].special_function == KEY_RIGHT_SPECIAL_FUNC){
        // Perform right_callback if it exists
        menu_entry_TypeDef* entry_ptr = menumode.current_menu + menumode.menu_cursor_pos;
        if (entry_ptr->right_callback != NO_RIGHT_CB){
            entry_ptr->right_callback();
        }
    }
    // Lastly, print menu page
    menu_mode_print_at_cursor_pos();
}

// #endregion

// #region Menumode Pre Prompt menu structure

/*
#########################################################################################
##                                                                                     ##
##   Menumode Pre Prompt menu structure                                                ##
##                                                                                     ##
#########################################################################################
*/

// check UI.h 'menu_entry_TypeDef' to see how to read those structures

// Menu Title              Entries                  Where to go with enter    Go back to     Enter CB        Left CB         Right CB
menu_entry_TypeDef pre_prompt_confirm_menu[] = {
{"Finished prompt?   ",    "Yes, send          ",   pre_prompt_settings_menu,    NO_PARENT,     menu_cb_9,      NO_LEFT_CB,     NO_RIGHT_CB},
{"Finished prompt?   ",    "No, go back        ",   pre_prompt_cancel_menu,      NO_PARENT,     NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"Finished prompt?   ",    "No, goto Chatview  ",   NO_CHILD,                    NO_PARENT,     menu_cb_6,      NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};
menu_entry_TypeDef pre_prompt_cancel_menu[] = {
{"Do you want to:    ",    "Keep editing prompt",  NO_CHILD,                    NO_PARENT,      menu_cb_7,      NO_LEFT_CB,     NO_RIGHT_CB},
{"Do you want to:    ",    "Delete prompt      ",  NO_CHILD,                    NO_PARENT,      menu_cb_8,      NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};
// Special case: The prev. chat option can get removed (and added again) during runtime if this option is not allowed!
menu_entry_TypeDef pre_prompt_settings_menu[] = {
{"Do you want to:    ",    "Start a new chat   ",   pre_prompt_camera_menu,      NO_PARENT,     menu_cb_1,      NO_LEFT_CB,     NO_RIGHT_CB},
{"Do you want to:    ",    "Send in prev. chat ",   pre_prompt_camera_menu,      NO_PARENT,     menu_cb_2,      NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};
menu_entry_TypeDef pre_prompt_camera_menu[] = {
{"Do you want to:    ",    "Append all images  ",   NO_CHILD,                    NO_PARENT,     menu_cb_3,      NO_LEFT_CB,     NO_RIGHT_CB},
{"Do you want to:    ",    "Append no images   ",   NO_CHILD,                    NO_PARENT,     menu_cb_4,      NO_LEFT_CB,     NO_RIGHT_CB},
{"Do you want to:    ",    "Delete all images  ",   NO_CHILD,                    NO_PARENT,     menu_cb_5,      NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

// #endregion

// #region Menumode Pre Prompt menu callbacks

/*
#########################################################################################
##                                                                                     ##
##   Menumode Pre Prompt menu callbacks                                                ##
##                                                                                     ##
#########################################################################################
*/

void menu_cb_1(void){

    // User wants a new chat started
    scribblemode.start_new_chat = 1;
    // 'Repair' menu in case this option was removed in menu_cb_9
    pre_prompt_settings_menu[1] = (menu_entry_TypeDef)
                                    { "Do you want to:    ",
                                    "Send in prev. chat ",
                                    pre_prompt_camera_menu,
                                    NO_PARENT,menu_cb_2,
                                    NO_LEFT_CB,
                                    NO_RIGHT_CB};
}
void menu_cb_2(void){

    // User wants to continue in previous chat
    scribblemode.start_new_chat = 0;
}
void menu_cb_3(void){

    // Send prompt + all images in littleFS, goto chatview mode
    // Get scribble page content and reset scribble page
    char scribble_page_copy[SCRIBBLE_PAGE_LENGTH+1];
    strcpy(scribble_page_copy,scribblemode.scribble_page);
    scribble_page_copy[scribblemode.scribble_page_length] = '\0';
    scribble_mode_reset(&scribblemode);
    // Append pictures
    char pictures[MAX_SAVED_PICTURES][64] = {0};
    size_t cam_dir_size;
    uint8_t pic_cnt;
    get_saved_pictures_paths(&cam_dir_size,&pic_cnt,pictures);
    // Send prompt
    if (UI_send_prompt(scribble_page_copy,pictures) == ESP_OK){
        if (maybe_enter_chatview() != ESP_OK){
            dogm204_print_screen(chatview_mode_enter_fail_instead_scribble_enter_msg);
            vTaskDelay(pdMS_TO_TICKS(1000));
            UI.UI_mode = UI_MODE_SCRIBBLE;
            dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
            scribble_page_print_at_cursor_pos();
        }
        else{
            UI.UI_mode = UI_MODE_CHATVIEW;
            chatview_print_chatitem_at_cursor();
        }
    }
    else{
        // This code can only be reached if ai model is not supported
        dogm204_print_screen(ai_model_not_supported_msg);
        vTaskDelay(pdMS_TO_TICKS(1000));
        UI.UI_mode = UI_MODE_SCRIBBLE;
        dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
        scribble_page_print_at_cursor_pos();
    }
    // Delete camera directory afterwards
    if (UI.delete_imgs_after_sending){
        delete_camera_directory();
        get_saved_pics(&UI.cam_img_cnt);
    }
}
void menu_cb_4(void){

    // Send only prompt, goto chatview mode
    // Get scribble page content and reset scribble page
    char scribble_page_copy[SCRIBBLE_PAGE_LENGTH+1];
    strcpy(scribble_page_copy,scribblemode.scribble_page);
    scribble_page_copy[scribblemode.scribble_page_length] = '\0';
    scribble_mode_reset(&scribblemode);
    // No pictures appended
    char pictures[MAX_SAVED_PICTURES][64] = {0};
    // Send prompt, go to chatview mode
    if (UI_send_prompt(scribble_page_copy,pictures) == ESP_OK){
        if (maybe_enter_chatview() != ESP_OK){
            dogm204_print_screen(chatview_mode_enter_fail_instead_scribble_enter_msg);
            vTaskDelay(pdMS_TO_TICKS(1000));
            UI.UI_mode = UI_MODE_SCRIBBLE;
            dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
            scribble_page_print_at_cursor_pos();
        }
        else{
            UI.UI_mode = UI_MODE_CHATVIEW;
            chatview_print_chatitem_at_cursor();
        }
    }
    else{
        // This code can only be reached if ai model is not supported
        dogm204_print_screen(ai_model_not_supported_msg);
        vTaskDelay(pdMS_TO_TICKS(1000));
        UI.UI_mode = UI_MODE_SCRIBBLE;
        dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
        scribble_page_print_at_cursor_pos();
    }
}
void menu_cb_5(void){

    // Send only prompt, delete image directory, goto chatview mode
    // Get scribble page content and reset scribble page
    char scribble_page_copy[SCRIBBLE_PAGE_LENGTH+1];
    strcpy(scribble_page_copy,scribblemode.scribble_page);
    scribble_page_copy[scribblemode.scribble_page_length] = '\0';
    scribble_mode_reset(&scribblemode);
    // No pictures appended
    char pictures[MAX_SAVED_PICTURES][64] = {0};
    // Delete image directory
    delete_camera_directory();
    get_saved_pics(&UI.cam_img_cnt);
    // Send prompt, go to chatview mode
    if (UI_send_prompt(scribble_page_copy,pictures) == ESP_OK){
        if (maybe_enter_chatview() != ESP_OK){
            dogm204_print_screen(chatview_mode_enter_fail_instead_scribble_enter_msg);
            vTaskDelay(pdMS_TO_TICKS(1000));
            UI.UI_mode = UI_MODE_SCRIBBLE;
            dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
            scribble_page_print_at_cursor_pos();
        }
        else{
            UI.UI_mode = UI_MODE_CHATVIEW;
            chatview_print_chatitem_at_cursor();
        }
    }
    else{
        // This code can only be reached if ai model is not supported
        dogm204_print_screen(ai_model_not_supported_msg);
        vTaskDelay(pdMS_TO_TICKS(1000));
        UI.UI_mode = UI_MODE_SCRIBBLE;
        dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
        scribble_page_print_at_cursor_pos();
    }
}
void menu_cb_6(void){

    // Go to ChatView Mode, (not sending prompt)
    if (maybe_enter_chatview() != ESP_OK){
        dogm204_print_screen(chatview_mode_enter_fail_instead_scribble_enter_msg);
        vTaskDelay(pdMS_TO_TICKS(1000));
        UI.UI_mode = UI_MODE_SCRIBBLE;
        dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
        scribble_page_print_at_cursor_pos();
    }
    else{
        UI.UI_mode = UI_MODE_CHATVIEW;
        chatview_print_chatitem_at_cursor();
    }
}
void menu_cb_7(void){

    // Return to scribble page
    UI.UI_mode = UI_MODE_SCRIBBLE;
    dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
    scribble_page_print_at_cursor_pos();
}
void menu_cb_8(void){

    // Return to scirbble page and delete prompt
    UI.UI_mode = UI_MODE_SCRIBBLE;
    dogm204_display_control(DOGM204_CURSOR_ON_BIT|DOGM204_CURSOR_BLINK_BIT|DOGM204_DISPLAY_ON_BIT);
    scribble_mode_reset(&scribblemode);
    scribble_page_print_at_cursor_pos();
}
void menu_cb_9(void){

    // If max. conversation lenght is reached, disable continuing prev. chat (also if no previous chat exists)
    uint16_t text_exchanges = openai_chat_dir_text_exchanges();
    // ESP_LOGI("Test","Textexchanges: %u",text_exchanges);
    if ((text_exchanges == 0) || (text_exchanges >= (MAX_CONVERSATION_LENGTH-1))){
        pre_prompt_settings_menu[1] = (menu_entry_TypeDef)MENU_END;
    }
}

// #endregion

// #region Menumode Main menu tree structure

/*
#########################################################################################
##                                                                                     ##
##  Menumode Main Menu tree structure                                                  ##
##                                                                                     ##
#########################################################################################
*/

// check UI.h 'menu_entry_TypeDef' to see how to read those structures

// Some features are still missing, pressing on those menu entries will do nothing

// Menu Title           Entries                     Where to go with enter      Go back to      Enter callback  Left callback   Right callback
menu_entry_TypeDef main_menu[] = {
{"Main Menu          ",    "Wifi Settings      ",   wifi_main_menu,             NO_PARENT,      menu_cb_32,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Main Menu          ",    "AI Settings        ",   ai_main_menu,               NO_PARENT,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"Main Menu          ",    "File system        ",   filesys_main_menu,          NO_PARENT,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"Main Menu          ",    "UI Settings        ",   ui_main_menu,               NO_PARENT,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"Main Menu          ",    "Device Settings    ",   device_main_menu,           NO_PARENT,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"Main Menu          ",    "Debugging          ",   debugging_main_menu,        NO_PARENT,      menu_cb_10,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

// Special case: Entry text of last entry "Wifi:..." shows On/Off state, dynamically changed during runtime via callback functions
menu_entry_TypeDef wifi_main_menu[] = {
{"Wifi Settings      ",    "Edit saved wifis   ",    wifis_edit_menu,            main_menu,      menu_cb_11,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Wifi Settings      ",    "Man. choose wifi   ",    wifis_man_choose_menu,      main_menu,      menu_cb_12,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Wifi Settings      ",    "Wifi:              ",    NO_CHILD,                   main_menu,      menu_cb_13,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

menu_entry_TypeDef ai_main_menu[] = {
{"AI Settings        ",    "Choose AI Model    ",   chose_ai_menu,              main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"AI Settings        ",    "OpenAI version     ",   choose_openai_ver_menu,     main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"AI Settings        ",    "Gemini Version     ",   NO_CHILD,                   main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"AI Settings        ",    "Claude Version     ",   NO_CHILD,                   main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"AI Settings        ",    "Enter API Code     ",   api_select_ai_menu,         main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"AI Settings        ",    "Delete chat history",   NO_CHILD,                   main_menu,      menu_cb_14,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

menu_entry_TypeDef filesys_main_menu[] = {
{"File system        ",    "Filesystem settings",   NO_CHILD,                   main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"File system        ",    "Open File system   ",   NO_CHILD,                   main_menu,      menu_cb_15,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

menu_entry_TypeDef ui_main_menu[] = {
{"UI Settings        ",    "Auto-off-timer     ",   autooff_settings_menu,      main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"UI Settings        ",    "Side display       ",   NO_CHILD,                   main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"UI Settings        ",    "UI unlock code     ",   unlck_change_conf_menu,     main_menu,      menu_cb_16,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

menu_entry_TypeDef device_main_menu[] = {
{"Device Settings    ",    "Set display contr. ",   display_contr_set_menu,     main_menu,      menu_cb_17,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Device Settings    ",    "Get device info    ",   NO_CHILD,                   main_menu,      menu_cb_18,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Device Settings    ",    "Camera settings    ",   camera_settings_menu,       main_menu,      NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"Device Settings    ",    "Factory reset      ",   factory_rst_conf_menu,      main_menu,      menu_cb_19,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Device Settings    ",    "Enter debug mode   ",   enter_debug_conf_menu,      main_menu,      menu_cb_20,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

menu_entry_TypeDef debugging_main_menu[] = {
{"Debug Settings     ",    "Print battery info ",   NO_CHILD,                   main_menu,      menu_cb_21,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

// Special cases: Entry texts assigned via callback when entering those menus at runtime:
menu_entry_TypeDef wifis_edit_menu[WIFI_MAX_STORED_LOGINDATA+1];
menu_entry_TypeDef wifis_man_choose_menu[WIFI_MAX_STORED_LOGINDATA+1];

menu_entry_TypeDef chose_ai_menu[] = {
{"Select AI model    ",    "OpenAI             ",   NO_CHILD,                   ai_main_menu,   menu_cb_22,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Select AI model    ",    "Gemini             ",   NO_CHILD,                   ai_main_menu,   menu_cb_23,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Select AI model    ",    "Claude             ",   NO_CHILD,                   ai_main_menu,   menu_cb_24,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

// Make sure these options match openai_supported_models[] in LLMs.c
menu_entry_TypeDef choose_openai_ver_menu[] = {
{"Select version     ",    "GPT 5.5            ",   NO_CHILD,                   ai_main_menu,   menu_cb_25,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Select version     ",    "GPT 5.6            ",   NO_CHILD,                   ai_main_menu,   menu_cb_26,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Select version     ",    "GPT 5.6 Luna       ",   NO_CHILD,                   ai_main_menu,   menu_cb_27,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Select version     ",    "GPT 5.6 Terra      ",   NO_CHILD,                   ai_main_menu,   menu_cb_28,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

menu_entry_TypeDef api_select_ai_menu[] = {
{"API Key for...     ",    "OpenAI             ",   NO_CHILD,                   ai_main_menu,   menu_cb_29,     NO_LEFT_CB,     NO_RIGHT_CB},
{"API Key for...     ",    "Gemini             ",   NO_CHILD,                   ai_main_menu,   menu_cb_30,     NO_LEFT_CB,     NO_RIGHT_CB},
{"API Key for...     ",    "Claude             ",   NO_CHILD,                   ai_main_menu,   menu_cb_31,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

// Special case: Entry texts changed dynamically at runtime via callbacks
menu_entry_TypeDef autooff_settings_menu[] = {
{"Auto-off-Timer     ",    "Activated:         ",   NO_CHILD,                   ui_main_menu,   menu_cb_33,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Auto-off-Timer     ",    "Set to     min     ",   NO_CHILD,                   ui_main_menu,   NO_ENTER_CB,    menu_cb_34,     menu_cb_35},
MENU_END
};

menu_entry_TypeDef unlck_change_conf_menu[] = {
{"Proceed?           ",    "Yes                ",   NO_CHILD,                   ui_main_menu,   menu_cb_36,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Proceed?           ",    "No, go back        ",   ui_main_menu,               ui_main_menu,   NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

// Special case: Entry texts changed dynamically at runtime via callbacks
menu_entry_TypeDef display_contr_set_menu[] = {
{"Adjust contrast    ",    "Sidedisplay:       ",   NO_CHILD,              device_main_menu,    NO_ENTER_CB,    menu_cb_37,     menu_cb_38},
{"Adjust contrast    ",    "Maindisplay:       ",   NO_CHILD,              device_main_menu,    NO_ENTER_CB,    menu_cb_39,     menu_cb_40},
MENU_END
};

// Special case, JPG quality entry text changed dynamically via callbacks
menu_entry_TypeDef camera_settings_menu[] = {
{"Camera settings    ",    "Set framesize      ",   set_framesize_menu,    device_main_menu,    NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
{"Camera settings    ",    "JPG Quality:       ",   NO_CHILD,              device_main_menu,    NO_ENTER_CB,    menu_cb_41,     menu_cb_42},
{"Camera settings    ",    "Delete all images  ",   NO_CHILD,              device_main_menu,    menu_cb_43,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

// Make sure these match up with the ones supported in camera.h!
menu_entry_TypeDef set_framesize_menu[] = {
{"Set framesize      ",    "VGA_640_480_PX      ",  NO_CHILD,          camera_settings_menu,    menu_cb_44,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Set framesize      ",    "XGA_1024_768_PX     ",  NO_CHILD,          camera_settings_menu,    menu_cb_45,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Set framesize      ",    "UXGA_1600_1200_PX   ",  NO_CHILD,          camera_settings_menu,    menu_cb_46,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Set framesize      ",    "QSXGA_2560_1920_PX  ",  NO_CHILD,          camera_settings_menu,    menu_cb_47,     NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

menu_entry_TypeDef factory_rst_conf_menu[] = {
{"Proceed?           ",    "Yes                 ",  NO_CHILD,               device_main_menu,   menu_cb_48,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Proceed?           ",    "No, go back         ",  device_main_menu,       device_main_menu,   NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

menu_entry_TypeDef enter_debug_conf_menu[] = {
{"Proceed?           ",    "Yes                 ",  NO_CHILD,               device_main_menu,   menu_cb_48,     NO_LEFT_CB,     NO_RIGHT_CB},
{"Proceed?           ",    "No, go back         ",  device_main_menu,       device_main_menu,   NO_ENTER_CB,    NO_LEFT_CB,     NO_RIGHT_CB},
MENU_END
};

// #endregion

// #region Menumode main menu tree callbacks

/*
#########################################################################################
##                                                                                     ##
##   Menumode main menu tree callbacks                                                 ##
##                                                                                     ##
#########################################################################################
*/

void menu_cb_10(void){

    // Change menu entry for wifi: Show if on/off
}
void menu_cb_11(void){

    // Construct the wifis_edit_menu dynamically
    for (uint8_t i=0; i<WIFI_MAX_STORED_LOGINDATA; i++){
        // Entries
        if (get_wifi_ssid(i,wifis_edit_menu[i].entry_text,MAIN_DISPLAY_COLUMNS)!=ESP_OK){
            strcpy(wifis_edit_menu[i].entry_text,"Save new wifi here ");
        }
        // ESP_LOGI("UI Test","Login %u: |%s|",i,wifis_edit_menu[i].entry_text);
        // Rest (Header etc.)
        strcpy(wifis_edit_menu[i].menu_title,"Edit a Wifi        ");
        wifis_edit_menu[i].menu_child = NO_CHILD;
        wifis_edit_menu[i].menu_parent = wifi_main_menu;
        wifis_edit_menu[i].enter_callback = menu_cb_49;
        wifis_edit_menu[i].left_callback = NO_LEFT_CB;
        wifis_edit_menu[i].right_callback = NO_RIGHT_CB;
    }
    // Menu end
    wifis_edit_menu[WIFI_MAX_STORED_LOGINDATA] = (menu_entry_TypeDef)MENU_END;
}
void menu_cb_12(void){

    // Construct the wifis_man_choose_menu dynamically
    for (uint8_t i=0; i<WIFI_MAX_STORED_LOGINDATA; i++){
        // Entries
        if (get_wifi_ssid(i,wifis_man_choose_menu[i].entry_text,MAIN_DISPLAY_COLUMNS)!=ESP_OK){
            strcpy(wifis_man_choose_menu[i].entry_text,"Empty              ");
            wifis_man_choose_menu[i].enter_callback = menu_cb_50;
        }
        else{
            wifis_man_choose_menu[i].enter_callback = NO_ENTER_CB;
        }
        // ESP_LOGI("UI Test","Login %u: |%s|",i,wifis_man_choose_menu[i].entry_text);
        // Rest (Header etc.)
        strcpy(wifis_man_choose_menu[i].menu_title,"Choose a Wifi      ");
        wifis_man_choose_menu[i].menu_child = NO_CHILD;
        wifis_man_choose_menu[i].menu_parent = wifi_main_menu;
        wifis_man_choose_menu[i].left_callback = NO_LEFT_CB;
        wifis_man_choose_menu[i].right_callback = NO_RIGHT_CB;
    }
    // Menu end
    wifis_man_choose_menu[WIFI_MAX_STORED_LOGINDATA] = (menu_entry_TypeDef)MENU_END;
}
void menu_cb_13(void){

}
void menu_cb_14(void){

}
void menu_cb_15(void){

}
void menu_cb_16(void){

}
void menu_cb_17(void){

}
void menu_cb_18(void){

}
void menu_cb_19(void){

}
void menu_cb_20(void){

}
void menu_cb_21(void){

}
void menu_cb_22(void){

}
void menu_cb_23(void){

}
void menu_cb_24(void){

}
void menu_cb_25(void){

}
void menu_cb_26(void){

}
void menu_cb_27(void){

}
void menu_cb_28(void){

}
void menu_cb_29(void){

    // Save OpenAI API key
    api_key_for_temp = AI_MODEL_OPENAI;
    enter_textinput_mode(&enter_api_key_page);
    UI.UI_mode = UI_MODE_TEXTINPUT;
    print_textinput_page_at_cursor();
}
void menu_cb_30(void){

    // Gemini API key
    api_key_for_temp = AI_MODEL_GEMINI;
    enter_textinput_mode(&enter_api_key_page);
    UI.UI_mode = UI_MODE_TEXTINPUT;
    print_textinput_page_at_cursor();
}
void menu_cb_31(void){

    // Claude API key
    api_key_for_temp = AI_MODEL_CLAUDE;
    enter_textinput_mode(&enter_api_key_page);
    UI.UI_mode = UI_MODE_TEXTINPUT;
    print_textinput_page_at_cursor();
}
void menu_cb_32(void){

}
void menu_cb_33(void){

}
void menu_cb_34(void){

}
void menu_cb_35(void){

}
void menu_cb_36(void){

}
void menu_cb_37(void){

}
void menu_cb_38(void){

}
void menu_cb_39(void){

}
void menu_cb_40(void){

}
void menu_cb_41(void){

}
void menu_cb_42(void){

}
void menu_cb_43(void){

}
void menu_cb_44(void){

}
void menu_cb_45(void){

}
void menu_cb_46(void){

}
void menu_cb_47(void){

}
void menu_cb_48(void){

}
void menu_cb_49(void){

    // Save where the user wants to save his new wifi
    wifi_index_temp = menumode.menu_cursor_pos;
    // Switch to textinput mode so user can type in an SSID->Password
    enter_textinput_mode(&enter_ssid_page);
    UI.UI_mode = UI_MODE_TEXTINPUT;
    print_textinput_page_at_cursor();
}
void menu_cb_50(void){

    // Set preferred wifi login
    esp_err_t ret = wifi_set_prefered_wifi(menumode.menu_cursor_pos);
    if (ret == ESP_OK){
        dogm204_print_screen(pref_wifi_saved_success_msg);
    }
    else{
        dogm204_print_screen(pref_wifi_saved_fail_msg);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// #endregion


// #region Fileviewmode static functions

/*
#########################################################################################
##                                                                                     ##
##   Fileviewmode static functions                                                     ##
##                                                                                     ##
#########################################################################################
*/

static void handle_pressed_keys_fileview(Key_TypeDef* cur_pressed_keys){

}

// #endregion

// #region LittleFS static functions 

/*
#########################################################################################
##                                                                                     ##
##   LittleFS static functions                                                         ##
##                                                                                     ##
#########################################################################################
*/

static void littleFS_init(void){

    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",       
        .partition_label = "littlefs",  // see partitions.csv
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE("LFS", "LittleFS mount failed: %s",esp_err_to_name(ret));
        return;
    }
    size_t total = 0, used = 0;
    esp_littlefs_info("littlefs", &total, &used);
    ESP_LOGI("LFS", "LittleFS mounted: %d KB total, %d KB used", total / 1024, used / 1024);
    // Create all neccessary directories if they are not there yet
    // To temporarily store JPGs:
    mkdir("/littlefs/cam",0755);
    // For chatdate of the AI conversations
    mkdir("/littlefs/chat",0755);
    // For saved texts (through file upload)
    mkdir("/littlefs/savedtexts",0755);
}

// #endregion

// #region Generic extern UI functions

/*
#########################################################################################
##                                                                                     ##
##   Generic extern UI functions                                                       ##
##                                                                                     ##
#########################################################################################
*/

void UI_init(void){

    nvs_get_ui_infos();
    get_saved_pics(&UI.cam_img_cnt);
    // Start in calculator mode always (empty equation and solution line)
    UI.UI_mode = UI_MODE_CALCULATOR;
    calculatormode_init(&calculatormode);
    // Current UI version
    strcpy(UI.UI_version_string,UI_VER_STRING);
    littleFS_init();
    llms_init();
    // Start UI sidetask
    xTaskCreate(UI_sidetask,"UI_side",4096,NULL,2,&ui_sidetask_handle);
    // Only for debugging:
    UI.file_dir_size = 0;
}

void UI_handle_pressed_keys(Key_TypeDef* cur_pressed_keys){

    // Called in app_main when the pressed keys have changed, UI reaction depends on current UI mode:
    user_activity_sensed = 1;
    switch (UI.UI_mode){
        case UI_MODE_CALCULATOR:
            handle_pressed_keys_calc(cur_pressed_keys);
            break;
        case UI_MODE_SCRIBBLE:
            handle_pressed_keys_scribble(cur_pressed_keys);
            break;
        case UI_MODE_TEXTINPUT:
            handle_pressed_keys_textinput(cur_pressed_keys);
            break;
        case UI_MODE_CHATVIEW:
            handle_pressed_keys_chatview(cur_pressed_keys);
            break;
        case UI_MODE_MENU:
            handle_pressed_keys_menu(cur_pressed_keys);
            break;
        case UI_MODE_FILEVIEW:
            handle_pressed_keys_fileview(cur_pressed_keys);
            break;
    }
}

void get_UI_state(UI_TypeDef* ui){

    // At the moment only gets the values required for the sidedisplay rendering
    ui->UI_mode = UI.UI_mode;
    ui->cam_img_cnt = UI.cam_img_cnt;
    ui->file_dir_size = UI.file_dir_size;
    ui->current_ai_model = UI.current_ai_model;
    strcpy(ui->openai_model_version,UI.openai_model_version);
    strcpy(ui->gemini_model_version,UI.gemini_model_version);
    strcpy(ui->claude_model_version,UI.claude_model_version);
}

// #endregion

