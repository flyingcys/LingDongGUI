#include "internal.h"

#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldWindow.h"

#include <stdio.h>

int main(void)
{
#define PRINT_SIZE(name, type) printf("TINYUI_SIZEOF_%s=%zu\n", name, sizeof(type))
    PRINT_SIZE("WIDGET_WRAPPER", struct tinyui_widget);
    PRINT_SIZE("WINDOW_WRAPPER", struct tinyui_window);
    PRINT_SIZE("BACKGROUND_WRAPPER", struct tinyui_background);
    PRINT_SIZE("CANVAS_WRAPPER", struct tinyui_canvas);
    PRINT_SIZE("LABEL_WRAPPER", struct tinyui_label);
    PRINT_SIZE("BUTTON_WRAPPER", struct tinyui_button);
    PRINT_SIZE("KEYBOARD_WRAPPER", struct tinyui_keyboard);
    PRINT_SIZE("CHECKBOX_WRAPPER", struct tinyui_checkbox);
    PRINT_SIZE("SWITCH_WRAPPER", struct tinyui_switch);
    PRINT_SIZE("SLIDER_WRAPPER", struct tinyui_slider);
    PRINT_SIZE("PROGRESS_BAR_WRAPPER", struct tinyui_progress_bar);
    PRINT_SIZE("ANIMATION_WRAPPER", struct tinyui_animation);
    PRINT_SIZE("ARC_WRAPPER", struct tinyui_arc);
    PRINT_SIZE("GAUGE_WRAPPER", struct tinyui_gauge);
    PRINT_SIZE("ICON_SLIDER_WRAPPER", struct tinyui_icon_slider);
    PRINT_SIZE("RADIAL_MENU_WRAPPER", struct tinyui_radial_menu);
    PRINT_SIZE("QRCODE_WRAPPER", struct tinyui_qrcode);
    PRINT_SIZE("PROGRESS_WHEEL_WRAPPER", struct tinyui_progress_wheel);
    PRINT_SIZE("DATE_TIME_WRAPPER", struct tinyui_date_time);
    PRINT_SIZE("CALENDAR_WRAPPER", struct tinyui_calendar);
    PRINT_SIZE("CLOCK_WRAPPER", struct tinyui_clock);
    PRINT_SIZE("LIST_WRAPPER", struct tinyui_list);
    PRINT_SIZE("MESSAGE_BOX_WRAPPER", struct tinyui_message_box);
    PRINT_SIZE("TEXT_WRAPPER", struct tinyui_text);
    PRINT_SIZE("LINE_EDIT_WRAPPER", struct tinyui_line_edit);
    PRINT_SIZE("COMBO_BOX_WRAPPER", struct tinyui_combo_box);
    PRINT_SIZE("SCROLL_SELECTER_WRAPPER", struct tinyui_scroll_selecter);
    PRINT_SIZE("IMAGE_WRAPPER", struct tinyui_image);
    PRINT_SIZE("GRAPH_WRAPPER", struct tinyui_graph);
    PRINT_SIZE("TABLE_WRAPPER", struct tinyui_table);
    printf("TINYUI_SIZEOF_SWITCH_WRAPPER_DELTA=%zu\n",
           sizeof(struct tinyui_switch) - sizeof(struct tinyui_widget));
    /* ldWindow_t is the concrete backend container used by TinyUI roots. */
    PRINT_SIZE("BACKEND_WIDGET_LD_WINDOW", ldWindow_t);
    PRINT_SIZE("LEGACY_TIMER_NODE", struct tinyui_app_timer);
    PRINT_SIZE("LEGACY_EVENT_CALLBACK_STORAGE", xBtnInfo_t);
    PRINT_SIZE("RUNTIME_BOOKKEEPING", struct tinyui_app);
#undef PRINT_SIZE
    if (sizeof(struct tinyui_widget) > 192) {
        fprintf(stderr,
                "widget wrapper size regression: expected <= 192, got %zu\n",
                sizeof(struct tinyui_widget));
        return 1;
    }
    if (sizeof(ldWindow_t) > 512) {
        fprintf(stderr,
                "backend widget size regression: expected <= 512, got %zu\n",
                sizeof(ldWindow_t));
        return 1;
    }
    return 0;
}
