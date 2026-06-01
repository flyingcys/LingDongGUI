#include "backend.h"
#include "internal.h"
#include "ldButton.h"
#include "ldCalendar.h"
#include "ldCheckBox.h"
#include "ldImage.h"
#include "ldLabel.h"
#include "ldList.h"
#include "ldSlider.h"
#include "ldSwitch.h"
#include "ldText.h"
#include "ldWindow.h"

static ldColor picoui_backend_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void picoui_backend_apply_window_style(struct picoui_backend_widget *backend_widget,
                                              unsigned int bg_color)
{
    ldWindowSetColor((ldWindow_t *)backend_widget->ld_widget, picoui_backend_rgb_to_ld_color(bg_color));
}

static void picoui_backend_apply_label_style(struct picoui_backend_widget *backend_widget,
                                             unsigned int bg_color,
                                             unsigned int text_color)
{
    ldLabel_t *ld_label = (ldLabel_t *)backend_widget->ld_widget;

    ldLabelSetBackgroundColor(ld_label, picoui_backend_rgb_to_ld_color(bg_color));
    ldLabelSetTextColor(ld_label, picoui_backend_rgb_to_ld_color(text_color));
}

static void picoui_backend_apply_text_style(struct picoui_backend_widget *backend_widget,
                                            unsigned int bg_color,
                                            unsigned int text_color)
{
    ldText_t *ld_text = (ldText_t *)backend_widget->ld_widget;

    ldTextSetBackgroundColor(ld_text, picoui_backend_rgb_to_ld_color(bg_color));
    ldTextSetTextColor(ld_text, picoui_backend_rgb_to_ld_color(text_color));
}

static void picoui_backend_apply_button_style(struct picoui_backend_widget *backend_widget,
                                              enum picoui_state state,
                                              unsigned int bg_color,
                                              unsigned int text_color,
                                              unsigned int border_color)
{
    ldButton_t *ld_button = (ldButton_t *)backend_widget->ld_widget;
    ldColor release_color = picoui_backend_rgb_to_ld_color(bg_color);
    ldColor press_color = picoui_backend_rgb_to_ld_color(bg_color);

    (void)border_color;
    if (state != PICOUI_STATE_PRESSED) {
        press_color = picoui_backend_rgb_to_ld_color(backend_widget->theme->colors[PICOUI_COLOR_ACCENT]);
    }

    ldButtonSetColor(ld_button, release_color, press_color);
    ldButtonSetTextColor(ld_button, picoui_backend_rgb_to_ld_color(text_color));
}

static void picoui_backend_apply_checkbox_style(struct picoui_backend_widget *backend_widget,
                                                enum picoui_part part,
                                                unsigned int bg_color,
                                                unsigned int text_color,
                                                unsigned int border_color)
{
    ldCheckBox_t *ld_checkbox = (ldCheckBox_t *)backend_widget->ld_widget;
    ldColor main_bg = picoui_backend_rgb_to_ld_color(backend_widget->theme->colors[PICOUI_COLOR_PANEL]);
    ldColor indicator_color = picoui_backend_rgb_to_ld_color(border_color);

    if (part == PICOUI_PART_INDICATOR) {
        indicator_color = picoui_backend_rgb_to_ld_color(bg_color);
    } else if (part == PICOUI_PART_MAIN) {
        main_bg = picoui_backend_rgb_to_ld_color(bg_color);
    }

    ldCheckBoxSetColor(ld_checkbox, main_bg, indicator_color);
    ldCheckBoxSetTextColor(ld_checkbox, picoui_backend_rgb_to_ld_color(text_color));
}

static void picoui_backend_apply_switch_style(struct picoui_backend_widget *backend_widget,
                                              enum picoui_part part,
                                              unsigned int bg_color,
                                              unsigned int border_color)
{
    ldSwitch_t *ld_switch = (ldSwitch_t *)backend_widget->ld_widget;
    unsigned int off_track = backend_widget->theme->colors[PICOUI_COLOR_BORDER];
    unsigned int on_track = backend_widget->theme->colors[PICOUI_COLOR_ACCENT];
    unsigned int knob_color = bg_color;
    unsigned int edge_color = backend_widget->theme->colors[PICOUI_COLOR_BORDER];

    switch (part) {
    case PICOUI_PART_MAIN:
        off_track = bg_color;
        edge_color = border_color;
        break;
    case PICOUI_PART_INDICATOR:
        on_track = bg_color;
        edge_color = border_color;
        break;
    case PICOUI_PART_KNOB:
        knob_color = bg_color;
        edge_color = border_color;
        break;
    case PICOUI_PART_TRACK:
        off_track = bg_color;
        edge_color = border_color;
        break;
    default:
        break;
    }

    ldSwitchSetColor(ld_switch,
                     picoui_backend_rgb_to_ld_color(off_track),
                     picoui_backend_rgb_to_ld_color(on_track),
                     picoui_backend_rgb_to_ld_color(knob_color),
                     picoui_backend_rgb_to_ld_color(edge_color));
}

