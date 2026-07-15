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


static struct tinyui_label *tinyui_label_as_label(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_LABEL)) {
        return 0;
    }
    return (struct tinyui_label *)w;
}

static const struct tinyui_label *tinyui_label_as_label_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_LABEL)) {
        return 0;
    }
    return (const struct tinyui_label *)w;
}

struct tinyui_image_source;

struct tinyui_label_create_ctx {
    const char *id;
};

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

static int tinyui_label_align_is_valid(enum tinyui_align align)
{
    return align == TINYUI_ALIGN_START || align == TINYUI_ALIGN_CENTER || align == TINYUI_ALIGN_END;
}

static arm_2d_align_t tinyui_label_map_text_align(enum tinyui_align x_align,
                                                  enum tinyui_align y_align)
{
    int align = tinyui_align_to_arm2d(x_align);

    switch (y_align) {
    case TINYUI_ALIGN_START:
        align |= ARM_2D_ALIGN_TOP;
        break;
    case TINYUI_ALIGN_END:
        align |= ARM_2D_ALIGN_BOTTOM;
        break;
    case TINYUI_ALIGN_CENTER:
    default:
        align |= ARM_2D_ALIGN_TOP | ARM_2D_ALIGN_BOTTOM;
        break;
    }

    return (arm_2d_align_t)align;
}

static int tinyui_label_props_are_valid(const tinyui_label_props_t *props)
{
    if (props == 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LABEL_FIELD_WIDTH) != 0 && props->width < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LABEL_FIELD_HEIGHT) != 0 && props->height < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LABEL_FIELD_RADIUS) != 0 && props->radius < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LABEL_FIELD_PADDING) != 0 && props->padding < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LABEL_FIELD_ALIGN) != 0
        && !tinyui_label_align_is_valid(props->align)) {
        return 0;
    }
    if ((props->fields & TINYUI_LABEL_FIELD_BACKGROUND_SOURCE) != 0
        && props->background_source != 0
        && tinyui_image_source_get_image_tile(props->background_source) == 0) {
        return 0;
    }
    /* Label has no LD border/radius/padding; reject presence before create. */
    if ((props->fields
         & (TINYUI_LABEL_FIELD_BORDER_COLOR | TINYUI_LABEL_FIELD_RADIUS
            | TINYUI_LABEL_FIELD_PADDING))
        != 0) {
        return 0;
    }
    return 1;
}

static void *tinyui_runtime_internal_label_ld_init(void *ctx,
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
 * @return Pointer to the object on success, NULL on failure
 */

static tinyui_obj_t *tinyui_label_create_with_id(tinyui_obj_t *parent, uint16_t explicit_id)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "label";
    struct tinyui_label *label;
    struct tinyui_label_create_ctx ctx = {.id = id};

    if (parent_w == 0) {
        return 0;
    }
    label = (struct tinyui_label *)tinyui_runtime_internal_widget_create_leaf_with_id(
        parent_w,
        TINYUI_BACKEND_WIDGET_LABEL,
        tinyui_runtime_internal_label_ld_init,
        &ctx,
        sizeof(*label),
        explicit_id);
    if (label == 0) {
        return 0;
    }
    label->id = id;
    if (tinyui_label_set_font((tinyui_obj_t *)label, NULL) != 0) {
        tinyui_runtime_internal_widget_destroy_common(&label->widget);
        return 0;
    }

    return (tinyui_obj_t *)label;
}

tinyui_obj_t *tinyui_label_create(tinyui_obj_t *parent)
{
    return tinyui_label_create_with_id(parent, 0);
}

/**
 * @brief Create label widget with properties
 *
 * Presence fields are fully pre-validated, then ordinary create + formal setters
 * run. Any setter failure deletes the object and rolls back parent child count.
 */

tinyui_obj_t *tinyui_label_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_label_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_label *label;
    uint16_t explicit_id = 0;

    if (props == 0) {
        return tinyui_label_create(parent);
    }

    if (!tinyui_label_props_are_valid(props)) {
        return 0;
    }

    if ((props->fields & TINYUI_LABEL_FIELD_ID) != 0) {
        explicit_id = props->id;
    }

    obj = tinyui_label_create_with_id(parent, explicit_id);
    if (obj == 0) {
        return 0;
    }
    label = (struct tinyui_label *)(void *)obj;

    if ((props->fields & TINYUI_LABEL_FIELD_USER_DATA) != 0) {
        if (tinyui_runtime_internal_widget_set_user_data(&label->widget, props->user_data) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_STYLE_CLASS) != 0) {
        if (tinyui_runtime_internal_widget_set_style_class(&label->widget, props->style_class) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_WIDTH) != 0
        || (props->fields & TINYUI_LABEL_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&label->widget);
        int h = tinyui_runtime_internal_widget_get_height(&label->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_LABEL_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_LABEL_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_obj_set_size((tinyui_obj_t *)label, w, h) != TINYUI_OK) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_TEXT) != 0) {
        if (tinyui_label_set_text((tinyui_obj_t *)label, props->text) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_FONT) != 0) {
        if (tinyui_label_set_font((tinyui_obj_t *)label, props->font) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_BG_COLOR) != 0) {
        if (tinyui_label_set_bg_color((tinyui_obj_t *)label, props->bg_color) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_TEXT_COLOR) != 0) {
        if (tinyui_label_set_text_color((tinyui_obj_t *)label, props->text_color) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_TRANSPARENT) != 0) {
        if (tinyui_label_set_transparent((tinyui_obj_t *)label, props->transparent) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_ALIGN) != 0) {
        if (tinyui_label_set_align((tinyui_obj_t *)label, props->align) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LABEL_FIELD_BACKGROUND_SOURCE) != 0) {
        if (tinyui_label_set_background_source((tinyui_obj_t *)label, props->background_source) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)label);
            return 0;
        }
    }

    return obj;
}


