// ChatGPT Hardware Hack compatible with Casio FX Series

// network.c -> Functions and variables to connect to wifi and call openAI API

// © 2026 Jonas Heselschwerdt
// Licensed under CC BY-NC 4.0




#include "network.h"
#include <string.h>
#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"

#include "config.h"
#include "secret.h"

#include "OpenAI.h"

#include <stdlib.h>

static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;





// NVS (non volatile storage) functions to save wifi credentials and your API key

// Setup NVS

void wifi_credentials_nvs_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        // reformat partition if neccessary

        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
}

// Save and load wifi credentials

esp_err_t wifi_save_credentials(int index, const char *ssid, const char *pass) {

    if (index < 0 || index >= MAX_WIFI_ENTRIES) return ESP_ERR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(WIFI_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    char key_ssid[16], key_pass[16];
    sprintf(key_ssid, "ssid%d", index);
    sprintf(key_pass, "pass%d", index);

    err = nvs_set_str(handle, key_ssid, ssid);
    if (err != ESP_OK) { nvs_close(handle); return err; }

    err = nvs_set_str(handle, key_pass, pass);
    if (err != ESP_OK) { nvs_close(handle); return err; }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

esp_err_t wifi_load_credentials(int index, char *ssid, size_t ssid_len,
                                char *pass, size_t pass_len) {

    if (index < 0 || index >= MAX_WIFI_ENTRIES) return ESP_ERR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(WIFI_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    char key_ssid[16], key_pass[16];
    sprintf(key_ssid, "ssid%d", index);
    sprintf(key_pass, "pass%d", index);

    // load ssid

    err = nvs_get_str(handle, key_ssid, ssid, &ssid_len);
    if (err != ESP_OK) { nvs_close(handle); return err; }

    // load password if buffer != NULL

    if (pass != NULL) {
        err = nvs_get_str(handle, key_pass, pass, &pass_len);
        if (err != ESP_OK) { nvs_close(handle); return err; }
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t wifi_clear_credentials(int index) {

    if (index < 0 || index >= MAX_WIFI_ENTRIES) return ESP_ERR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(WIFI_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    char key_ssid[16], key_pass[16];
    sprintf(key_ssid, "ssid%d", index);
    sprintf(key_pass, "pass%d", index);

    nvs_erase_key(handle, key_ssid);
    nvs_erase_key(handle, key_pass);

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

// Save and load API Key

esp_err_t api_save_key(const char *key) {

    nvs_handle_t handle;
    esp_err_t err = nvs_open(API_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    // delete old API Key

    nvs_erase_key(handle, "api_key");

    err = nvs_set_str(handle, "api_key", key);
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}


esp_err_t api_load_key(char *key_buf, size_t buf_len) {

    if (!key_buf || buf_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(API_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    size_t required_len = buf_len; 
    err = nvs_get_str(handle, "api_key", key_buf, &required_len);

    nvs_close(handle);

    // Terminate string

    if (err == ESP_OK) {
        key_buf[buf_len - 1] = '\0';
    }

    return err;
}

// Function to load Wifi SSIDs / Passwords / API Keys from secret.h
// if you dont want to type them in via the calculators user interface

void preset_test_credentials(void) {
    
    // Add wifi 0 if necessary

    char ssid[32], pass[64];
    esp_err_t err = wifi_load_credentials(0, ssid, sizeof(ssid), pass, sizeof(pass));
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI("PRESET", "Wifi 0 not found, saving test credentials from secret.h");
        wifi_save_credentials(0, WIFI_SSID, WIFI_PASS);
    }

    // Add API Key if necessary

    char api_buf[256] = {0};
    err = api_load_key(api_buf, sizeof(api_buf));
    if (err == ESP_ERR_NVS_NOT_FOUND || strlen(api_buf) < 10) {
        
        #ifdef OpenAIApiKey
        
        if (strlen(OpenAIApiKey) >= 10) {
            ESP_LOGI("PRESET", "API-Key not found, save API Key from secret.h");

            char key_ram[256];
            strncpy(key_ram, OpenAIApiKey, sizeof(key_ram) - 1);
            key_ram[sizeof(key_ram) - 1] = '\0';

            api_save_key(key_ram);
        }

        else {
            ESP_LOGW("PRESET", "OpenAIApiKey is empty or incomplete");
        }
        #else

        ESP_LOGW("PRESET", "No API Key in secret.h ");

        #endif

    } else {
        ESP_LOGI("PRESET", "API-Key already exists! (len=%u)", (unsigned)strlen(api_buf));
    }
}






// Wifi Manager functions

static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static volatile bool wifi_connected = false;
static volatile bool wifi_scan_in_progress = false;
static volatile bool wifi_scan_done = false;

static const char *TAG_WIFI = "WIFI_MANAGER";

#define WIFI_SCAN_RETRY_DELAY_MS   2000   
#define WIFI_SCAN_PERIOD_MS       15000  

// Event Handler: set wifi state

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG_WIFI, "STA started");
                wifi_connected = false;
                break;

            case WIFI_EVENT_SCAN_DONE:
                wifi_scan_in_progress = false;
                wifi_scan_done = true;
                ESP_LOGI(TAG_WIFI, "Scan done");
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                wifi_connected = false;
                ESP_LOGW(TAG_WIFI, "Disconnected from AP");
                break;

            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_connected = true;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG_WIFI, "Got IP");
    }
}

// Search for wifi credentials that match surrounding wifis

static void wifi_try_connect_from_nvs(void)
{
    uint16_t ap_count = 0;
    if (esp_wifi_scan_get_ap_num(&ap_count) != ESP_OK || ap_count == 0) {
        ESP_LOGW(TAG_WIFI, "No APs found");
        return;
    }

    wifi_ap_record_t *ap_records = (wifi_ap_record_t *)malloc(ap_count * sizeof(wifi_ap_record_t));
    if (!ap_records) {
        ESP_LOGE(TAG_WIFI, "malloc for AP records failed");
        return;
    }

    if (esp_wifi_scan_get_ap_records(&ap_count, ap_records) != ESP_OK) {
        ESP_LOGE(TAG_WIFI, "scan_get_ap_records failed");
        free(ap_records);
        return;
    }

    for (int i = 0; i < MAX_WIFI_ENTRIES; i++) {
        char ssid[32], pass[64];
        if (wifi_load_credentials(i, ssid, sizeof(ssid), pass, sizeof(pass)) != ESP_OK) {
            continue;
        }

        for (int j = 0; j < ap_count; j++) {
            if (strcmp((const char *)ap_records[j].ssid, ssid) == 0) {
                ESP_LOGI(TAG_WIFI, "Found known network: %s (RSSI=%d)", ssid, ap_records[j].rssi);

                wifi_config_t cfg = {0};
                strncpy((char *)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid));
                strncpy((char *)cfg.sta.password, pass, sizeof(cfg.sta.password));

                cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
                ESP_ERROR_CHECK(esp_wifi_connect());

                free(ap_records);
                return;
            }
        }
    }

    ESP_LOGW(TAG_WIFI, "No known network nearby");
    free(ap_records);
}

// Task that controls disconnect / reconnect cycle in background

static void wifi_manager_task(void *param)
{
    for (;;) {
        if (!wifi_connected) {
            
            if (!wifi_scan_in_progress && !wifi_scan_done) {
                wifi_scan_in_progress = true;
                wifi_scan_done = false;

                wifi_scan_config_t scan_cfg = {
                    .ssid = NULL,
                    .bssid = NULL,
                    .channel = 0,
                    .show_hidden = false,
                    .scan_type = WIFI_SCAN_TYPE_ACTIVE,
                    .scan_time.active = {
                        .min = 100,
                        .max = 300
                    }
                };
                ESP_LOGI(TAG_WIFI, "Starting scan...");
                esp_err_t err = esp_wifi_scan_start(&scan_cfg, false); 
                if (err != ESP_OK) {
                    wifi_scan_in_progress = false;
                    ESP_LOGE(TAG_WIFI, "scan_start failed: %s", esp_err_to_name(err));
                    vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_RETRY_DELAY_MS));
                }
            }

            // Scan done, try connecting

            if (wifi_scan_done) {
                wifi_try_connect_from_nvs();
                wifi_scan_done = false;

                // Still not connected -> short delay

                if (!wifi_connected) {
                    vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_RETRY_DELAY_MS));
                }
            }

            // Neither scan in progress or scan done: Wait

            if (!wifi_scan_in_progress && !wifi_scan_done) {
                vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_PERIOD_MS));
            }
        } else {

            // Connected: Wait

            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}

