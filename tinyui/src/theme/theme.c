/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "internal.h"
#include "runtime_bridge.h"
#include "theme/theme.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldCalendar.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldImage.h"
#include "../../../src/gui/ldLabel.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/gui/ldSlider.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/gui/ldText.h"
#include "../../../src/gui/ldWindow.h"

#include <string.h>
#include <stdint.h>

void ldBaseSetHeight(ldBase_t *ptWidget, int16_t height);

static const unsigned int TINYUI_THEME_V0_COLORS[TINYUI_COLOR_COUNT] = {
    [TINYUI_COLOR_TEXT_PRIMARY] = 0x1F2328U,
    [TINYUI_COLOR_BG] = 0xF6F8FAU,
    [TINYUI_COLOR_PANEL] = 0xFFFFFFU,
    [TINYUI_COLOR_BORDER] = 0xD0D7DEU,
    [TINYUI_COLOR_ACCENT] = 0x0969DAU,
    [TINYUI_COLOR_DISABLED] = 0x8C959FU,
};

static const int TINYUI_THEME_V0_METRICS[TINYUI_METRIC_COUNT] = {
    [TINYUI_METRIC_PADDING] = 8,
    [TINYUI_METRIC_RADIUS] = 6,
    [TINYUI_METRIC_BORDER_WIDTH] = 1,
    [TINYUI_METRIC_CONTROL_HEIGHT] = 32,
};

static int tinyui_theme_state_is_valid(enum tinyui_state state)
{
    return state >= TINYUI_STATE_DEFAULT && state <= TINYUI_STATE_FOCUSED;
}

static int tinyui_theme_part_is_valid(enum tinyui_part part)
{
    return part >= TINYUI_PART_MAIN && part <= TINYUI_PART_TRACK;
}

static int tinyui_theme_part_supported(enum tinyui_backend_widget_kind kind, enum tinyui_part part)
{
    switch (kind) {
        case TINYUI_BACKEND_WIDGET_WINDOW:
            return part == TINYUI_PART_MAIN;
        case TINYUI_BACKEND_WIDGET_IMAGE:
            return part == TINYUI_PART_MAIN;
        case TINYUI_BACKEND_WIDGET_CALENDAR:
            return part == TINYUI_PART_MAIN || part == TINYUI_PART_TEXT;
        case TINYUI_BACKEND_WIDGET_LABEL:
        case TINYUI_BACKEND_WIDGET_TEXT:
            return part == TINYUI_PART_MAIN || part == TINYUI_PART_TEXT;
        case TINYUI_BACKEND_WIDGET_LIST:
            return part == TINYUI_PART_MAIN || part == TINYUI_PART_TEXT;
        case TINYUI_BACKEND_WIDGET_BUTTON:
            return part == TINYUI_PART_MAIN || part == TINYUI_PART_TEXT;
        case TINYUI_BACKEND_WIDGET_CHECKBOX:
            return part == TINYUI_PART_MAIN || part == TINYUI_PART_TEXT
                   || part == TINYUI_PART_INDICATOR;
        case TINYUI_BACKEND_WIDGET_SWITCH:
            return part == TINYUI_PART_MAIN || part == TINYUI_PART_INDICATOR
                   || part == TINYUI_PART_KNOB || part == TINYUI_PART_TRACK;
        case TINYUI_BACKEND_WIDGET_SLIDER:
            return part == TINYUI_PART_MAIN || part == TINYUI_PART_KNOB || part == TINYUI_PART_TRACK;
        default:
            return 0;
    }
}