/**
 * @brief Set text of label widget
 *
 * Thin forwarder to the common text path only (no second backend set).
 */

int tinyui_label_set_text(tinyui_obj_t *label_obj, const char *text)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    if (label == 0 || text == 0) {
        return -1;
    }

    return tinyui_obj_set_text((tinyui_obj_t *)label, text) == TINYUI_OK ? 0 : -1;
}

/**
 * @brief Get text of label widget
 */

const char * tinyui_label_get_text(tinyui_obj_t *label_obj)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    ldLabel_t *ld_label;

    if (label == 0) {
        return 0;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return 0;
    }

    return (const char *)ldLabelGetText(ld_label);
}

/**
 * @brief Set font of label widget
 */

int tinyui_label_set_font(tinyui_obj_t *label_obj, const struct tinyui_font *font)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    ldLabel_t *ld_label;
    arm_2d_font_t *resolved_font;

    if (label == 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }

    resolved_font = tinyui_resolve_ld_font(font, 12);
    if (resolved_font == 0) {
        return -1;
    }

    ldLabelSetFont(ld_label, resolved_font);
    label->widget.font = font;
    return 0;
}

/**
 * @brief Set text color of label widget
 */

int tinyui_label_set_text_color(tinyui_obj_t *label_obj, unsigned int rgb)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    if (label == 0) {
        return -1;
    }

    return tinyui_obj_set_text_color((tinyui_obj_t *)label, rgb) == TINYUI_OK ? 0 : -1;
}

/**
 * @brief Get text color of label widget
 */

int tinyui_label_get_text_color(tinyui_obj_t *label_obj, unsigned int *rgb)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
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
 */

int tinyui_label_set_bg_color(tinyui_obj_t *label_obj, unsigned int rgb)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    if (label == 0) {
        return -1;
    }

    return tinyui_obj_set_bg_color((tinyui_obj_t *)label, rgb) == TINYUI_OK ? 0 : -1;
}

/**
 * @brief Get bg color of label widget
 */

int tinyui_label_get_bg_color(tinyui_obj_t *label_obj, unsigned int *rgb)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
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
 */

int tinyui_label_set_transparent(tinyui_obj_t *label_obj, int transparent)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    ldLabel_t *ld_label;

    if (label == 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }

    ldLabelSetTransparent(ld_label, transparent != 0);
    return 0;
}

/**
 * @brief Get transparent of label widget
 */

int tinyui_label_get_transparent(tinyui_obj_t *label_obj, int *transparent)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
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
 */

int tinyui_label_set_align(tinyui_obj_t *label_obj, enum tinyui_align align)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    ldLabel_t *ld_label;

    if (label == 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }
    if (!tinyui_label_align_is_valid(align)) {
        return -1;
    }

    ldLabelSetAlign(ld_label, (arm_2d_align_t)tinyui_align_to_arm2d(align));
    return 0;
}

/**
 * @brief Get align of label widget
 */

int tinyui_label_get_align(tinyui_obj_t *label_obj, enum tinyui_align *align)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
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
 * @brief Set horizontal and vertical text alignment of label widget
 */

int tinyui_label_set_text_align(tinyui_obj_t *label_obj, enum tinyui_align x_align, enum tinyui_align y_align)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    ldLabel_t *ld_label;

    if (label == 0) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0 || !tinyui_label_align_is_valid(x_align)
        || !tinyui_label_align_is_valid(y_align)) {
        return -1;
    }

    ldLabelSetAlign(ld_label, tinyui_label_map_text_align(x_align, y_align));
    return 0;
}

/**
 * @brief Set background source of label widget
 */

int tinyui_label_set_background_source(tinyui_obj_t *label_obj, struct tinyui_image_source *source)
{
    struct tinyui_label *label = tinyui_label_as_label(label_obj);
    ldLabel_t *ld_label;

    if (label == 0 || (source != 0 && tinyui_image_source_get_image_tile(source) == 0)) {
        return -1;
    }

    ld_label = tinyui_label_backend(label);
    if (ld_label == 0) {
        return -1;
    }

    ldLabelSetBackgroundImage(ld_label,
                              source != NULL ? tinyui_image_source_get_image_tile(source) : NULL,
                              source != NULL ? tinyui_image_source_get_mask_tile(source) : NULL);
    return 0;
}
