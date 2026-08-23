idf_component_register(
    SRCS
        "lcd.c"
        "touch.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_lcd esp_timer
)
