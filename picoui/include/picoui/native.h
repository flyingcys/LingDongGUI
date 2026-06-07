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

#ifndef PICOUI_NATIVE_H
#define PICOUI_NATIVE_H

struct picoui_rect;
struct picoui_image_source;
struct picoui_canvas;
struct picoui_qrcode;
struct picoui_animation;

struct picoui_native_image {
    void *tile;
    void *mask;
    unsigned int mask_color;
};

struct picoui_native_font {
    void *font;
};

/**
 * @brief Get default native font.
 *
 * @return Wrapped default font.
 */
struct picoui_native_font picoui_native_font_default(void);

/**
 * @brief Render text into a software buffer using native font fallback rules.
 *
 * @param[in] font Native font wrapper
 * @param[in] text UTF-8/ASCII text pointer
 * @param[out] buffer ARGB/RGB software buffer
 * @param[in] width Buffer width
 * @param[in] height Buffer height
 * @param[in] rgb Foreground color
 * @param[out] dirty_rect Dirty area written by renderer
 * @return 0 on success, -1 on failure
 */
int picoui_native_font_render_text(struct picoui_native_font font,
                                   const char *text,
                                   unsigned int *buffer,
                                   int width,
                                   int height,
                                   unsigned int rgb,
                                   struct picoui_rect *dirty_rect);

/**
 * @brief Render an image source into a software buffer.
 *
 * @param[in] source Image source
 * @param[in] mask_color Mask recolor value for masked pixels
 * @param[out] buffer ARGB buffer
 * @param[in] width Buffer width
 * @param[in] height Buffer height
 * @param[out] dirty_rect Dirty area written by renderer
 * @return 0 on success, -1 on failure
 */
int picoui_native_image_render_buffer(const struct picoui_image_source *source,
                                      unsigned int mask_color,
                                      unsigned int *buffer,
                                      int width,
                                      int height,
                                      struct picoui_rect *dirty_rect);

/**
 * @brief Render canvas primitive commands into a software buffer.
 *
 * @param[in] canvas Canvas widget
 * @param[out] buffer ARGB buffer
 * @param[in] width Buffer width
 * @param[in] height Buffer height
 * @param[out] dirty_rect Dirty area written by renderer
 * @return 0 on success, -1 on failure
 */
int picoui_native_canvas_render_buffer(const struct picoui_canvas *canvas,
                                       unsigned int *buffer,
                                       int width,
                                       int height,
                                       struct picoui_rect *dirty_rect);

/**
 * @brief Render qrcode modules into a software buffer.
 *
 * @param[in] qrcode QRCode widget
 * @param[out] buffer ARGB buffer
 * @param[in] width Buffer width
 * @param[in] height Buffer height
 * @param[out] dirty_rect Dirty area written by renderer
 * @return 0 on success, -1 on failure
 */
int picoui_native_qrcode_render_buffer(const struct picoui_qrcode *qrcode,
                                       unsigned int *buffer,
                                       int width,
                                       int height,
                                       struct picoui_rect *dirty_rect);

/**
 * @brief Bind animation to an explicit list of frame sources.
 *
 * @param[in] animation Animation widget
 * @param[in] sources Frame source array
 * @param[in] frame_count Frame count
 * @return 0 on success, -1 on failure
 */
int picoui_native_animation_bind_frame_sources(struct picoui_animation *animation,
                                               struct picoui_image_source **sources,
                                               int frame_count);

enum picoui_native_align {
    PICOUI_NATIVE_ALIGN_START = 0,
    PICOUI_NATIVE_ALIGN_CENTER,
    PICOUI_NATIVE_ALIGN_END,
    PICOUI_NATIVE_ALIGN_STRETCH,
    PICOUI_NATIVE_ALIGN_SPACE_EVENLY,
    PICOUI_NATIVE_ALIGN_SPACE_AROUND,
    PICOUI_NATIVE_ALIGN_SPACE_BETWEEN
};

enum picoui_native_nav_dir {
    PICOUI_NATIVE_NAV_LEFT = 0,
    PICOUI_NATIVE_NAV_RIGHT,
    PICOUI_NATIVE_NAV_UP,
    PICOUI_NATIVE_NAV_DOWN,
    PICOUI_NATIVE_NAV_ENTER,
    PICOUI_NATIVE_NAV_BACK
};

enum picoui_native_signal {
    PICOUI_NATIVE_SIGNAL_NONE = 0,
    PICOUI_NATIVE_SIGNAL_PRESS,
    PICOUI_NATIVE_SIGNAL_HOLD_DOWN,
    PICOUI_NATIVE_SIGNAL_RELEASE,
    PICOUI_NATIVE_SIGNAL_CLICKED_ITEM,
    PICOUI_NATIVE_SIGNAL_FINISHED,
    PICOUI_NATIVE_SIGNAL_VALUE_CHANGED
};

enum picoui_native_readback_policy {
    PICOUI_NATIVE_READBACK_NOT_APPLICABLE = 0,
    PICOUI_NATIVE_READBACK_BACKEND_FIELD,
    PICOUI_NATIVE_READBACK_BACKEND_COMMITTED
};

/**
 * @brief Native platform: image wrap
 *
 * @param[in] tile tile
 * @param[in] mask mask
 * @param[in] mask_color mask color
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_native_image picoui_native_image_wrap(void *tile, void *mask, unsigned int mask_color);

/**
 * @brief Native platform: font wrap
 *
 * @param[in] font font
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_native_font picoui_native_font_wrap(void *font);

#endif
