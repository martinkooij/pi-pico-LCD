# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(LCDexample C CXX ASM)

pico_sdk_init()

# Add executable. Default name is the project name, 
add_executable(LCDexample_bl pico_lcd_example_backlight.cpp)
add_subdirectory(../lcd_display build)

pico_add_extra_outputs(LCDexample_bl)
target_link_libraries(LCDexample_bl pico_stdlib lcd_display)
