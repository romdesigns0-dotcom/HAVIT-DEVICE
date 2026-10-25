#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAVIT_DEFAULT_BACKGROUND_1_EYES,
    HAVIT_DEFAULT_BACKGROUND_2_APP,
    HAVIT_DEFAULT_BACKGROUND_3_SCROLL_BASE,
    HAVIT_DEFAULT_BACKGROUND_4_SCROLL_RING,
} havit_default_background_t;

/** Create the named background; Default 4 lives in the display's fixed top layer. */
lv_obj_t *havit_default_background_create(lv_obj_t *parent,
                                           havit_default_background_t variant);

/** Select Default 2 or the original Default 3/4 layers for scrolling content. */
void havit_default_background_set_scroll_active(bool active);

#ifdef __cplusplus
}
#endif
