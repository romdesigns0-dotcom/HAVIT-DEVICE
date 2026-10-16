#pragma once

#include "esp_display_panel.hpp"
#include "esp_err.h"

esp_err_t havit_board_start(void);
esp_panel::drivers::LCD *havit_board_lcd(void);
esp_panel::drivers::Touch *havit_board_touch(void);
