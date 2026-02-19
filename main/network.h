// ChatGPT Hardware Hack compatible with Casio FX Series

// network.h -> Functions and variables to connect to wifi and call openAI API

// © 2026 Jonas Heselschwerdt
// Licensed under CC BY-NC 4.0




#ifndef NETWORK_H
#define NETWORK_H

#include "esp_err.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "UI.h"
#include "keyset.h"

void wifi_initialize(void);
char* handle_openai_chat(const char* user_input);
esp_err_t wifi_load_credentials(int index, char *ssid, size_t ssid_len,
                                char *pass, size_t pass_len);
esp_err_t api_save_key(const char *key);
esp_err_t wifi_save_credentials(int index, const char *ssid, const char *pass);
esp_err_t wifi_clear_credentials(int index);
void wifi_credentials_nvs_init(void);
void preset_test_credentials(void);
void wifi_manager_start(void);
bool check_for_http_server(const char* server_location);
void read_from_http(const char *server_location);
void littlefs_init(void);
void display_from_local_html(uint16_t *html_menu, uint8_t html_text_passage, char *current_html_text, Cursor *cursor);
esp_err_t settings_set_str(const char *key, const char *value);
esp_err_t settings_get_str(const char *key, char *out, size_t out_len);
void clear_html(void);
void hid_send_char(char c);
void hid_mode_stop(void);
void hid_mode_start(void);
void hid_send_special_key(Special_Hid_Key key);
void toggle_usb_mode(Cursor cursor);


#endif