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

#ifndef PICOUI_CANVAS_H
#define PICOUI_CANVAS_H

#include "picoui/image.h"
#include "picoui/theme.h"

struct picoui_window;
struct picoui_canvas;

/**
 * @brief Create canvas widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_canvas *picoui_canvas_create(struct picoui_window *parent, const char *id);

/**
 * @brief canvas clear
 *
 * @param[in] canvas Canvas widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_canvas_clear(struct picoui_canvas *canvas);

/**
 * @brief canvas fill rect
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @param[in] opacity Opacity (0-255)
 * @return 0 on success, -1 on failure
 */

int picoui_canvas_fill_rect(struct picoui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            unsigned int rgb,
                            int opacity);

/**
 * @brief canvas draw line
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x0 x0
 * @param[in] y0 y0
 * @param[in] x1 x1
 * @param[in] y1 y1
 * @param[in] line_size line size
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @param[in] opacity_max opacity max
 * @param[in] opacity_min opacity min
 * @return 0 on success, -1 on failure
 */

int picoui_canvas_draw_line(struct picoui_canvas *canvas,
                            int x0,
                            int y0,
                            int x1,
                            int y1,
                            int line_size,
                            unsigned int rgb,
                            int opacity_max,
                            int opacity_min);

/**
 * @brief canvas draw image
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] source Image source
 * @param[in] mask_color mask color
 * @param[in] opacity Opacity (0-255)
 * @return 0 on success, -1 on failure
 */

int picoui_canvas_draw_image(struct picoui_canvas *canvas,
                             int x,
                             int y,
                             int width,
                             int height,
                             struct picoui_image_source *source,
                             unsigned int mask_color,
                             int opacity);

/**
 * @brief canvas draw image scaled
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] source Image source
 * @param[in] scale Scale factor
 * @param[in] opacity Opacity (0-255)
 * @return 0 on success, -1 on failure
 */

int picoui_canvas_draw_image_scaled(struct picoui_canvas *canvas,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    struct picoui_image_source *source,
                                    float scale,
                                    int opacity);

/**
 * @brief canvas draw text
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] text Text widget instance
 * @param[in] align align
 * @param[in] text_color Text color
 * @param[in] opacity Opacity (0-255)
 * @return 0 on success, -1 on failure
 */

int picoui_canvas_draw_text(struct picoui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            const char *text,
                            enum picoui_align align,
                            unsigned int text_color,
                            int opacity);

/**
 * @brief Get command count of canvas widget
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] count Count
 * @return The property value, negative on error
 */

int picoui_canvas_get_command_count(const struct picoui_canvas *canvas, int *count);

#endif
