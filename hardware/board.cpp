#include "havit_board.h"

#include "esp_display_panel.hpp"

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

esp_err_t havit_board_start(void)
{
    if(!s_board.init()) return ESP_FAIL;
    return s_board.begin() ? ESP_OK : ESP_FAIL;
}
