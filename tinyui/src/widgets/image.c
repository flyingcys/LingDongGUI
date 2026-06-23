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
#include "image.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldImage.h"

#include <stdlib.h>
#include <string.h>

/* ---- test seam state ---- */
static ld_scene_t *s_image_depose_scene = NULL;

static void tinyui_image_ld_depose_cb(void *ld_widget)
{
    if (s_image_depose_scene != NULL) {
        ldImage_depose(s_image_depose_scene, (ldImage_t *)ld_widget);
        s_image_depose_scene = NULL;
    }
}

static int image_props_valid(const struct tinyui_image_props *props)
{
    return props != 0
        && props->id != 0
        && (props->source == 0 || props->source->img_tile != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_image_ld_init(void *ctx,
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

struct tinyui_image *tinyui_image_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_image *image;

    if (parent == 0 || id == 0) {
        return 0;
    }
    image = (struct tinyui_image *)tinyui_widget_create_leaf(&parent->widget,
                                                             TINYUI_BACKEND_WIDGET_IMAGE,
                                                             tinyui_image_ld_init,
                                                             0,
                                                             sizeof(*image));
    if (image == 0) {
        return 0;
    }
    image->id = id;

    return image;
}

/**
 * @brief Create image widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_image *tinyui_image_create_with_props(struct tinyui_window *parent,
                                                    const struct tinyui_image_props *props)
{
    struct tinyui_image *image;

    if (!image_props_valid(props)) {
        return 0;
    }

    image = tinyui_image_create(parent, props->id);
    if (image == 0) {
        return 0;
    }

    if ((props->source != 0 && tinyui_image_set_source(image, props->source) != 0)
        || (props->style_class != 0
            && tinyui_widget_set_style_class(&image->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&image->widget, props->user_data) != 0
        || tinyui_widget_set_bg_color(&image->widget, props->bg_color) != 0
        || tinyui_widget_set_text_color(&image->widget, props->text_color) != 0
        || tinyui_widget_set_border_color(&image->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&image->widget, props->radius) != 0
        || tinyui_widget_set_padding(&image->widget, props->padding) != 0
        || ((props->width > 0 || props->height > 0)
            && tinyui_widget_set_size(&image->widget, props->width, props->height) != 0)) {
        s_image_depose_scene = image->widget.ld_event_bridge_scene;
        tinyui_widget_destroy_common(&image->widget, tinyui_image_ld_depose_cb);
        return 0;
    }

    return image;
}

/**
 * @brief Set source of image widget
 *
 * @param[in] image Image widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_image_set_source(struct tinyui_image *image, struct tinyui_image_source *source)
{
    if (image == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    if (image->widget.ld_widget == 0 || image->widget.kind != TINYUI_BACKEND_WIDGET_IMAGE) {
        return -1;
    }

    ldImageSetImage((ldImage_t *)image->widget.ld_widget,
                    source != 0 ? source->img_tile : 0,
                    source != 0 ? source->mask_tile : 0);
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

int tinyui_image_set_mask_color(struct tinyui_image *image, unsigned int rgb)
{
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
