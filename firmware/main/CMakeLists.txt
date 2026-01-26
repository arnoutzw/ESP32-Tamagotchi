idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES display input pet game save_manager sprites nvs_flash esp_timer
)