static void tinyui_theme_map_widget_colors(const struct tinyui_theme *theme,
                                           enum tinyui_part part,
                                           enum tinyui_state state,
                                           unsigned int *bg,
                                           unsigned int *text,
                                           unsigned int *border)
{
    unsigned int bg_color = theme->colors[TINYUI_COLOR_PANEL];
    unsigned int text_color = theme->colors[TINYUI_COLOR_TEXT_PRIMARY];
    unsigned int border_color = theme->colors[TINYUI_COLOR_BORDER];
    int active_state = state == TINYUI_STATE_PRESSED || state == TINYUI_STATE_CHECKED
                       || state == TINYUI_STATE_FOCUSED;

    if (part == TINYUI_PART_TRACK) {
        bg_color = theme->colors[TINYUI_COLOR_BORDER];
    }

    if (state == TINYUI_STATE_DISABLED) {
        bg_color = theme->colors[TINYUI_COLOR_BG];
        text_color = theme->colors[TINYUI_COLOR_DISABLED];
        border_color = theme->colors[TINYUI_COLOR_DISABLED];
    } else if (active_state) {
        if (part == TINYUI_PART_TEXT) {
            text_color = theme->colors[TINYUI_COLOR_ACCENT];
        } else if (part == TINYUI_PART_KNOB) {
            bg_color = theme->colors[TINYUI_COLOR_ACCENT];
        } else {
            bg_color = theme->colors[TINYUI_COLOR_ACCENT];
            border_color = theme->colors[TINYUI_COLOR_ACCENT];
        }
    }

    *bg = bg_color;
    *text = text_color;
    *border = border_color;
}

static int tinyui_theme_apply_widget_metrics(const struct tinyui_theme *theme,
                                             struct tinyui_widget *widget,
                                             enum tinyui_backend_widget_kind kind)
{
    if (kind != TINYUI_BACKEND_WIDGET_WINDOW && kind != TINYUI_BACKEND_WIDGET_IMAGE) {
        widget->height = theme->metrics[TINYUI_METRIC_CONTROL_HEIGHT];
        if (widget->ld_widget != 0) {
            ldBaseSetHeight((ldBase_t *)widget->ld_widget,
                            (int16_t)theme->metrics[TINYUI_METRIC_CONTROL_HEIGHT]);
        }
    }

    if (tinyui_widget_set_radius(widget, theme->metrics[TINYUI_METRIC_RADIUS]) != 0
        || tinyui_widget_set_padding(widget, theme->metrics[TINYUI_METRIC_PADDING]) != 0) {
        return -1;
    }

    return 0;
}

static int tinyui_theme_backend_can_apply_style(const struct tinyui_widget *widget)
{
    return widget != 0
        && widget->ld_widget != 0
        && widget->owner != 0
        && widget->owner->theme != 0;
}

static void tinyui_theme_apply_window_style(struct tinyui_widget *widget,
                                            unsigned int bg_color)
{
    ldWindowSetColor((ldWindow_t *)widget->ld_widget, tinyui_rgb_to_ld_color(bg_color));
}

static void tinyui_theme_apply_label_style(struct tinyui_widget *widget,
                                           unsigned int bg_color,
                                           unsigned int text_color)
{
    ldLabel_t *ld_label = (ldLabel_t *)widget->ld_widget;

    ldLabelSetBackgroundColor(ld_label, tinyui_rgb_to_ld_color(bg_color));
    ldLabelSetTextColor(ld_label, tinyui_rgb_to_ld_color(text_color));
}

static void tinyui_theme_apply_text_style(struct tinyui_widget *widget,
                                          unsigned int bg_color,
                                          unsigned int text_color)
{
    ldText_t *ld_text = (ldText_t *)widget->ld_widget;

    ldTextSetBackgroundColor(ld_text, tinyui_rgb_to_ld_color(bg_color));
    ldTextSetTextColor(ld_text, tinyui_rgb_to_ld_color(text_color));
}

static void tinyui_theme_apply_button_style(struct tinyui_widget *widget,
                                            enum tinyui_state state,
                                            unsigned int bg_color,
                                            unsigned int text_color)
{
    ldButton_t *ld_button = (ldButton_t *)widget->ld_widget;
    ldColor release_color = tinyui_rgb_to_ld_color(bg_color);
    ldColor press_color = tinyui_rgb_to_ld_color(bg_color);

    if (state != TINYUI_STATE_PRESSED) {
        press_color = tinyui_rgb_to_ld_color(widget->owner->theme->colors[TINYUI_COLOR_ACCENT]);
    }

    ldButtonSetColor(ld_button, release_color, press_color);
    ldButtonSetTextColor(ld_button, tinyui_rgb_to_ld_color(text_color));
}

