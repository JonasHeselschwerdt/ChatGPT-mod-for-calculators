idf_component_register(SRCS "network.c" "keyset.c" "UI.c" "main.c"
                    PRIV_REQUIRES esp_wifi nvs_flash
                    INCLUDE_DIRS "."
                    REQUIRES driver)
