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
#include "widgets/text.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldText.h"

#include <stdlib.h>
#include <string.h>

struct tinyui_image_source;

/* ---- test seam state ---- */
static int s_fail_next_set_font = 0;


static arm_2d_font_t *tinyui_text_resolve_font_internal(const struct tinyui_font *font)
{
    return tinyui_resolve_ld_font(font, 12);
}

void tinyui_text_test_fail_next_set_font(void)
{
    s_fail_next_set_font = 1;
}

static int text_props_valid(const struct tinyui_text_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_text_ld_init(void *ctx,
                                 struct ld_scene_t *scene,
                                 uint16_t name_id,
                                 uint16_t parent_name_id)
{
    (void)ctx;
    return ldText_init(scene,
                       NULL,
                       name_id,
                       parent_name_id,
                       0,
                       0,
                       220,
                       48,
                       NULL,
                       TEXT_BOX_LINE_ALIGN_LEFT,
                       false);
}

/**
 * @brief Create text widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_text *tinyui_text_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_text *text;

    if (parent == 0 || id == 0) {
        return 0;
    }
    text = (struct tinyui_text *)tinyui_widget_create_leaf(&parent->widget,
                                                           TINYUI_BACKEND_WIDGET_TEXT,
                                                           tinyui_text_ld_init,
                                                           0,
                                                           sizeof(*text));
    if (text == 0) {
        return 0;
    }
    text->id = id;
    if (tinyui_text_set_font(text, NULL) != 0) {
        tinyui_widget_destroy_common(&text->widget);
        return 0;
    }

    return text;
}

/**
 * @brief Create text widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_text *tinyui_text_create_with_props(struct tinyui_window *parent,
                                                  const struct tinyui_text_props *props)
{
    struct tinyui_text *text;

    if (!text_props_valid(props)) {
        return 0;
    }

    text = tinyui_text_create(parent, props->id);
    if (text == 0) {
        return 0;
    }

    if ((props->text != 0 && tinyui_text_set_text(text, props->text) != 0)
        || (props->font != 0 && tinyui_text_set_font(text, props->font) != 0)
        || (props->style_class != 0
            && tinyui_widget_set_style_class(&text->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&text->widget, props->user_data) != 0
        || tinyui_widget_set_bg_color(&text->widget, props->bg_color) != 0
        || tinyui_widget_set_text_color(&text->widget, props->text_color) != 0
        || tinyui_widget_set_border_color(&text->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&text->widget, props->radius) != 0
        || tinyui_widget_set_padding(&text->widget, props->padding) != 0
        || ((props->width > 0 || props->height > 0)
            && tinyui_widget_set_size(&text->widget, props->width, props->height) != 0)) {
        tinyui_widget_destroy_common(&text->widget);
        return 0;
    }

    return text;
}

/**
 * @brief Set text of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] value Value
 * @return -1 on failure
 */

int tinyui_text_set_text(struct tinyui_text *text, const char *value)
{
    if (text == 0 || value == 0) {
        return -1;
    }

    if (tinyui_widget_set_text(&text->widget, value) != 0) {
        return -1;
    }
    return tinyui_widget_set_backend_text(&text->widget, value);
}

/**
 * @brief Set static text of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int tinyui_text_set_static_text(struct tinyui_text *text, const char *value)
{
    if (text == 0 || value == 0 || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextSetStaticText((ldText_t *)text->widget.ld_widget, (const uint8_t *)value);
    text->widget.text = value;
    return 0;
}

/**
 * @brief Set font of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int tinyui_text_set_font(struct tinyui_text *text, const struct tinyui_font *font)
{
    ldText_t *ld_text;
    arm_2d_font_t *resolved_font;

    if (text == 0 || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ld_text = (ldText_t *)text->widget.ld_widget;

    if (s_fail_next_set_font != 0) {
        s_fail_next_set_font = 0;
        return -1;
    }

    resolved_font = tinyui_text_resolve_font_internal(font);
    if (resolved_font == NULL) {
        return -1;
    }

    if (font != NULL && ldTextSetFont(ld_text, resolved_font) != 0) {
        return -1;
    }
    if (font == NULL && ldTextSetConsumedFont(ld_text, resolved_font) != 0) {
        return -1;
    }

    text->widget.font = font;
    return 0;
}

/**
 * @brief Set transparent of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] transparent transparent
 * @return -1 on failure
 */

int tinyui_text_set_transparent(struct tinyui_text *text, int transparent)
{
    if (text == 0 || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextSetTransparent((ldText_t *)text->widget.ld_widget, transparent != 0);
    return 0;
}

/**
 * @brief Set text color of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_text_set_text_color(struct tinyui_text *text, unsigned int rgb)
{
    if (text == 0 || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextSetTextColor((ldText_t *)text->widget.ld_widget,
                       (ldColor)tinyui_rgb_to_ld_color(rgb));
    text->widget.text_color = rgb;
    return 0;
}

/**
 * @brief Set bg color of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_text_set_bg_color(struct tinyui_text *text, unsigned int rgb)
{
    if (text == 0 || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextSetBackgroundColor((ldText_t *)text->widget.ld_widget,
                             (ldColor)tinyui_rgb_to_ld_color(rgb));
    text->widget.bg_color = rgb;
    return 0;
}

/**
 * @brief Set background source of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_text_set_background_source(struct tinyui_text *text,
                                      struct tinyui_image_source *source)
{
    if (text == 0 || (source != 0 && tinyui_image_source_get_image_tile(source) == 0)
        || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextSetBackgroundImage((ldText_t *)text->widget.ld_widget,
                             source != NULL ? tinyui_image_source_get_image_tile(source) : NULL,
                             source != NULL ? tinyui_image_source_get_mask_tile(source) : NULL);
    return 0;
}

/**
 * @brief Set consumed font of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int tinyui_text_set_consumed_font(struct tinyui_text *text, const struct tinyui_font *font)
{
    return tinyui_text_set_font(text, font);
}

int tinyui_text_set_scroll_enabled(struct tinyui_text *text, int enabled)
{
    if (text == 0 || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextSetScrollEnabled((ldText_t *)text->widget.ld_widget, enabled != 0);
    return 0;
}

/**
 * @brief text scroll seek
 *
 * @param[in] text Text widget instance
 * @param[in] offset Offset
 * @return -1 on failure
 */

int tinyui_text_scroll_seek(struct tinyui_text *text, int offset)
{
    if (text == 0 || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextScrollSeek((ldText_t *)text->widget.ld_widget, (int16_t)offset);
    return 0;
}

/**
 * @brief text scroll move
 *
 * @param[in] text Text widget instance
 * @param[in] move_value move value
 * @return -1 on failure
 */

int tinyui_text_scroll_move(struct tinyui_text *text, int move_value)
{
    if (text == 0 || move_value < -128 || move_value > 127
        || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextScrollMove((ldText_t *)text->widget.ld_widget, (int8_t)move_value);
    return 0;
}
