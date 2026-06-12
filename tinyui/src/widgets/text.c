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
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldText.h"

#include <stdlib.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_a1_font_t ARM_2D_FONT_16x24;

static int tinyui_text_fail_next_set_font = 0;

static ldColor tinyui_text_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static ldText_t *tinyui_text_get_ld_text(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->kind != PICOUI_BACKEND_WIDGET_TEXT || widget->ld_widget == NULL) {
        return NULL;
    }
    return (ldText_t *)widget->ld_widget;
}

static int tinyui_text_apply_consumed_font(ldText_t *ld_text, arm_2d_font_t *font)
{
    if (ld_text == NULL || font == NULL) {
        return -1;
    }

    return ldTextSetConsumedFont(ld_text, font);
}

static arm_2d_font_t *tinyui_text_default_font(void)
{
    return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
}

static arm_2d_font_t *tinyui_text_resolve_font(const struct picoui_font *font)
{
    if (font != NULL && font->kind == PICOUI_FONT_KIND_VRES && font->vres_addr != 0) {
        return (arm_2d_font_t *)ldBaseGetVresFont(font->vres_addr);
    }

    if (font == NULL || font->family == NULL || font->size <= 0) {
        return tinyui_text_default_font();
    }

    if (strcmp(font->family, "Sans") == 0 && font->size >= 20) {
        return (arm_2d_font_t *)&ARM_2D_FONT_16x24;
    }

    return tinyui_text_default_font();
}

