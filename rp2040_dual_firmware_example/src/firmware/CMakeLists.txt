add_executable(firmware main.c)
pico_set_linker_script(firmware ${CMAKE_CURRENT_SOURCE_DIR}/../custom_flash.ld)
target_link_libraries(firmware pico_stdlib)
pico_add_extra_outputs(firmware)