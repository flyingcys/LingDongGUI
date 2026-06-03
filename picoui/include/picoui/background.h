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

#ifndef PICOUI_BACKGROUND_H
#define PICOUI_BACKGROUND_H

struct picoui_app;
struct picoui_background;
struct picoui_image_source;

/**
 * @brief Create background widget
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_background *picoui_background_create(struct picoui_app *app, const char *id);

/**
 * @brief Set source of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_background_set_source(struct picoui_background *background,
                                 struct picoui_image_source *source);

/**
 * @brief Set color of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_background_set_color(struct picoui_background *background, unsigned int rgb);

/**
 * @brief Get color of background widget
 *
 * @param[out] background Background widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int picoui_background_get_color(struct picoui_background *background, unsigned int *rgb);

/**
 * @brief Set offset of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int picoui_background_set_offset(struct picoui_background *background, int offset_x, int offset_y);

/**
 * @brief Get offset of background widget
 *
 * @param[out] background Background widget instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return The property value, negative on error
 */

int picoui_background_get_offset(struct picoui_background *background,
                                 int *offset_x,
                                 int *offset_y);

#endif
