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

#ifndef PICOUI_PROGRESS_WHEEL_H
#define PICOUI_PROGRESS_WHEEL_H

struct picoui_widget;
struct picoui_progress_wheel;

struct picoui_progress_wheel_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int dot_enabled;
};

/**
 * @brief Create progress wheel widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_progress_wheel *picoui_progress_wheel_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create progress wheel widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_progress_wheel *picoui_progress_wheel_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_progress_wheel_props *props);

/**
 * @brief progress wheel init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_progress_wheel *picoui_progress_wheel_init(struct picoui_widget *parent, const char *id);

/**
 * @brief Set percent of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int picoui_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent);

/**
 * @brief Set progress of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int picoui_progress_wheel_set_progress(struct picoui_progress_wheel *wheel, int percent);

/**
 * @brief Get percent of progress wheel widget
 *
 * @param[in] wheel wheel
 * @return The property value, negative on error
 */

int picoui_progress_wheel_get_percent(const struct picoui_progress_wheel *wheel);

/**
 * @brief Set wheel color of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_progress_wheel_set_wheel_color(struct picoui_progress_wheel *wheel, unsigned int rgb);

/**
 * @brief Set dot color of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_progress_wheel_set_dot_color(struct picoui_progress_wheel *wheel, unsigned int rgb);

/**
 * @brief Set dot enabled of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_progress_wheel_set_dot_enabled(struct picoui_progress_wheel *wheel, int enabled);

/**
 * @brief Get dot enabled of progress wheel widget
 *
 * @param[in] wheel wheel
 * @return The property value, negative on error
 */

int picoui_progress_wheel_get_dot_enabled(const struct picoui_progress_wheel *wheel);

#endif
