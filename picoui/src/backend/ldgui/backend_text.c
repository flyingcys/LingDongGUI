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

#include "backend.h"
#include "internal.h"
#include "ldBase.h"
#include "ldText.h"
#include "picoui/widget.h"

#include <stdlib.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_a1_font_t ARM_2D_FONT_16x24;

struct picoui_backend_text_box_prefix_view {
    text_box_cfg_t tCFG;
};

static int picoui_backend_text_fail_next_set_font = 0;

static ldColor picoui_backend_text_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static ldText_t *picoui_backend_text_get_ld_text(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->kind != PICOUI_BACKEND_WIDGET_TEXT || widget->ld_widget == NULL) {
        return NULL;
    }
    return (ldText_t *)widget->ld_widget;
}

static struct picoui_backend_app_state *picoui_backend_text_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static int picoui_backend_text_apply_consumed_font(ldText_t *ld_text, arm_2d_font_t *font)
{
    if (ld_text == NULL || font == NULL) {
        return -1;
    }

    return ldTextSetConsumedFont(ld_text, font);
}

static arm_2d_font_t *picoui_backend_text_default_font(void)
{
    return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
}

static arm_2d_font_t *picoui_backend_text_resolve_font(const struct picoui_font *font)
{
    if (font != NULL && font->kind == PICOUI_FONT_KIND_VRES && font->vres_addr != 0) {
        return (arm_2d_font_t *)ldBaseGetVresFont(font->vres_addr);
    }

    if (font == NULL || font->family == NULL || font->size <= 0) {
        return picoui_backend_text_default_font();
    }

    if (strcmp(font->family, "Sans") == 0 && font->size >= 20) {
        return (arm_2d_font_t *)&ARM_2D_FONT_16x24;
    }

    return picoui_backend_text_default_font();
}

/**
 * @brief text: test fail next set font
 *
 */

void picoui_backend_text_test_fail_next_set_font(void)
{
    picoui_backend_text_fail_next_set_font = 1;
}

/**
 * @brief Set font of text backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_backend_text_set_font(void *backend_widget, const void *font)
{
    struct picoui_backend_widget *widget = backend_widget;
    const struct picoui_font *picoui_font = (const struct picoui_font *)font;
    ldText_t *ld_text;
    arm_2d_font_t *resolved_font;

    if (widget == NULL || widget->kind != PICOUI_BACKEND_WIDGET_TEXT || widget->ld_widget == NULL) {
        return -1;
    }

    ld_text = (ldText_t *)widget->ld_widget;

    if (picoui_backend_text_fail_next_set_font != 0) {
        picoui_backend_text_fail_next_set_font = 0;
        return -1;
    }

    resolved_font = picoui_backend_text_resolve_font(picoui_font);
    if (resolved_font == NULL) {
        return -1;
    }

    if (font != NULL && ldTextSetFont(ld_text, resolved_font) != 0) {
        return -1;
    }

    if (font == NULL && picoui_backend_text_apply_consumed_font(ld_text, resolved_font) != 0) {
        return -1;
    }

    widget->font = font;
    return 0;
}

/**
 * @brief Set static text of text backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_text_set_static_text(void *backend_widget, const char *text)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldText_t *ld_text = picoui_backend_text_get_ld_text(backend_widget);

    if (ld_text == NULL || text == NULL) {
        return -1;
    }

    ldTextSetStaticText(ld_text, (const uint8_t *)text);
    widget->text = text;
    return 0;
}

/**
 * @brief Set transparent of text backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_backend_text_set_transparent(void *backend_widget, int transparent)
{
    ldText_t *ld_text = picoui_backend_text_get_ld_text(backend_widget);

    if (ld_text == NULL) {
        return -1;
    }

    ldTextSetTransparent(ld_text, transparent != 0);
    return 0;
}

/**
 * @brief Set text color of text backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_text_set_text_color(void *backend_widget, unsigned int rgb)
{
    ldText_t *ld_text = picoui_backend_text_get_ld_text(backend_widget);

    if (ld_text == NULL) {
        return -1;
    }

    ldTextSetTextColor(ld_text, picoui_backend_text_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set bg color of text backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_text_set_bg_color(void *backend_widget, unsigned int rgb)
{
    ldText_t *ld_text = picoui_backend_text_get_ld_text(backend_widget);

    if (ld_text == NULL) {
        return -1;
    }

    ldTextSetBackgroundColor(ld_text, picoui_backend_text_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set background source of text backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_text_set_background_source(void *backend_widget,
                                              struct picoui_image_source *source)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldText_t *ld_text = picoui_backend_text_get_ld_text(backend_widget);

    if (ld_text == NULL || (source != NULL && source->img_tile == NULL)) {
        return -1;
    }

    ldTextSetBackgroundImage(ld_text,
                             source != NULL ? source->img_tile : NULL,
                             source != NULL ? source->mask_tile : NULL);
    widget->image_source = source;
    return 0;
}

/**
 * @brief text: scroll seek
 *
 * @param[in] backend_widget backend widget
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int picoui_backend_text_scroll_seek(void *backend_widget, int offset)
{
    ldText_t *ld_text = picoui_backend_text_get_ld_text(backend_widget);

    if (ld_text == NULL) {
        return -1;
    }

    ldTextScrollSeek(ld_text, (int16_t)offset);
    return 0;
}

/**
 * @brief text: scroll move
 *
 * @param[in] backend_widget backend widget
 * @param[in] move_value move value
 * @return 0 on success, -1 on failure
 */

int picoui_backend_text_scroll_move(void *backend_widget, int move_value)
{
    ldText_t *ld_text = picoui_backend_text_get_ld_text(backend_widget);

    if (ld_text == NULL || move_value < -128 || move_value > 127) {
        return -1;
    }

    ldTextScrollMove(ld_text, (int8_t)move_value);
    return 0;
}

/**
 * @brief Create backend for text
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_text(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldText_t *ld_text;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_text_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_text = ldText_init(app_state->ld_scene,
                          NULL,
                          name_id,
                          parent_widget->ld_name_id,
                          0,
                          0,
                          220,
                          48,
                          NULL,
                          TEXT_BOX_LINE_ALIGN_LEFT,
                          false);
    if (ld_text == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_TEXT;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_text;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}
