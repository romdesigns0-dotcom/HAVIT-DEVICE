#pragma once

#include "esp_display_panel.hpp"
#include "esp_err.h"
#include <stdint.h>

esp_err_t havit_board_start(void);
esp_panel::drivers::LCD *havit_board_lcd(void);
esp_panel::drivers::Touch *havit_board_touch(void);
void havit_board_set_backlight(bool enabled);
void havit_board_set_brightness(uint8_t percent);
