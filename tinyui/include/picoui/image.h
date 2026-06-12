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

#ifndef PICOUI_IMAGE_H
#define PICOUI_IMAGE_H

#include "../widget.h"

struct picoui_window;
struct picoui_image;

struct picoui_image_source {
    void *img_tile;
    void *mask_tile;
    unsigned int kind;
    unsigned int vres_addr;
};

struct picoui_image_props {
    const char *id;
    struct picoui_image_source *source;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

/**
 * @brief Create image widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_image *picoui_image_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create image widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_image *picoui_image_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_image_props *props);

/**
 * @brief Image source: from vres
 *
 * @param[in] addr Address
 * @param[in] out Output parameter
 * @return 0 on success, -1 on failure
 */

int picoui_image_source_from_vres(unsigned int addr, struct picoui_image_source *out);

/**
 * @brief Font: from vres
 *
 * @param[in] addr Address
 * @param[in] out Output parameter
 * @return 0 on success, -1 on failure
 */

int picoui_font_from_vres(unsigned int addr, struct picoui_font *out);

/**
 * @brief Destroy image source widget
 *
 * @param[in] source Image source
 */

void picoui_image_source_destroy(struct picoui_image_source *source);

/**
 * @brief Destroy font widget
 *
 * @param[in] font font
 */

void picoui_font_destroy(struct picoui_font *font);

/**
 * @brief Set source of image widget
 *
 * @param[in] image Image widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_image_set_source(struct picoui_image *image, struct picoui_image_source *source);

/**
 * @brief Set mask color of image widget
 *
 * @param[in] image Image widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_image_set_mask_color(struct picoui_image *image, unsigned int rgb);

#endif
