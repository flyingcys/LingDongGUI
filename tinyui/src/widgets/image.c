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

int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static ldColor tinyui_image_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static int tinyui_image_props_are_valid(const struct tinyui_image_props *props)
{
    return props != 0
        && props->id != 0
        && (props->source == 0 || props->source->img_tile != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static int tinyui_image_finish_detach_after_backend_failure(
    struct tinyui_backend_widget *backend)
{
    struct tinyui_backend_widget *parent;
    struct tinyui_backend_widget *cursor;

    if (backend == 0 || backend->parent == 0) {
        return 0;
    }

    parent = backend->parent;
    if (parent->first_child == backend) {
        parent->first_child = backend->next_sibling;
    } else {
        cursor = parent->first_child;
        while (cursor != 0 && cursor->next_sibling != backend) {
            cursor = cursor->next_sibling;
        }
        if (cursor == 0) {
            return -1;
        }
        cursor->next_sibling = backend->next_sibling;
    }

    backend->parent = 0;
    backend->next_sibling = 0;
    backend->owner = 0;
    backend->root = 0;
    return 0;
}

static void tinyui_image_dispose_partial_impl(struct tinyui_image *image)
{
    struct tinyui_backend_widget *backend;
    struct tinyui_app *app_state;
    int detach_result = 0;

    if (image == 0) {
        return;
    }

    backend = (struct tinyui_backend_widget *)image->widget.backend_widget;
    if (backend != 0) {
        app_state = tinyui_runtime_bridge_backend_state(backend->owner);
        if (backend->parent != 0) {
            detach_result = tinyui_runtime_bridge_detach_from_parent(backend);
            if (detach_result != 0) {
                detach_result = tinyui_image_finish_detach_after_backend_failure(backend);
            }
        }
        (void)tinyui_runtime_bridge_unbind_host(backend);
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldImage_depose(app_state->ld_scene, (ldImage_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(image);
}

static struct tinyui_image *tinyui_image_create_with_props_impl(
    struct tinyui_window *parent,
    const struct tinyui_image_props *props)
{
    struct tinyui_image *image;

    if (!tinyui_image_props_are_valid(props)) {
        return 0;
    }

    image = tinyui_image_create(parent, props->id);
    if (image == 0) {
        return 0;
    }

    if (props->source != 0 && tinyui_image_set_source(image, props->source) != 0) {
        tinyui_image_dispose_partial_impl(image);
        return 0;
    }
    if (props->style_class != 0
        && tinyui_widget_set_style_class(&image->widget, props->style_class) != 0) {
        tinyui_image_dispose_partial_impl(image);
        return 0;
    }
    if (tinyui_widget_set_user_data(&image->widget, props->user_data) != 0
        || tinyui_widget_set_bg_color(&image->widget, props->bg_color) != 0
        || tinyui_widget_set_text_color(&image->widget, props->text_color) != 0
        || tinyui_widget_set_border_color(&image->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&image->widget, props->radius) != 0
        || tinyui_widget_set_padding(&image->widget, props->padding) != 0
        || ((props->width > 0 || props->height > 0)
            && tinyui_widget_set_size(&image->widget, props->width, props->height) != 0)) {
        tinyui_image_dispose_partial_impl(image);
        return 0;
    }

    return image;
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
    struct tinyui_backend_widget *backend;
    struct tinyui_backend_widget *parent_backend;
    struct tinyui_app *app_state;
    ldImage_t *ld_image;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct tinyui_backend_widget *)parent->widget.backend_widget;
    app_state = parent_backend != 0
        ? tinyui_runtime_bridge_backend_state_from_parent(parent_backend)
        : 0;
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    image = calloc(1, sizeof(*image));
    if (image == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(image);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(image);
        return 0;
    }

    ld_image = ldImage_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_backend->ld_name_id,
                            0,
                            0,
                            220,
                            56,
                            NULL,
                            NULL);
    if (ld_image == 0) {
        free(backend);
        free(image);
        return 0;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         TINYUI_BACKEND_WIDGET_IMAGE,
                                         id,
                                         parent_backend->theme) != 0) {
        ldImage_depose(app_state->ld_scene, ld_image);
        free(backend);
        free(image);
        return 0;
    }
    backend->ld_widget = ld_image;
    backend->ld_name_id = name_id;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldImage_depose(app_state->ld_scene, ld_image);
        free(backend);
        free(image);
        return 0;
    }

    image->widget.backend_widget = backend;
    image->id = id;
    image->widget.visible = 1;
    image->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(image->widget.backend_widget, &image->widget) != 0) {
        tinyui_image_dispose_partial_impl(image);
        return 0;
    }
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
    return tinyui_image_create_with_props_impl(parent, props);
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
    struct tinyui_backend_widget *backend;
    ldImage_t *ld_image;

    if (image == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    backend = (struct tinyui_backend_widget *)image->widget.backend_widget;
    if (backend == 0 || backend->kind != TINYUI_BACKEND_WIDGET_IMAGE || backend->ld_widget == 0) {
        return -1;
    }
    ld_image = (ldImage_t *)backend->ld_widget;

    backend->image_source = source;
    ldImageSetImage(ld_image,
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
    struct tinyui_backend_widget *backend;
    ldImage_t *ld_image;

    if (image == 0) {
        return -1;
    }

    backend = (struct tinyui_backend_widget *)image->widget.backend_widget;
    if (backend == 0 || backend->kind != TINYUI_BACKEND_WIDGET_IMAGE || backend->ld_widget == 0) {
        return -1;
    }
    ld_image = (ldImage_t *)backend->ld_widget;

    ldImageSetMaskColor(ld_image, tinyui_image_rgb_to_ld_color(rgb));
    image->widget.bg_color = rgb;
    return 0;
}
