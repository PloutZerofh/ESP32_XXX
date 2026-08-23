idf_component_register(SRCS "main.c" "lcd.c" "lv_port.c" "touch.c"
                           "assets/img_logo.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer esp_lcd driver lvgl)
