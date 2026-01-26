idf_component_register(
    SRCS "save_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash pet esp_timer
)
