idf_component_register(SRCS "Source/AI_calc_main.c"
                            "Source/AI_calc_UI.c"
                            "Source/AI_calc_keypad.c"
                    INCLUDE_DIRS "Include"
                    REQUIRES driver
                    REQUIRES esp_timer)