static void tinyui_theme_apply_checkbox_style(struct tinyui_widget *widget,
                                              enum tinyui_part part,
                                              unsigned int bg_color,
                                              unsigned int text_color,
                                              unsigned int border_color)
{
    ldCheckBox_t *ld_checkbox = (ldCheckBox_t *)widget->ld_widget;
    ldColor main_bg = tinyui_rgb_to_ld_color(widget->owner->theme->colors[TINYUI_COLOR_PANEL]);
    ldColor indicator_color = tinyui_rgb_to_ld_color(border_color);

    if (part == TINYUI_PART_INDICATOR) {
        indicator_color = tinyui_rgb_to_ld_color(bg_color);
    } else if (part == TINYUI_PART_MAIN) {
        main_bg = tinyui_rgb_to_ld_color(bg_color);
    }

    ldCheckBoxSetColor(ld_checkbox, main_bg, indicator_color);
    ldCheckBoxSetTextColor(ld_checkbox, tinyui_rgb_to_ld_color(text_color));
}

static void tinyui_theme_apply_switch_style(struct tinyui_widget *widget,
                                            enum tinyui_part part,
                                            unsigned int bg_color,
                                            unsigned int border_color)
{
    ldSwitch_t *ld_switch = (ldSwitch_t *)widget->ld_widget;
    unsigned int off_track = widget->owner->theme->colors[TINYUI_COLOR_BORDER];
    unsigned int on_track = widget->owner->theme->colors[TINYUI_COLOR_ACCENT];
    unsigned int knob_color = bg_color;
    unsigned int edge_color = widget->owner->theme->colors[TINYUI_COLOR_BORDER];

    switch (part) {
    case TINYUI_PART_MAIN:
        off_track = bg_color;
        edge_color = border_color;
        break;
    case TINYUI_PART_INDICATOR:
        on_track = bg_color;
        edge_color = border_color;
        break;
    case TINYUI_PART_KNOB:
        knob_color = bg_color;
        edge_color = border_color;
        break;
    case TINYUI_PART_TRACK:
        off_track = bg_color;
        edge_color = border_color;
        break;
    default:
        break;
    }

    ldSwitchSetColor(ld_switch,
                     tinyui_rgb_to_ld_color(off_track),
                     tinyui_rgb_to_ld_color(on_track),
                     tinyui_rgb_to_ld_color(knob_color),
                     tinyui_rgb_to_ld_color(edge_color));
}

static void tinyui_theme_apply_slider_style(struct tinyui_widget *widget,
                                            enum tinyui_part part,
                                            unsigned int bg_color,
                                            unsigned int border_color)
{
    ldSlider_t *ld_slider = (ldSlider_t *)widget->ld_widget;
    unsigned int slider_bg = widget->owner->theme->colors[TINYUI_COLOR_PANEL];
    unsigned int slider_frame = widget->owner->theme->colors[TINYUI_COLOR_BORDER];
    unsigned int slider_indic = widget->owner->theme->colors[TINYUI_COLOR_ACCENT];

    switch (part) {
    case TINYUI_PART_MAIN:
        slider_bg = bg_color;
        slider_frame = border_color;
        break;
    case TINYUI_PART_KNOB:
        slider_indic = bg_color;
        break;
    case TINYUI_PART_TRACK:
        slider_bg = bg_color;
        slider_frame = border_color;
        break;
    default:
        break;
    }

    ldSliderSetColor(ld_slider,
                     tinyui_rgb_to_ld_color(slider_bg),
                     tinyui_rgb_to_ld_color(slider_frame),
                     tinyui_rgb_to_ld_color(slider_indic));
}

