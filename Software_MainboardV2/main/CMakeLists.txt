idf_component_register(SRCS "Source/AI_calc_main.c"
                            "Source/AI_calc_UI.c"
                            "Source/AI_calc_keypad.c"
                            "Source/AI_calc_maindisplay.c"
                            "Source/AI_calc_device.c"
                            "Source/AI_calc_sidedisplay.c"
                            "Source/AI_calc_battery.c"
                    INCLUDE_DIRS "Include"
                    REQUIRES driver
                    REQUIRES esp_timer
                    REQUIRES esp_adc
                    REQUIRES u8g2)
