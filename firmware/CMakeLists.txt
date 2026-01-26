# ESP32 Tamagotchi - Top-level CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Use custom partition table for OTA support
set(PARTITION_TABLE_CSV "${CMAKE_CURRENT_SOURCE_DIR}/partitions.csv")

# Include ESP-IDF project configuration
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp32-tamagotchi)
