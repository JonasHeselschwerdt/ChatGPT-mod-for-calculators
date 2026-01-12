// ChatGPT Hardware Hack compatible with Casio FX Series

// network.h -> Functions and variables to connect to wifi and call openAI API

// © 2026 Jonas Heselschwerdt
// Free for personal, research and educational use
// Commercial use requires written permission




#ifndef NETWORK_H
#define NETWORK_H

#include "esp_err.h"
#include <string.h>
#include <stdio.h>

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

#endif