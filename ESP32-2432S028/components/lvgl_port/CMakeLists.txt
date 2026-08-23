idf_component_register(
    SRCS
        "lv_port.c"
        "lv_port_disp.c"
        "lv_port_indev.c"
    INCLUDE_DIRS "include"
    REQUIRES bsp lvgl esp_timer
)
