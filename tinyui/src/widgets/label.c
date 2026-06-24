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
#include "widgets/label.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldLabel.h"

#include <stdlib.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

struct tinyui_image_source;

struct tinyui_label_create_ctx {
    const char *id;
};

/* ---- test seam state ---- */


static ldLabel_t *tinyui_label_backend(struct tinyui_label *label)
{
    if (label == 0 || label->widget.ld_widget == 0
        || label->widget.kind != TINYUI_BACKEND_WIDGET_LABEL) {
        return 0;
    }

    return (ldLabel_t *)label->widget.ld_widget;
}

static enum tinyui_align tinyui_label_unmap_align(arm_2d_align_t align)
{
    switch (align & (ARM_2D_ALIGN_LEFT | ARM_2D_ALIGN_RIGHT)) {
    case ARM_2D_ALIGN_LEFT:
        return TINYUI_ALIGN_START;
    case ARM_2D_ALIGN_RIGHT:
        return TINYUI_ALIGN_END;
    default:
        return TINYUI_ALIGN_CENTER;
    }
}

static int tinyui_label_props_are_valid(const struct tinyui_label_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0
        && (props->background_source == 0 || props->background_source->img_tile != 0);
}

static void *tinyui_label_ld_init(void *ctx,
                                  struct ld_scene_t *scene,
                                  uint16_t name_id,
                                  uint16_t parent_name_id)
{
    (void)ctx;
    return ldLabel_init(scene, NULL, name_id, parent_name_id, 0, 0, 220, 28, NULL);
}

/**
 * @brief Create label widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_label *tinyui_label_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_label *label;
    struct tinyui_label_create_ctx ctx = {.id = id};

    if (parent == 0 || id == 0) {
        return 0;
    }
    label = (struct tinyui_label *)tinyui_widget_create_leaf(&parent->widget,
                                                             TINYUI_BACKEND_WIDGET_LABEL,
                                                             tinyui_label_ld_init,
                                                             &ctx,
                                                             sizeof(*label));
    if (label == 0) {
        return 0;
    }
    label->id = id;

    return label;
}

/**
 * @brief Create label widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_label *tinyui_label_create_with_props(struct tinyui_window *parent,
                                                    const struct tinyui_label_props *props)
{
    struct tinyui_label *label;

    if (!tinyui_label_props_are_valid(props)) {
        return 0;
    }

    label = tinyui_label_create(parent, props->id);
    if (label == 0) {
        return 0;
    }

    if ((props->text != 0 && tinyui_label_set_text(label, props->text) != 0)
        || (props->font != 0 && tinyui_label_set_font(label, props->font) != 0)
        || (props->style_class != 0
            && tinyui_widget_set_style_class(&label->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&label->widget, props->user_data) != 0
        || tinyui_widget_set_border_color(&label->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&label->widget, props->radius) != 0
        || tinyui_widget_set_padding(&label->widget, props->padding) != 0
        || ((props->width > 0 || props->height > 0)
            && tinyui_widget_set_size(&label->widget, props->width, props->height) != 0)
        || tinyui_label_set_bg_color(label, props->bg_color) != 0
        || tinyui_label_set_text_color(label, props->text_color) != 0
        || tinyui_label_set_background_source(label, props->background_source) != 0
        || tinyui_label_set_transparent(label, props->transparent) != 0
        || tinyui_label_set_align(label, props->align) != 0) {
        tinyui_widget_destroy_common(&label->widget);
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

int tinyui_label_set_text(struct tinyui_label *label, const char *text)
{
    if (label == 0 || text == 0) {
        return -1;
    }

    if (tinyui_widget_set_text(&label->widget, text) != 0) {
        return -1;
    }
    return tinyui_widget_set_backend_text(&label->widget, text);
}

/**
 * @brief Get text of label widget
 *
 * @param[out] label Label widget instance
 */

const char *tinyui_label_get_text(struct tinyui_label *label)
{
    ldLabel_t *ld_label = tinyui_label_backend(label);

    if (ld_label == 0) {
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

int tinyui_label_set_font(struct tinyui_label *label, const struct tinyui_font *font)
{
    ldLabel_t *ld_label = tinyui_label_backend(label);

    if (ld_label == 0) {
        return -1;
    }

    label->widget.font = font;
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

int tinyui_label_set_text_color(struct tinyui_label *label, unsigned int rgb)
{
    ldLabel_t *ld_label;

    if (label == 0 || tinyui_widget_set_text_color(&label->widget, rgb) != 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }

    ldLabelSetTextColor(ld_label, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Get text color of label widget
 *
 * @param[out] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_label_get_text_color(struct tinyui_label *label, unsigned int *rgb)
{
    ldLabel_t *ld_label;

    if (label == 0 || rgb == 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }

    *rgb = tinyui_ld_color_to_rgb((unsigned int)ldLabelGetTextColor(ld_label));
    return 0;
}

/**
 * @brief Set bg color of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_label_set_bg_color(struct tinyui_label *label, unsigned int rgb)
{
    ldLabel_t *ld_label;

    if (label == 0 || tinyui_widget_set_bg_color(&label->widget, rgb) != 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }

    ldLabelSetBackgroundColor(ld_label, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Get bg color of label widget
 *
 * @param[out] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_label_get_bg_color(struct tinyui_label *label, unsigned int *rgb)
{
    ldLabel_t *ld_label;

    if (label == 0 || rgb == 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }

    *rgb = tinyui_ld_color_to_rgb((unsigned int)ldLabelGetBackgroundColor(ld_label));
    return 0;
}

/**
 * @brief Set transparent of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] transparent transparent
 * @return -1 on failure
 */

int tinyui_label_set_transparent(struct tinyui_label *label, int transparent)
{
    ldLabel_t *ld_label = tinyui_label_backend(label);

    if (ld_label == 0) {
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

int tinyui_label_get_transparent(struct tinyui_label *label, int *transparent)
{
    ldLabel_t *ld_label;

    if (label == 0 || transparent == 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
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

int tinyui_label_set_align(struct tinyui_label *label, enum tinyui_align align)
{
    ldLabel_t *ld_label = tinyui_label_backend(label);

    if (ld_label == 0) {
        return -1;
    }
    if (align != TINYUI_ALIGN_START && align != TINYUI_ALIGN_CENTER && align != TINYUI_ALIGN_END) {
        return -1;
    }

    ldLabelSetAlign(ld_label, (arm_2d_align_t)tinyui_align_to_arm2d(align));
    return 0;
}

/**
 * @brief Get align of label widget
 *
 * @param[out] label Label widget instance
 * @param[out] align align
 * @return -1 on failure
 */

int tinyui_label_get_align(struct tinyui_label *label, enum tinyui_align *align)
{
    ldLabel_t *ld_label;

    if (label == 0 || align == 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
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

int tinyui_label_set_background_source(struct tinyui_label *label,
                                       struct tinyui_image_source *source)
{
    ldLabel_t *ld_label;

    if (label == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }

    ldLabelSetBackgroundImage(ld_label,
                              source != NULL ? source->img_tile : NULL,
                              source != NULL ? source->mask_tile : NULL);
    return 0;
}
