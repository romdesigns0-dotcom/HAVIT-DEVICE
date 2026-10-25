#include "havit_board.h"

#include "esp_display_panel.hpp"
#include "esp_log.h"

using esp_panel::board::Board;

namespace {
Board s_board;
}

esp_panel::drivers::LCD *havit_board_lcd(void)
{
    return s_board.getLCD();
}

esp_panel::drivers::Touch *havit_board_touch(void)
{
    return s_board.getTouch();
}

void havit_board_set_backlight(bool enabled)
{
    auto *backlight = s_board.getBacklight();
    if(backlight == nullptr) return;
    if(enabled) backlight->on();
    else backlight->off();
}

void havit_board_set_brightness(uint8_t percent)
{
    auto *backlight = s_board.getBacklight();
    if(backlight == nullptr) return;
    if(!backlight->setBrightness(percent))
        ESP_LOGW("havit_board", "Backlight brightness change to %u%% failed", (unsigned)percent);
}

esp_err_t havit_board_start(void)
{
    if(!s_board.init()) return ESP_FAIL;
    return s_board.begin() ? ESP_OK : ESP_FAIL;
}
