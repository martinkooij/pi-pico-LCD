add_library(lcd_display INTERFACE)

target_sources(lcd_display INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/lcd_display.cpp
)

target_include_directories(lcd_display INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# Pull in pico libraries that we need
target_link_libraries(lcd_display INTERFACE pico_stdlib pico_time)