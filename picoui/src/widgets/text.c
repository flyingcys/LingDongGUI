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
#include "picoui/text.h"

#include <stdlib.h>

int picoui_native_text_set_text(struct picoui_text *text, const char *value);

static int picoui_text_props_are_valid(const struct picoui_text_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

/**
 * @brief Create text widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_text *picoui_text_create(struct picoui_window *parent, const char *id)
{
    struct picoui_text *text;

    if (parent == 0 || id == 0) {
        return 0;
    }

    text = calloc(1, sizeof(*text));
    if (text == 0) {
        return 0;
    }

    text->widget.backend_widget = picoui_backend_create_text(parent->widget.backend_widget, id);
    if (text->widget.backend_widget == 0) {
        free(text);
        return 0;
    }
    if (picoui_backend_widget_bind_host(text->widget.backend_widget, &text->widget) != 0) {
        free(text);
        return 0;
    }

    text->id = id;
    text->widget.visible = 1;
    text->widget.enabled = 1;
    return text;
}

/**
 * @brief Create text widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_text *picoui_text_create_with_props(struct picoui_window *parent,
                                                  const struct picoui_text_props *props)
{
    struct picoui_text *text;

    if (!picoui_text_props_are_valid(props)) {
        return 0;
    }

    text = picoui_text_create(parent, props->id);
    if (text == 0) {
        return 0;
    }

    if (props->text != 0 && picoui_text_set_text(text, props->text) != 0) {
        free(text);
        return 0;
    }
    if (props->font != 0 && picoui_text_set_font(text, props->font) != 0) {
        free(text);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&text->widget, props->style_class) != 0) {
        free(text);
        return 0;
    }
    if (picoui_widget_set_user_data(&text->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&text->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&text->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&text->widget, props->border_color) != 0
        || picoui_widget_set_radius(&text->widget, props->radius) != 0
        || picoui_widget_set_padding(&text->widget, props->padding) != 0) {
        free(text);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&text->widget, props->width, props->height) != 0) {
        free(text);
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

int picoui_text_set_text(struct picoui_text *text, const char *value)
{
    struct picoui_backend_widget *backend;
    const char *old_widget_text;
    const char *old_backend_text;

    if (text == 0 || value == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }

    old_widget_text = text->widget.text;
    old_backend_text = backend->text;

    if (picoui_widget_set_text(&text->widget, value) != 0) {
        return -1;
    }
    if (picoui_backend_set_text(text->widget.backend_widget, value) != 0) {
        text->widget.text = old_widget_text;
        return -1;
    }
    if (picoui_native_text_set_text(text, value) != 0) {
        text->widget.text = old_widget_text;
        backend->text = old_backend_text;
        return -1;
    }
    return 0;
}

/**
 * @brief Set static text of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_static_text(struct picoui_text *text, const char *value)
{
    struct picoui_backend_widget *backend;
    const char *old_widget_text;
    const char *old_backend_text;

    if (text == 0 || value == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }

    old_widget_text = text->widget.text;
    old_backend_text = backend->text;

    if (picoui_backend_text_set_static_text(text->widget.backend_widget, value) != 0) {
        return -1;
    }
    text->widget.text = value;
    if (picoui_native_text_set_text(text, value) != 0) {
        text->widget.text = old_widget_text;
        backend->text = old_backend_text;
        return -1;
    }
    return 0;
}

const char *picoui_text_get_text(const struct picoui_text *text)
{
    if (text == 0) {
        return 0;
    }

    return text->widget.text;
}

/**
 * @brief Set font of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_font(struct picoui_text *text, const struct picoui_font *font)
{
    if (text == 0) {
        return -1;
    }

    if (picoui_backend_widget_set_font(text->widget.backend_widget, font) != 0) {
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

int picoui_text_set_transparent(struct picoui_text *text, int transparent)
{
    if (text == 0) {
        return -1;
    }

    return picoui_backend_text_set_transparent(text->widget.backend_widget, transparent);
}

/**
 * @brief Set text color of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_text_color(struct picoui_text *text, unsigned int rgb)
{
    if (text == 0) {
        return -1;
    }

    if (picoui_backend_text_set_text_color(text->widget.backend_widget, rgb) != 0) {
        return -1;
    }
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

int picoui_text_set_bg_color(struct picoui_text *text, unsigned int rgb)
{
    if (text == 0) {
        return -1;
    }

    if (picoui_backend_text_set_bg_color(text->widget.backend_widget, rgb) != 0) {
        return -1;
    }
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

int picoui_text_set_background_source(struct picoui_text *text,
                                      struct picoui_image_source *source)
{
    if (text == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    return picoui_backend_text_set_background_source(text->widget.backend_widget, source);
}

/**
 * @brief Set consumed font of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_consumed_font(struct picoui_text *text, const struct picoui_font *font)
{
    return picoui_text_set_font(text, font);
}

/**
 * @brief text scroll seek
 *
 * @param[in] text Text widget instance
 * @param[in] offset Offset
 * @return -1 on failure
 */

int picoui_text_scroll_seek(struct picoui_text *text, int offset)
{
    if (text == 0) {
        return -1;
    }

    return picoui_backend_text_scroll_seek(text->widget.backend_widget, offset);
}

/**
 * @brief text scroll move
 *
 * @param[in] text Text widget instance
 * @param[in] move_value move value
 * @return -1 on failure
 */

int picoui_text_scroll_move(struct picoui_text *text, int move_value)
{
    if (text == 0) {
        return -1;
    }

    return picoui_backend_text_scroll_move(text->widget.backend_widget, move_value);
}