void wifi_manager_start(void)
{
    ESP_LOGI(TAG_WIFI, "Wi-Fi Manager starting...");

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    // Need bigger stack (8192)
    xTaskCreate(wifi_manager_task, "wifi_manager_task", 8192, NULL, 5, NULL);
}

// Helper function

bool wifi_is_connected(void)
{
    return wifi_connected;
}







// OpenAI communication functions

static char final_response[answer_page_length + 1];
static EventGroupHandle_t openai_event_group;
#define OPENAI_DONE_BIT BIT0

// Debug-Wrapper

static OpenAI_StringResponse_t* debug_chat_message(OpenAI_ChatCompletion_t *chat, const char *p, bool save)
{
    OpenAI_StringResponse_t *resp = chat->message(chat, p, save);
    if (!resp) {
        
        size_t plen = strlen(p);
        const char *pview = p;
        char promptbuf[129] = {0};
        if (plen > 128) {
            memcpy(promptbuf, p, 125);
            strcpy(promptbuf + 125, "...");
            pview = promptbuf;
        }
        ESP_LOGE("OpenAI", "chat->message returned NULL for prompt: \"%s\"", pview);
        
    }
    return resp;
}

static void openai_task(void *param) {

    const char *user_input = (const char *)param;

    memset(final_response, ' ', answer_page_length);
    final_response[answer_page_length] = 0;

    // Load API Key from NVS

    char api_key_buf[256] = {0};
    esp_err_t err = api_load_key(api_key_buf, sizeof(api_key_buf));

    ESP_LOGI("DEBUG", "api_load_key() = %s, Key = '%s'", esp_err_to_name(err), api_key_buf);

    if (err != ESP_OK || strlen(api_key_buf) < 10) {
        ESP_LOGE("OpenAI", "API key not found in NVS!");
        memcpy(final_response, "ERROR: NO API KEY", 17);
        xEventGroupSetBits(openai_event_group, OPENAI_DONE_BIT);
        vTaskDelete(NULL);
        return;
    }

    // Initialize OpenAI with key from NVS

    OpenAI_t *openai = OpenAICreate(api_key_buf);
    if (!openai) {
        memcpy(final_response, "ERROR: INIT FAILED", 18);
        xEventGroupSetBits(openai_event_group, OPENAI_DONE_BIT);
        vTaskDelete(NULL);
        return;
    }


    OpenAI_ChatCompletion_t *chat = openai->chatCreate(openai);
    if (!chat) {
        memcpy(final_response, "ERROR: CHAT CREATE", 19);
        OpenAIDelete(openai);
        xEventGroupSetBits(openai_event_group, OPENAI_DONE_BIT);
        vTaskDelete(NULL);
        return;
    }

    chat->setModel(chat, "gpt-4o");
    chat->setMaxTokens(chat, 200);
    chat->setTemperature(chat, 0.7f);
    chat->setStop(chat, "\r");
    chat->setPresencePenalty(chat, 0);
    chat->setFrequencyPenalty(chat, 0);
    chat->setUser(chat, "OpenAI-ESP32");

    // Use debug wrapper:

    OpenAI_StringResponse_t *resp = debug_chat_message(chat, user_input, false);

    if (resp) {
        
        uint32_t len = resp->getLen(resp);
        ESP_LOGI("OpenAI", "Response hat %lu Chunks", (unsigned long)len);

        for (uint32_t i = 0; i < len; i++) {
            char *chunk = resp->getData(resp, i);
            if (chunk) {
                ESP_LOGI("OpenAI", "Chunk %lu: %s", (unsigned long)i, chunk);
            } else {
                ESP_LOGE("OpenAI", "Chunk %lu konnte nicht gelesen werden",
                         (unsigned long)i);
            }
        }

        char *response = resp->getData(resp, 0);
        if (response) {
            size_t len = strlen(response);
            if (len > answer_page_length) len = answer_page_length;
            memcpy(final_response, response, len);
        } else {
            const char *err = resp->getError(resp);
            if (err) {
                size_t len = strlen(err);
                if (len > answer_page_length) len = answer_page_length;
                memcpy(final_response, err, len);
            } else {
                memcpy(final_response, "ERROR: EMPTY RESP", 17);
            }
        }
        resp->deleteResponse(resp);
    } else {
        memcpy(final_response, "ERROR: NO RESPONSE", 19);
    }

    openai->chatDelete(chat);
    OpenAIDelete(openai);

    xEventGroupSetBits(openai_event_group, OPENAI_DONE_BIT);
    vTaskDelete(NULL);
}

char* handle_openai_chat(const char* user_input) {
    if (!openai_event_group) {
        openai_event_group = xEventGroupCreate();
    }

    // Reset event bit
    xEventGroupClearBits(openai_event_group, OPENAI_DONE_BIT);

    // Start task
    xTaskCreate(openai_task, "openai_task", 16384, (void*)user_input, 5, NULL);

    xEventGroupWaitBits(openai_event_group,
                        OPENAI_DONE_BIT,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);

    return final_response;
}