static void tinyui_theme_apply_list_style(struct tinyui_widget *widget,
                                          enum tinyui_part part,
                                          unsigned int bg_color,
                                          unsigned int text_color,
                                          unsigned int border_color)
{
    ldList_t *ld_list = (ldList_t *)widget->ld_widget;

    switch (part) {
    case TINYUI_PART_MAIN:
        ldListSetBackgroundColor(ld_list, tinyui_rgb_to_ld_color(bg_color));
        ldListSetSelectColor(ld_list, tinyui_rgb_to_ld_color(border_color));
        break;
    case TINYUI_PART_TEXT:
        ldListSetTextColor(ld_list, tinyui_rgb_to_ld_color(text_color));
        break;
    default:
        break;
    }
}

static void tinyui_theme_apply_image_style(struct tinyui_widget *widget,
                                           unsigned int bg_color)
{
    ldImageSetMaskColor((ldImage_t *)widget->ld_widget, tinyui_rgb_to_ld_color(bg_color));
}

static void tinyui_theme_apply_calendar_style(struct tinyui_widget *widget,
                                              enum tinyui_part part,
                                              unsigned int bg_color,
                                              unsigned int text_color,
                                              unsigned int border_color)
{
    ldCalendar_t *ld_calendar = (ldCalendar_t *)widget->ld_widget;

    switch (part) {
    case TINYUI_PART_MAIN:
        ld_calendar->bgColor = tinyui_rgb_to_ld_color(bg_color);
        ld_calendar->itemColor = tinyui_rgb_to_ld_color(border_color);
        break;
    case TINYUI_PART_TEXT:
        ld_calendar->textColor = tinyui_rgb_to_ld_color(text_color);
        break;
    default:
        break;
    }
}

static int tinyui_theme_apply_native_widget_style(struct tinyui_widget *widget,
                                                  enum tinyui_part part,
                                                  enum tinyui_state state,
                                                  unsigned int bg_color,
                                                  unsigned int text_color,
                                                  unsigned int border_color)
{
    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_WINDOW:
        tinyui_theme_apply_window_style(widget, bg_color);
        break;
    case TINYUI_BACKEND_WIDGET_LABEL:
        tinyui_theme_apply_label_style(widget, bg_color, text_color);
        break;
    case TINYUI_BACKEND_WIDGET_TEXT:
        tinyui_theme_apply_text_style(widget, bg_color, text_color);
        break;
    case TINYUI_BACKEND_WIDGET_BUTTON:
        tinyui_theme_apply_button_style(widget, state, bg_color, text_color);
        break;
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
        tinyui_theme_apply_checkbox_style(widget, part, bg_color, text_color, border_color);
        break;
    case TINYUI_BACKEND_WIDGET_SWITCH:
        tinyui_theme_apply_switch_style(widget, part, bg_color, border_color);
        break;
    case TINYUI_BACKEND_WIDGET_SLIDER:
        tinyui_theme_apply_slider_style(widget, part, bg_color, border_color);
        break;
    case TINYUI_BACKEND_WIDGET_LIST:
        tinyui_theme_apply_list_style(widget, part, bg_color, text_color, border_color);
        break;
    case TINYUI_BACKEND_WIDGET_IMAGE:
        tinyui_theme_apply_image_style(widget, bg_color);
        break;
    case TINYUI_BACKEND_WIDGET_CALENDAR:
        tinyui_theme_apply_calendar_style(widget, part, bg_color, text_color, border_color);
        break;
    default:
        return -1;
    }

    return 0;
}

int tinyui_theme_apply_widget_style(struct tinyui_widget *widget,
                                    enum tinyui_part part,
                                    enum tinyui_state state,
                                    unsigned int bg_color,
                                    unsigned int text_color,
                                    unsigned int border_color)
{
    if (!tinyui_theme_backend_can_apply_style(widget)) {
        return -1;
    }

    if (!tinyui_theme_part_is_valid(part)
        || !tinyui_theme_state_is_valid(state)
        || !tinyui_theme_part_supported(widget->kind, part)) {
        return -1;
    }

    return tinyui_theme_apply_native_widget_style(widget,
                                                  part,
                                                  state,
                                                  bg_color,
                                                  text_color,
                                                  border_color);
}

