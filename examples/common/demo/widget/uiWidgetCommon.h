#ifndef __UI_WIDGET_COMMON_H__
#define __UI_WIDGET_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ldGui.h"
#include "fonts/uiFonts.h"
#include "images/uiImages.h"

#define UI_WIDGET_PAGE_WIDTH           480
#define UI_WIDGET_PAGE_HEIGHT          272
#define UI_WIDGET_MARGIN               12
#define UI_WIDGET_GAP                  10
#define UI_WIDGET_HEADER_HEIGHT        30
#define UI_WIDGET_AUTO_SWITCH_MS       3000
#define UI_WIDGET_DYNAMIC_UPDATE_MS    100

#define UI_WIDGET_KEYBOARD_ID          22

extern const uint8_t *g_widget_scroll_items[5];
extern const uint8_t *g_widget_icon_names[5];
extern const uint8_t *g_widget_combo_box_items[3];
extern const uint8_t g_widget_message_title[];
extern const uint8_t g_widget_message_text[];
extern const uint8_t *g_widget_message_buttons[3];
extern uint8_t *g_widget_day_names[7];
extern uint8_t g_widget_header_format[];

void uiWidgetHandleFocusNavigation(ld_scene_t *ptScene);

#ifdef __cplusplus
}
#endif

#endif
