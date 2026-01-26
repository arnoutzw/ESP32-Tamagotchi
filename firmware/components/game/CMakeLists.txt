idf_component_register(
    SRCS "game.c" "minigame.c"
    INCLUDE_DIRS "include"
    REQUIRES display input pet sprites esp_timer
)
