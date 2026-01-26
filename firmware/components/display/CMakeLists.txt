idf_component_register(
    SRCS "display.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_lcd spi_flash
    PRIV_REQUIRES esp_timer
)
