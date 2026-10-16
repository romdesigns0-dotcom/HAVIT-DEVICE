set(UI_DIR "${CMAKE_CURRENT_LIST_DIR}/../../ui")
set(LV_EDITOR_PROJECT_SOURCES)
include("${UI_DIR}/file_list_gen.cmake")
file(GLOB UI_FONT_FILES "${UI_DIR}/fonts/*.bin")

idf_component_register(
    SRCS "main.cpp" "havit_nvs_store.c" "havit_ui_fonts.c"
        "${UI_DIR}/ui_ring_overlay.c"
        ${LV_EDITOR_PROJECT_SOURCES} "${UI_DIR}/ui.c"
    INCLUDE_DIRS "." "${UI_DIR}"
    REQUIRES havit_os_core nvs_flash esp_timer esp32_display_panel lvgl
        esp_wifi_config esp_wifi esp_event esp_netif esp_http_server esp_http_client json mbedtls
    EMBED_FILES
        "assets/ring1.rgb565a8"
        "assets/ring2.rgb565a8"
        "assets/ring3.rgb565a8"
        "assets/google_logo.rgb565a8"
        "assets/separator.rgb565a8"
        "assets/continue_pill.rgb565a8"
        "assets/arrow.rgb565a8"
        "assets/arrow_welcome.rgb565a8"
        ${UI_FONT_FILES})

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_LVGL_H_INCLUDE_SIMPLE=1 LV_EDITOR_PREVIEW=1)