/**
 * @brief Create theme instance
 *
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_theme *tinyui_theme_create(void)
{
    struct tinyui_theme *theme = ldCalloc(1, sizeof(struct tinyui_theme));
    int i;

    if (theme == 0) {
        return 0;
    }

    for (i = 0; i < TINYUI_COLOR_COUNT; ++i) {
        theme->colors[i] = TINYUI_THEME_V0_COLORS[i];
    }

    for (i = 0; i < TINYUI_METRIC_COUNT; ++i) {
        theme->metrics[i] = TINYUI_THEME_V0_METRICS[i];
    }

    return theme;
}

/**
 * @brief Destroy theme instance
 *
 * @param[in] theme Theme instance
 */

void tinyui_theme_destroy(struct tinyui_theme *theme)
{
    ldFree(theme);
}

/**
 * @brief Set color of theme
 *
 * @param[in] theme Theme instance
 * @param[in] id Widget identifier string
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_theme_set_color(struct tinyui_theme *theme, enum tinyui_color_id id, unsigned int rgb)
{
    if (theme == 0 || id < 0 || id >= TINYUI_COLOR_COUNT) {
        return -1;
    }

    theme->colors[id] = rgb;
    return 0;
}

/**
 * @brief Set metric of theme
 *
 * @param[in] theme Theme instance
 * @param[in] id Widget identifier string
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int tinyui_theme_set_metric(struct tinyui_theme *theme, enum tinyui_metric_id id, int value)
{
    if (theme == 0 || id < 0 || id >= TINYUI_METRIC_COUNT || value < 0) {
        return -1;
    }

    theme->metrics[id] = value;
    return 0;
}

/**
 * @brief Theme: apply to widget
 *
 * @param[in] theme Theme instance
 * @param[in] widget Widget instance
 * @param[in] part part
 * @param[in] state State value
 * @return -1 on failure
 */

int tinyui_theme_apply_to_widget(struct tinyui_theme *theme,
                                 struct tinyui_widget *widget,
                                 enum tinyui_part part,
                                 enum tinyui_state state)
{
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;

    if (theme == 0 || widget == 0 || widget->ld_widget == 0
        || widget->owner == 0 || widget->owner->theme == 0) {
        return -1;
    }

    if (!tinyui_theme_part_is_valid(part) || !tinyui_theme_state_is_valid(state)) {
        return -1;
    }

    if (!tinyui_theme_part_supported(widget->kind, part)) {
        return -1;
    }
    /* C1: use widget->ld_widget + widget->kind instead of backend fields */
    if (widget->ld_widget == 0) {
        return -1;
    }

    tinyui_theme_map_widget_colors(theme, part, state, &bg_color, &text_color, &border_color);
    if (tinyui_theme_apply_widget_metrics(theme, widget, widget->kind) != 0) {
        return -1;
    }

    widget->bg_color = bg_color;
    widget->text_color = text_color;
    widget->border_color = border_color;

    /* C3-T1: native helpers read folded fields directly (ld_widget/kind/owner->theme) */
    return tinyui_theme_apply_native_widget_style(widget,
                                                  part,
                                                  state,
                                                  bg_color,
                                                  text_color,
                                                  border_color);
}

/**
 * @brief Set theme of app
 *
 * @param[in] app Application instance
 * @param[in] theme Theme instance
 * @return -1 on failure
 */

int tinyui_app_set_theme(struct tinyui_app *app, struct tinyui_theme *theme)
{
    if (app == 0 || theme == 0) {
        return -1;
    }

    return tinyui_runtime_bridge_bind_theme(app, theme);
}