static void tinyui_text_dispose_partial(struct picoui_text *text)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;

    if (text == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    if (backend != 0) {
        app_state = tinyui_runtime_bridge_backend_state(backend->owner);
        if (backend->parent != 0) {
            (void)tinyui_runtime_bridge_detach_from_parent(backend);
        }
        (void)tinyui_runtime_bridge_unbind_host(backend);
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldText_depose(app_state->ld_scene, (ldText_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(text);
}

void tinyui_text_test_fail_next_set_font(void)
{
    tinyui_text_fail_next_set_font = 1;
}

int tinyui_text_set_font(void *backend_widget, const void *font)
{
    struct picoui_backend_widget *widget = backend_widget;
    const struct picoui_font *picoui_font = (const struct picoui_font *)font;
    ldText_t *ld_text;
    arm_2d_font_t *resolved_font;

    if (widget == NULL || widget->kind != PICOUI_BACKEND_WIDGET_TEXT || widget->ld_widget == NULL) {
        return -1;
    }

    ld_text = (ldText_t *)widget->ld_widget;

    if (tinyui_text_fail_next_set_font != 0) {
        tinyui_text_fail_next_set_font = 0;
        return -1;
    }

    resolved_font = tinyui_text_resolve_font(picoui_font);
    if (resolved_font == NULL) {
        return -1;
    }

    if (font != NULL && ldTextSetFont(ld_text, resolved_font) != 0) {
        return -1;
    }

    if (font == NULL && tinyui_text_apply_consumed_font(ld_text, resolved_font) != 0) {
        return -1;
    }

    widget->font = font;
    return 0;
}

int tinyui_text_set_static_text(void *backend_widget, const char *text)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldText_t *ld_text = tinyui_text_get_ld_text(backend_widget);

    if (ld_text == NULL || text == NULL) {
        return -1;
    }

    ldTextSetStaticText(ld_text, (const uint8_t *)text);
    widget->text = text;
    return 0;
}

int tinyui_text_set_transparent(void *backend_widget, int transparent)
{
    ldText_t *ld_text = tinyui_text_get_ld_text(backend_widget);

    if (ld_text == NULL) {
        return -1;
    }

    ldTextSetTransparent(ld_text, transparent != 0);
    return 0;
}

int tinyui_text_set_text_color(void *backend_widget, unsigned int rgb)
{
    ldText_t *ld_text = tinyui_text_get_ld_text(backend_widget);

    if (ld_text == NULL) {
        return -1;
    }

    ldTextSetTextColor(ld_text, tinyui_text_rgb_to_ld_color(rgb));
    return 0;
}

int tinyui_text_set_bg_color(void *backend_widget, unsigned int rgb)
{
    ldText_t *ld_text = tinyui_text_get_ld_text(backend_widget);

    if (ld_text == NULL) {
        return -1;
    }

    ldTextSetBackgroundColor(ld_text, tinyui_text_rgb_to_ld_color(rgb));
    return 0;
}

int tinyui_text_set_background_source(void *backend_widget,
                                      struct picoui_image_source *source)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldText_t *ld_text = tinyui_text_get_ld_text(backend_widget);

    if (ld_text == NULL || (source != NULL && source->img_tile == NULL)) {
        return -1;
    }

    ldTextSetBackgroundImage(ld_text,
                             source != NULL ? source->img_tile : NULL,
                             source != NULL ? source->mask_tile : NULL);
    widget->image_source = source;
    return 0;
}

int tinyui_text_scroll_seek(void *backend_widget, int offset)
{
    ldText_t *ld_text = tinyui_text_get_ld_text(backend_widget);

    if (ld_text == NULL) {
        return -1;
    }

    ldTextScrollSeek(ld_text, (int16_t)offset);
    return 0;
}

int tinyui_text_scroll_move(void *backend_widget, int move_value)
{
    ldText_t *ld_text = tinyui_text_get_ld_text(backend_widget);

    if (ld_text == NULL || move_value < -128 || move_value > 127) {
        return -1;
    }

    ldTextScrollMove(ld_text, (int8_t)move_value);
    return 0;
}

static int tinyui_text_props_are_valid(const struct picoui_text_props *props)
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
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldText_t *ld_text;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->widget.backend_widget;
    app_state = parent_backend != 0
        ? tinyui_runtime_bridge_backend_state_from_parent(parent_backend)
        : 0;
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    text = calloc(1, sizeof(*text));
    if (text == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(text);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(text);
        return 0;
    }

    ld_text = ldText_init(app_state->ld_scene,
                          NULL,
                          name_id,
                          parent_backend->ld_name_id,
                          0,
                          0,
                          220,
                          48,
                          NULL,
                          TEXT_BOX_LINE_ALIGN_LEFT,
                          false);
    if (ld_text == 0) {
        free(backend);
        free(text);
        return 0;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_TEXT,
                                         id,
                                         parent_backend->theme) != 0) {
        ldText_depose(app_state->ld_scene, ld_text);
        free(backend);
        free(text);
        return 0;
    }
    backend->ld_widget = ld_text;
    backend->ld_name_id = name_id;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldText_depose(app_state->ld_scene, ld_text);
        free(backend);
        free(text);
        return 0;
    }

    text->widget.backend_widget = backend;
    text->id = id;
    text->widget.visible = 1;
    text->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(text->widget.backend_widget, &text->widget) != 0) {
        (void)tinyui_runtime_bridge_detach_from_parent(text->widget.backend_widget);
        ldText_depose(app_state->ld_scene, ld_text);
        free(backend);
        free(text);
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

struct picoui_text *picoui_text_create_with_props(struct picoui_window *parent,
                                                  const struct picoui_text_props *props)
{
    struct picoui_text *text;

    if (!tinyui_text_props_are_valid(props)) {
        return 0;
    }

    text = picoui_text_create(parent, props->id);
    if (text == 0) {
        return 0;
    }

    if (props->text != 0 && picoui_text_set_text(text, props->text) != 0) {
        tinyui_text_dispose_partial(text);
        return 0;
    }
    if (props->font != 0 && picoui_text_set_font(text, props->font) != 0) {
        tinyui_text_dispose_partial(text);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&text->widget, props->style_class) != 0) {
        tinyui_text_dispose_partial(text);
        return 0;
    }
    if (picoui_widget_set_user_data(&text->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&text->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&text->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&text->widget, props->border_color) != 0
        || picoui_widget_set_radius(&text->widget, props->radius) != 0
        || picoui_widget_set_padding(&text->widget, props->padding) != 0) {
        tinyui_text_dispose_partial(text);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&text->widget, props->width, props->height) != 0) {
        tinyui_text_dispose_partial(text);
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
    if (text == 0 || value == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&text->widget, value) != 0) {
        return -1;
    }
    return tinyui_widget_set_backend_text(text->widget.backend_widget, value);
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
    if (text == 0 || value == 0) {
        return -1;
    }

    if (tinyui_text_set_static_text(text->widget.backend_widget, value) != 0) {
        return -1;
    }
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

int picoui_text_set_font(struct picoui_text *text, const struct picoui_font *font)
{
    if (text == 0) {
        return -1;
    }

    if (tinyui_text_set_font(text->widget.backend_widget, font) != 0) {
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

    return tinyui_text_set_transparent(text->widget.backend_widget, transparent);
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

    if (tinyui_text_set_text_color(text->widget.backend_widget, rgb) != 0) {
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

    if (tinyui_text_set_bg_color(text->widget.backend_widget, rgb) != 0) {
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

    return tinyui_text_set_background_source(text->widget.backend_widget, source);
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

    return tinyui_text_scroll_seek(text->widget.backend_widget, offset);
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

    return tinyui_text_scroll_move(text->widget.backend_widget, move_value);
}
