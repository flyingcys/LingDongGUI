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
#include "widgets/image.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldImage.h"

#include <stdlib.h>
#include <string.h>


static struct tinyui_image *tinyui_image_as_image(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_IMAGE)) {
        return 0;
    }
    return (struct tinyui_image *)w;
}

static const struct tinyui_image *tinyui_image_as_image_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_IMAGE)) {
        return 0;
    }
    return (const struct tinyui_image *)w;
}

/* ---- test seam state ---- */


static int image_props_valid(const tinyui_image_props_t *props)
{
    return props != 0
        && (props->source == 0 || tinyui_image_source_get_image_tile(props->source) != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_runtime_internal_image_ld_init(void *ctx,
                                  struct ld_scene_t *scene,
                                  uint16_t name_id,
                                  uint16_t parent_name_id)
{
    (void)ctx;
    return ldImage_init(scene,
                        NULL,
                        name_id,
                        parent_name_id,
                        0,
                        0,
                        220,
                        56,
                        NULL,
                        NULL);
}

/**
 * @brief Create image widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_image_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "image";
    if (parent_w == 0) { return 0; }

    struct tinyui_image *image;

    if (parent_w == 0 || id == 0) {
        return 0;
    }
    image = (struct tinyui_image *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                             TINYUI_BACKEND_WIDGET_IMAGE,
                                                             tinyui_runtime_internal_image_ld_init,
                                                             0,
                                                             sizeof(*image));
    if (image == 0) {
        return 0;
    }
    image->id = id;

    return (tinyui_obj_t *)image;
}

/**
 * @brief Create image widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_image_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_image_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_image *image;

    if (props == 0) {
        return tinyui_image_create(parent);
    }

    obj = tinyui_image_create(parent);
    if (obj == 0) {
        return 0;
    }
    image = (struct tinyui_image *)(void *)obj;

    if ((props->fields & TINYUI_IMAGE_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_IMAGE_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&image->widget, props->user_data) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)image);
        return 0;
    }
    }
    if ((props->fields & TINYUI_IMAGE_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&image->widget, props->style_class) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)image);
        return 0;
    }
    }
        if ((props->fields & TINYUI_IMAGE_FIELD_WIDTH) != 0 || (props->fields & TINYUI_IMAGE_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&image->widget);
        int h = tinyui_runtime_internal_widget_get_height(&image->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_IMAGE_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_IMAGE_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&image->widget, w, h) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)image);
            return 0;
        }
    }
    if ((props->fields & TINYUI_IMAGE_FIELD_BG_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_bg_color(&image->widget, props->bg_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)image);
        return 0;
    }
    }
    if ((props->fields & TINYUI_IMAGE_FIELD_TEXT_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_text_color(&image->widget, props->text_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)image);
        return 0;
    }
    }
    if ((props->fields & TINYUI_IMAGE_FIELD_BORDER_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_border_color(&image->widget, props->border_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)image);
        return 0;
    }
    }
    if ((props->fields & TINYUI_IMAGE_FIELD_RADIUS) != 0) {
    if (tinyui_runtime_internal_widget_set_radius(&image->widget, props->radius) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)image);
        return 0;
    }
    }
    if ((props->fields & TINYUI_IMAGE_FIELD_PADDING) != 0) {
    if (tinyui_runtime_internal_widget_set_padding(&image->widget, props->padding) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)image);
        return 0;
    }
    }
    if ((props->fields & TINYUI_IMAGE_FIELD_SOURCE) != 0) {
    if (tinyui_image_set_source((tinyui_obj_t *)image, props->source) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)image);
        return 0;
    }
    }

    return obj;
}


/**
 * @brief Set source of image widget
 *
 * @param[in] image Image widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_image_set_source(tinyui_obj_t *image_obj, struct tinyui_image_source *source)
{
    struct tinyui_image *image = tinyui_image_as_image(image_obj);
    if (image == 0) { return -1; }

    if (image == 0 || (source != 0 && tinyui_image_source_get_image_tile(source) == 0)) {
        return -1;
    }

    if (image->widget.ld_widget == 0 || image->widget.kind != TINYUI_BACKEND_WIDGET_IMAGE) {
        return -1;
    }

    ldImageSetImage((ldImage_t *)image->widget.ld_widget,
                    source != 0 ? tinyui_image_source_get_image_tile(source) : 0,
                    source != 0 ? tinyui_image_source_get_mask_tile(source) : 0);
    image->source = source;
    return 0;
}

/**
 * @brief Set mask color of image widget
 *
 * @param[in] image Image widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_image_set_mask_color(tinyui_obj_t *image_obj, unsigned int rgb)
{
    struct tinyui_image *image = tinyui_image_as_image(image_obj);
    if (image == 0) { return -1; }

    if (image == 0) {
        return -1;
    }

    if (image->widget.ld_widget == 0 || image->widget.kind != TINYUI_BACKEND_WIDGET_IMAGE) {
        return -1;
    }

    ldImageSetMaskColor((ldImage_t *)image->widget.ld_widget,
                        (ldColor)tinyui_rgb_to_ld_color(rgb));
    image->widget.bg_color = rgb;
    return 0;
}
