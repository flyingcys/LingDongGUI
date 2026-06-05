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
#include "picoui/image.h"

#include <stdlib.h>

int picoui_native_image_set_source(struct picoui_image *image, struct picoui_image_source *source);

static int picoui_image_props_are_valid(const struct picoui_image_props *props)
{
    return props != 0
        && props->id != 0
        && (props->source == 0 || props->source->img_tile != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

/**
 * @brief Create image widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_image *picoui_image_create(struct picoui_window *parent, const char *id)
{
    struct picoui_image *image;

    if (parent == 0 || id == 0) {
        return 0;
    }

    image = calloc(1, sizeof(*image));
    if (image == 0) {
        return 0;
    }

    image->widget.backend_widget = picoui_backend_create_image(parent->widget.backend_widget, id);
    if (image->widget.backend_widget == 0) {
        free(image);
        return 0;
    }
    if (picoui_backend_widget_bind_host(image->widget.backend_widget, &image->widget) != 0) {
        free(image);
        return 0;
    }

    image->id = id;
    image->widget.visible = 1;
    image->widget.enabled = 1;
    return image;
}

/**
 * @brief Create image widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_image *picoui_image_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_image_props *props)
{
    struct picoui_image *image;

    if (!picoui_image_props_are_valid(props)) {
        return 0;
    }

    image = picoui_image_create(parent, props->id);
    if (image == 0) {
        return 0;
    }

    if (props->source != 0 && picoui_image_set_source(image, props->source) != 0) {
        free(image);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&image->widget, props->style_class) != 0) {
        free(image);
        return 0;
    }
    if (picoui_widget_set_user_data(&image->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&image->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&image->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&image->widget, props->border_color) != 0
        || picoui_widget_set_radius(&image->widget, props->radius) != 0
        || picoui_widget_set_padding(&image->widget, props->padding) != 0) {
        free(image);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&image->widget, props->width, props->height) != 0) {
        free(image);
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

int picoui_image_set_source(struct picoui_image *image, struct picoui_image_source *source)
{
    struct picoui_backend_widget *backend;
    struct picoui_image_source *old_source;
    struct picoui_image_source *old_backend_source;

    if (image == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)image->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }

    old_source = image->source;
    old_backend_source = backend->image_source;
    if (picoui_backend_set_image_source(image->widget.backend_widget, source) != 0) {
        return -1;
    }

    if (picoui_native_image_set_source(image, source) != 0) {
        (void)picoui_backend_set_image_source(image->widget.backend_widget, old_source);
        backend->image_source = old_backend_source;
        image->source = old_source;
        return -1;
    }

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

int picoui_image_set_mask_color(struct picoui_image *image, unsigned int rgb)
{
    if (image == 0) {
        return -1;
    }

    if (picoui_backend_image_set_mask_color(image->widget.backend_widget, rgb) != 0) {
        return -1;
    }

    image->widget.bg_color = rgb;
    return 0;
}
