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
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldLabel.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

struct picoui_image_source;
int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static ldColor tinyui_label_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static unsigned int tinyui_label_ld_color_to_rgb(ldColor color)
{
    uint32_t red = ((uint32_t)color >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)color >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)color & 0x1FU;

    red = (red << 3) | (red >> 2);
    green = (green << 2) | (green >> 4);
    blue = (blue << 3) | (blue >> 2);
    return (unsigned int)((red << 16) | (green << 8) | blue);
}

static arm_2d_align_t tinyui_label_map_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_START:
        return ARM_2D_ALIGN_LEFT;
    case PICOUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case PICOUI_ALIGN_CENTER:
        return ARM_2D_ALIGN_CENTRE;
    default:
        return ARM_2D_ALIGN_CENTRE;
    }
}

static enum picoui_align tinyui_label_unmap_align(arm_2d_align_t align)
{
    switch (align & (ARM_2D_ALIGN_LEFT | ARM_2D_ALIGN_RIGHT)) {
    case ARM_2D_ALIGN_LEFT:
        return PICOUI_ALIGN_START;
    case ARM_2D_ALIGN_RIGHT:
        return PICOUI_ALIGN_END;
    default:
        return PICOUI_ALIGN_CENTER;
    }
}

static ldLabel_t *tinyui_label_get_ld(struct picoui_label *label)
{
    struct picoui_backend_widget *backend;

    if (label == NULL || label->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_LABEL || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldLabel_t *)backend->ld_widget;
}

static int tinyui_label_props_are_valid(const struct picoui_label_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0
        && (props->background_source == 0 || props->background_source->img_tile != 0);
}

static void tinyui_label_dispose_partial(struct picoui_label *label)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;

    if (label == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    if (backend != 0) {
        app_state = tinyui_runtime_bridge_backend_state(backend->owner);
        if (backend->parent != 0) {
            (void)tinyui_runtime_bridge_detach_from_parent(backend);
        }
        (void)tinyui_runtime_bridge_unbind_host(backend);
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldLabel_depose(app_state->ld_scene, (ldLabel_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(label);
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
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldLabel_t *ld_label;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->widget.backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    label = calloc(1, sizeof(*label));
    if (label == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(label);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(label);
        return 0;
    }

    ld_label = ldLabel_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_backend->ld_name_id,
                            0,
                            0,
                            220,
                            28,
                            NULL);
    if (ld_label == 0) {
        free(backend);
        free(label);
        return 0;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_LABEL,
                                         id,
                                         parent_backend->theme) != 0) {
        ldLabel_depose(app_state->ld_scene, ld_label);
        free(backend);
        free(label);
        return 0;
    }
    backend->ld_widget = ld_label;
    backend->ld_name_id = name_id;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldLabel_depose(app_state->ld_scene, ld_label);
        free(backend);
        free(label);
        return 0;
    }

    label->id = id;
    label->widget.backend_widget = backend;
    label->widget.visible = 1;
    label->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(label->widget.backend_widget, &label->widget) != 0) {
        free(label);
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

    if (!tinyui_label_props_are_valid(props)) {
        return 0;
    }

    label = picoui_label_create(parent, props->id);
    if (label == 0) {
        return 0;
    }

    if (props->text != 0 && picoui_label_set_text(label, props->text) != 0) {
        tinyui_label_dispose_partial(label);
        return 0;
    }
    if (props->font != 0 && picoui_label_set_font(label, props->font) != 0) {
        tinyui_label_dispose_partial(label);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&label->widget, props->style_class) != 0) {
        tinyui_label_dispose_partial(label);
        return 0;
    }
    if (picoui_widget_set_user_data(&label->widget, props->user_data) != 0
        || picoui_widget_set_border_color(&label->widget, props->border_color) != 0
        || picoui_widget_set_radius(&label->widget, props->radius) != 0
        || picoui_widget_set_padding(&label->widget, props->padding) != 0) {
        tinyui_label_dispose_partial(label);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&label->widget, props->width, props->height) != 0) {
        tinyui_label_dispose_partial(label);
        return 0;
    }
    if (picoui_label_set_bg_color(label, props->bg_color) != 0
        || picoui_label_set_text_color(label, props->text_color) != 0
        || picoui_label_set_background_source(label, props->background_source) != 0
        || picoui_label_set_transparent(label, props->transparent) != 0
        || picoui_label_set_align(label, props->align) != 0) {
        tinyui_label_dispose_partial(label);
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
    return tinyui_widget_set_backend_text(label->widget.backend_widget, text);
}

/**
 * @brief Get text of label widget
 *
 * @param[out] label Label widget instance
 */

const char *picoui_label_get_text(struct picoui_label *label)
{
    ldLabel_t *ld_label;

    if (label == 0 || label->widget.backend_widget == 0) {
        return 0;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return 0;
    }

    return (const char *)ldLabelGetText(ld_label);
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
    ldLabel_t *ld_label;

    if (label == 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    label->widget.font = font;
    ((struct picoui_backend_widget *)label->widget.backend_widget)->font = font;
    if (font != NULL) {
        ldLabelSetFont(ld_label, (arm_2d_font_t *)font);
    } else {
        ldLabelSetFont(ld_label, (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    }
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0 || picoui_widget_set_text_color(&label->widget, rgb) != 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    ldLabelSetTextColor(ld_label, tinyui_label_rgb_to_ld_color(rgb));
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0 || rgb == 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    *rgb = tinyui_label_ld_color_to_rgb(ldLabelGetTextColor(ld_label));
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0 || picoui_widget_set_bg_color(&label->widget, rgb) != 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    ldLabelSetBackgroundColor(ld_label, tinyui_label_rgb_to_ld_color(rgb));
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0 || rgb == 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    *rgb = tinyui_label_ld_color_to_rgb(ldLabelGetBackgroundColor(ld_label));
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    ldLabelSetTransparent(ld_label, transparent != 0);
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0 || transparent == 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    *transparent = ldLabelGetTransparent(ld_label) ? 1 : 0;
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }
    if (align != PICOUI_ALIGN_START && align != PICOUI_ALIGN_CENTER && align != PICOUI_ALIGN_END) {
        return -1;
    }

    ldLabelSetAlign(ld_label, tinyui_label_map_align(align));
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0 || align == 0) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    *align = tinyui_label_unmap_align(ldLabelGetAlign(ld_label));
    return 0;
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
    ldLabel_t *ld_label;

    if (label == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_label = tinyui_label_get_ld(label);
    if (ld_label == NULL) {
        return -1;
    }

    ldLabelSetBackgroundImage(ld_label,
                              source != NULL ? source->img_tile : NULL,
                              source != NULL ? source->mask_tile : NULL);
    return 0;
}
