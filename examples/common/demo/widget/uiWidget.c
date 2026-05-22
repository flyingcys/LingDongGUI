#include "uiWidgetCommon.h"

const uint8_t *g_widget_scroll_items[5] = {
    (const uint8_t *)"1",
    (const uint8_t *)"10",
    (const uint8_t *)"123",
    (const uint8_t *)"99",
    (const uint8_t *)"7",
};

const uint8_t *g_widget_icon_names[5] = {
    (const uint8_t *)"11",
    (const uint8_t *)"22",
    (const uint8_t *)"33",
    (const uint8_t *)"44",
    (const uint8_t *)"55",
};

const uint8_t *g_widget_combo_box_items[3] = {
    (const uint8_t *)"11",
    (const uint8_t *)"22",
    (const uint8_t *)"00",
};

const uint8_t g_widget_message_title[] = "title";
const uint8_t g_widget_message_text[] = "12345678abcdefg\n99556";

const uint8_t *g_widget_message_buttons[3] = {
    (const uint8_t *)"11",
    (const uint8_t *)"22",
    (const uint8_t *)"33",
};

uint8_t *g_widget_day_names[7] = {
    (uint8_t *)"Sun",
    (uint8_t *)"Mon",
    (uint8_t *)"Tue",
    (uint8_t *)"Wed",
    (uint8_t *)"Thu",
    (uint8_t *)"Fir",
    (uint8_t *)"Sat",
};

uint8_t g_widget_header_format[] = "yyyy - mm - dd";

void uiWidgetHandleFocusNavigation(ld_scene_t *ptScene)
{
    if (xBtnGetState(KEY_NUM_UP, BTN_RELEASE)) {
        ldBaseFocusNavigate(ptScene, NAV_UP);
    }

    if (xBtnGetState(KEY_NUM_DOWN, BTN_RELEASE)) {
        ldBaseFocusNavigate(ptScene, NAV_DOWN);
    }

    if (xBtnGetState(KEY_NUM_LEFT, BTN_RELEASE)) {
        ldBaseFocusNavigate(ptScene, NAV_LEFT);
    }

    if (xBtnGetState(KEY_NUM_RIGHT, BTN_RELEASE)) {
        ldBaseFocusNavigate(ptScene, NAV_RIGHT);
    }
}
