cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(dual_firmware_example)
pico_sdk_init()

add_subdirectory(src/updater)
add_subdirectory(src/firmware)