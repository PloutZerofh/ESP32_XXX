idf_component_register(
    SRCS
        "main.c"
        "tasks/task_ui.c"
        "ui/screen_display/screen_main.c"
        "ui/assets/images/img_logo.c"
    INCLUDE_DIRS "." "ui"
    REQUIRES bsp lvgl_port lvgl
)
