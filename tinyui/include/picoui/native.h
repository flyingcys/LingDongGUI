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

struct picoui_native_image {
    void *tile;
    void *mask;
    unsigned int mask_color;
};

struct picoui_native_font {
    void *font;
};

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
