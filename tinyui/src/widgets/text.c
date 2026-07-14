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


static struct tinyui_text *tinyui_text_as_text(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_TEXT)) {
        return 0;
    }
    return (struct tinyui_text *)w;
}

static const struct tinyui_text *tinyui_text_as_text_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_TEXT)) {
        return 0;
    }
    return (const struct tinyui_text *)w;
}

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

static int text_props_valid(const tinyui_text_props_t *props)
{
    return props != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_runtime_internal_text_ld_init(void *ctx,
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

tinyui_obj_t *tinyui_text_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "text";
    if (parent_w == 0) { return 0; }

    struct tinyui_text *text;

    if (parent_w == 0 || id == 0) {
        return 0;
    }
    text = (struct tinyui_text *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                           TINYUI_BACKEND_WIDGET_TEXT,
                                                           tinyui_runtime_internal_text_ld_init,
                                                           0,
                                                           sizeof(*text));
    if (text == 0) {
        return 0;
    }
    text->id = id;
    if (tinyui_text_set_font(text, NULL) != 0) {
        tinyui_runtime_internal_widget_destroy_common(&text->widget);
        return 0;
    }

    return (tinyui_obj_t *)text;
}

/**
 * @brief Create text widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_text_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_text_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_text *text;

    if (props == 0) {
        return tinyui_text_create(parent);
    }

    obj = tinyui_text_create(parent);
    if (obj == 0) {
        return 0;
    }
    text = (struct tinyui_text *)(void *)obj;

    if ((props->fields & TINYUI_TEXT_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_TEXT_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&text->widget, props->user_data) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }
    if ((props->fields & TINYUI_TEXT_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&text->widget, props->style_class) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }
        if ((props->fields & TINYUI_TEXT_FIELD_WIDTH) != 0 || (props->fields & TINYUI_TEXT_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&text->widget);
        int h = tinyui_runtime_internal_widget_get_height(&text->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_TEXT_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_TEXT_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&text->widget, w, h) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)text);
            return 0;
        }
    }
    if ((props->fields & TINYUI_TEXT_FIELD_TEXT) != 0) {
    if (tinyui_text_set_text((tinyui_obj_t *)text, props->text) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }
    if ((props->fields & TINYUI_TEXT_FIELD_FONT) != 0) {
    if (tinyui_text_set_font((tinyui_obj_t *)text, props->font) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }
    if ((props->fields & TINYUI_TEXT_FIELD_BG_COLOR) != 0) {
    if (tinyui_text_set_bg_color((tinyui_obj_t *)text, props->bg_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }
    if ((props->fields & TINYUI_TEXT_FIELD_TEXT_COLOR) != 0) {
    if (tinyui_text_set_text_color((tinyui_obj_t *)text, props->text_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }
    if ((props->fields & TINYUI_TEXT_FIELD_BORDER_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_border_color(&text->widget, props->border_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }
    if ((props->fields & TINYUI_TEXT_FIELD_RADIUS) != 0) {
    if (tinyui_runtime_internal_widget_set_radius(&text->widget, props->radius) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }
    if ((props->fields & TINYUI_TEXT_FIELD_PADDING) != 0) {
    if (tinyui_runtime_internal_widget_set_padding(&text->widget, props->padding) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)text);
        return 0;
    }
    }

    return obj;
}


/**
 * @brief Set text of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] value Value
 * @return -1 on failure
 */

int tinyui_text_set_text(tinyui_obj_t *text_obj, const char *value)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

    if (text == 0 || value == 0) {
        return -1;
    }

    if (tinyui_runtime_internal_widget_set_text(&text->widget, value) != 0) {
        return -1;
    }
    return tinyui_runtime_internal_widget_set_backend_text(&text->widget, value);
}

/**
 * @brief Set static text of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int tinyui_text_set_static_text(tinyui_obj_t *text_obj, const char *value)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

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

int tinyui_text_set_font(tinyui_obj_t *text_obj, const struct tinyui_font *font)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

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

int tinyui_text_set_transparent(tinyui_obj_t *text_obj, int transparent)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

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

int tinyui_text_set_text_color(tinyui_obj_t *text_obj, unsigned int rgb)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

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

int tinyui_text_set_bg_color(tinyui_obj_t *text_obj, unsigned int rgb)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

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

int tinyui_text_set_background_source(tinyui_obj_t *text_obj, struct tinyui_image_source *source)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

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

int tinyui_text_set_consumed_font(tinyui_obj_t *text_obj, const struct tinyui_font *font)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

    return tinyui_text_set_font((tinyui_obj_t *)text, font);
}

int tinyui_text_set_scroll_enabled(tinyui_obj_t *text_obj, int enabled)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

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

int tinyui_text_scroll_seek(tinyui_obj_t *text_obj, int offset)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

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

int tinyui_text_scroll_move(tinyui_obj_t *text_obj, int move_value)
{
    struct tinyui_text *text = tinyui_text_as_text(text_obj);
    if (text == 0) { return -1; }

    if (text == 0 || move_value < -128 || move_value > 127
        || text->widget.ld_widget == 0
        || text->widget.kind != TINYUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    ldTextScrollMove((ldText_t *)text->widget.ld_widget, (int8_t)move_value);
    return 0;
}
