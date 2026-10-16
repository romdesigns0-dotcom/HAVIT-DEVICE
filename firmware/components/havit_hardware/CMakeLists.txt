idf_component_register(
    SRCS "../../../hardware/board.cpp" "../../../hardware/display.cpp" "../../../hardware/touch.cpp"
    INCLUDE_DIRS "../../../hardware/include"
    REQUIRES esp32_display_panel esp_timer lvgl)
