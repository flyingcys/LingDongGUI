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
#include "picoui/label.h"

#include <stdlib.h>

struct picoui_image_source;

int picoui_backend_label_set_text_color(struct picoui_label *label, unsigned int rgb);
int picoui_backend_label_get_text_color(struct picoui_label *label, unsigned int *rgb);
int picoui_backend_label_set_bg_color(struct picoui_label *label, unsigned int rgb);
int picoui_backend_label_get_bg_color(struct picoui_label *label, unsigned int *rgb);
int picoui_backend_label_set_transparent(struct picoui_label *label, int transparent);
int picoui_backend_label_get_transparent(struct picoui_label *label, int *transparent);
int picoui_backend_label_set_align(struct picoui_label *label, enum picoui_align align);
int picoui_backend_label_get_align(struct picoui_label *label, enum picoui_align *align);
int picoui_backend_label_set_background_source(struct picoui_label *label,
                                               struct picoui_image_source *source);
const char *picoui_backend_label_get_text(struct picoui_label *label);

static void picoui_label_destroy_partial(struct picoui_label *label)
{
    if (label == 0) {
        return;
    }

    if (label->widget.backend_widget != 0) {
        (void)picoui_backend_widget_unbind_host(label->widget.backend_widget);
        (void)picoui_backend_widget_detach_from_parent(label->widget.backend_widget);
        free(label->widget.backend_widget);
        label->widget.backend_widget = 0;
    }

    free(label);
}

static int picoui_label_props_are_valid(const struct picoui_label_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0
        && (props->background_source == 0 || props->background_source->img_tile != 0);
}

/**
 * @brief Create label widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id)
{
    struct picoui_label *label;

    if (parent == 0 || id == 0) {
        return 0;
    }

    label = calloc(1, sizeof(*label));
    if (label == 0) {
        return 0;
    }

    label->widget.backend_widget = picoui_backend_create_label(parent->widget.backend_widget, id);
    if (label->widget.backend_widget == 0) {
        free(label);
        return 0;
    }

    label->id = id;
    label->widget.visible = 1;
    label->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(label->widget.backend_widget, &label->widget) != 0) {
        picoui_label_destroy_partial(label);
        return 0;
    }
    return label;
}

/**
 * @brief Create label widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_label *picoui_label_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_label_props *props)
{
    struct picoui_label *label;

    if (!picoui_label_props_are_valid(props)) {
        return 0;
    }

    label = picoui_label_create(parent, props->id);
    if (label == 0) {
        return 0;
    }

    if (props->text != 0 && picoui_label_set_text(label, props->text) != 0) {
        picoui_label_destroy_partial(label);
        return 0;
    }
    if (props->font != 0 && picoui_label_set_font(label, props->font) != 0) {
        picoui_label_destroy_partial(label);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&label->widget, props->style_class) != 0) {
        picoui_label_destroy_partial(label);
        return 0;
    }
    if (picoui_widget_set_user_data(&label->widget, props->user_data) != 0
        || picoui_widget_set_border_color(&label->widget, props->border_color) != 0
        || picoui_widget_set_radius(&label->widget, props->radius) != 0
        || picoui_widget_set_padding(&label->widget, props->padding) != 0) {
        picoui_label_destroy_partial(label);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&label->widget, props->width, props->height) != 0) {
        picoui_label_destroy_partial(label);
        return 0;
    }
    if (picoui_label_set_bg_color(label, props->bg_color) != 0
        || picoui_label_set_text_color(label, props->text_color) != 0
        || picoui_label_set_background_source(label, props->background_source) != 0
        || picoui_label_set_transparent(label, props->transparent) != 0
        || picoui_label_set_align(label, props->align) != 0) {
        picoui_label_destroy_partial(label);
        return 0;
    }

    return label;
}

/**
 * @brief Set text of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] text Text widget instance
 * @return -1 on failure
 */

int picoui_label_set_text(struct picoui_label *label, const char *text)
{
    if (label == 0 || text == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&label->widget, text) != 0) {
        return -1;
    }
    return picoui_backend_set_text(label->widget.backend_widget, text);
}

/**
 * @brief Get text of label widget
 *
 * @param[out] label Label widget instance
 */

const char *picoui_label_get_text(struct picoui_label *label)
{
    if (label == 0) {
        return 0;
    }

    return label->widget.text;
}

/**
 * @brief Set font of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] font font
 * @return -1 on failure
 */

int picoui_label_set_font(struct picoui_label *label, const struct picoui_font *font)
{
    if (label == 0) {
        return -1;
    }

    label->widget.font = font;
    return picoui_backend_widget_set_font(label->widget.backend_widget, font);
}

/**
 * @brief Set text color of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_label_set_text_color(struct picoui_label *label, unsigned int rgb)
{
    if (label == 0 || picoui_widget_set_text_color(&label->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_label_set_text_color(label, rgb);
}

/**
 * @brief Get text color of label widget
 *
 * @param[out] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_label_get_text_color(struct picoui_label *label, unsigned int *rgb)
{
    if (label == 0 || rgb == 0) {
        return -1;
    }

    return picoui_backend_label_get_text_color(label, rgb);
}

/**
 * @brief Set bg color of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_label_set_bg_color(struct picoui_label *label, unsigned int rgb)
{
    if (label == 0 || picoui_widget_set_bg_color(&label->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_label_set_bg_color(label, rgb);
}

/**
 * @brief Get bg color of label widget
 *
 * @param[out] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_label_get_bg_color(struct picoui_label *label, unsigned int *rgb)
{
    if (label == 0 || rgb == 0) {
        return -1;
    }

    return picoui_backend_label_get_bg_color(label, rgb);
}

/**
 * @brief Set transparent of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] transparent transparent
 * @return -1 on failure
 */

int picoui_label_set_transparent(struct picoui_label *label, int transparent)
{
    if (label == 0) {
        return -1;
    }

    return picoui_backend_label_set_transparent(label, transparent != 0);
}

/**
 * @brief Get transparent of label widget
 *
 * @param[out] label Label widget instance
 * @param[in] transparent transparent
 * @return -1 on failure
 */

int picoui_label_get_transparent(struct picoui_label *label, int *transparent)
{
    if (label == 0 || transparent == 0) {
        return -1;
    }

    return picoui_backend_label_get_transparent(label, transparent);
}

/**
 * @brief Set align of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] align align
 * @return -1 on failure
 */

int picoui_label_set_align(struct picoui_label *label, enum picoui_align align)
{
    if (label == 0) {
        return -1;
    }

    return picoui_backend_label_set_align(label, align);
}

/**
 * @brief Get align of label widget
 *
 * @param[out] label Label widget instance
 * @param[out] align align
 * @return -1 on failure
 */

int picoui_label_get_align(struct picoui_label *label, enum picoui_align *align)
{
    if (label == 0 || align == 0) {
        return -1;
    }

    return picoui_backend_label_get_align(label, align);
}

/**
 * @brief Set background source of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int picoui_label_set_background_source(struct picoui_label *label,
                                       struct picoui_image_source *source)
{
    if (label == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    return picoui_backend_label_set_background_source(label, source);
}