static void picoui_backend_apply_slider_style(struct picoui_backend_widget *backend_widget,
                                              enum picoui_part part,
                                              unsigned int bg_color,
                                              unsigned int border_color)
{
    ldSlider_t *ld_slider = (ldSlider_t *)backend_widget->ld_widget;
    unsigned int slider_bg = backend_widget->theme->colors[PICOUI_COLOR_PANEL];
    unsigned int slider_frame = backend_widget->theme->colors[PICOUI_COLOR_BORDER];
    unsigned int slider_indic = backend_widget->theme->colors[PICOUI_COLOR_ACCENT];

    switch (part) {
    case PICOUI_PART_MAIN:
        slider_bg = bg_color;
        slider_frame = border_color;
        break;
    case PICOUI_PART_KNOB:
        slider_indic = bg_color;
        break;
    case PICOUI_PART_TRACK:
        slider_bg = bg_color;
        slider_frame = border_color;
        break;
    default:
        break;
    }

    ldSliderSetColor(ld_slider,
                     picoui_backend_rgb_to_ld_color(slider_bg),
                     picoui_backend_rgb_to_ld_color(slider_frame),
                     picoui_backend_rgb_to_ld_color(slider_indic));
}

static void picoui_backend_apply_list_style(struct picoui_backend_widget *backend_widget,
                                            enum picoui_part part,
                                            unsigned int bg_color,
                                            unsigned int text_color,
                                            unsigned int border_color)
{
    ldList_t *ld_list = (ldList_t *)backend_widget->ld_widget;

    switch (part) {
    case PICOUI_PART_MAIN:
        ldListSetBackgroundColor(ld_list, picoui_backend_rgb_to_ld_color(bg_color));
        ldListSetSelectColor(ld_list, picoui_backend_rgb_to_ld_color(border_color));
        break;
    case PICOUI_PART_TEXT:
        ldListSetTextColor(ld_list, picoui_backend_rgb_to_ld_color(text_color));
        break;
    default:
        break;
    }
}

static void picoui_backend_apply_image_style(struct picoui_backend_widget *backend_widget,
                                             unsigned int bg_color)
{
    ldImageSetMaskColor((ldImage_t *)backend_widget->ld_widget, picoui_backend_rgb_to_ld_color(bg_color));
}

static void picoui_backend_apply_calendar_style(struct picoui_backend_widget *backend_widget,
                                                enum picoui_part part,
                                                unsigned int bg_color,
                                                unsigned int text_color,
                                                unsigned int border_color)
{
    ldCalendar_t *ld_calendar = (ldCalendar_t *)backend_widget->ld_widget;

    switch (part) {
    case PICOUI_PART_MAIN:
        ld_calendar->bgColor = picoui_backend_rgb_to_ld_color(bg_color);
        ld_calendar->itemColor = picoui_backend_rgb_to_ld_color(border_color);
        break;
    case PICOUI_PART_TEXT:
        ld_calendar->textColor = picoui_backend_rgb_to_ld_color(text_color);
        break;
    default:
        break;
    }
}

int picoui_backend_widget_apply_style(void *backend_widget_ptr,
                                      enum picoui_part part,
                                      enum picoui_state state,
                                      unsigned int bg_color,
                                      unsigned int text_color,
                                      unsigned int border_color)
{
    struct picoui_backend_widget *backend_widget = (struct picoui_backend_widget *)backend_widget_ptr;

    if (backend_widget == 0 || backend_widget->ld_widget == 0 || backend_widget->theme == 0) {
        return -1;
    }

    switch (backend_widget->kind) {
    case PICOUI_BACKEND_WIDGET_WINDOW:
        picoui_backend_apply_window_style(backend_widget, bg_color);
        break;
    case PICOUI_BACKEND_WIDGET_LABEL:
        picoui_backend_apply_label_style(backend_widget, bg_color, text_color);
        break;
    case PICOUI_BACKEND_WIDGET_TEXT:
        picoui_backend_apply_text_style(backend_widget, bg_color, text_color);
        break;
    case PICOUI_BACKEND_WIDGET_BUTTON:
        picoui_backend_apply_button_style(backend_widget, state, bg_color, text_color, border_color);
        break;
    case PICOUI_BACKEND_WIDGET_CHECKBOX:
        picoui_backend_apply_checkbox_style(backend_widget, part, bg_color, text_color, border_color);
        break;
    case PICOUI_BACKEND_WIDGET_SWITCH:
        picoui_backend_apply_switch_style(backend_widget, part, bg_color, border_color);
        break;
    case PICOUI_BACKEND_WIDGET_SLIDER:
        picoui_backend_apply_slider_style(backend_widget, part, bg_color, border_color);
        break;
    case PICOUI_BACKEND_WIDGET_LIST:
        picoui_backend_apply_list_style(backend_widget, part, bg_color, text_color, border_color);
        break;
    case PICOUI_BACKEND_WIDGET_IMAGE:
        picoui_backend_apply_image_style(backend_widget, bg_color);
        break;
    case PICOUI_BACKEND_WIDGET_CALENDAR:
        picoui_backend_apply_calendar_style(backend_widget, part, bg_color, text_color, border_color);
        break;
    default:
        return -1;
    }

    return 0;
}
