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

#include "native.h"
#include "../backend/ldgui/backend.h"
#include "ldBase.h"

enum {
    PICOUI_LD_SIGNAL_NO_OPERATION = 0,
    PICOUI_LD_SIGNAL_PRESS = 1,
    PICOUI_LD_SIGNAL_HOLD_DOWN = 2,
    PICOUI_LD_SIGNAL_RELEASE = 3,
    PICOUI_LD_SIGNAL_CLICKED_ITEM = 12,
    PICOUI_LD_SIGNAL_FINISHED = 13,
    PICOUI_LD_SIGNAL_VALUE_CHANGED = 14,
};

enum {
    PICOUI_LD_NAV_UP = 0,
    PICOUI_LD_NAV_DOWN = 1,
    PICOUI_LD_NAV_LEFT = 2,
    PICOUI_LD_NAV_RIGHT = 3,
};

/**
 * @brief Native platform: image wrap
 *
 * @param[in] tile tile
 * @param[in] mask mask
 * @param[in] mask_color mask color
 * @return Pointer to the object
 */

struct picoui_native_image picoui_native_image_wrap(void *tile, void *mask, unsigned int mask_color)
{
    struct picoui_native_image image;

    image.tile = tile;
    image.mask = mask;
    image.mask_color = mask_color;
    return image;
}

/**
 * @brief Native platform: font wrap
 *
 * @param[in] font font
 * @return Pointer to the object
 */

struct picoui_native_font picoui_native_font_wrap(void *font)
{
    struct picoui_native_font native_font;

    native_font.font = font;
    return native_font;
}

/**
 * @brief Native platform: align to ld grid
 *
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int tinyui_native_align_to_ld_grid(enum picoui_native_align align)
{
    switch (align) {
    case PICOUI_NATIVE_ALIGN_END:
        return ldGridAlignEnd;
    case PICOUI_NATIVE_ALIGN_CENTER:
        return ldGridAlignCenter;
    case PICOUI_NATIVE_ALIGN_STRETCH:
        return ldGridAlignStretch;
    case PICOUI_NATIVE_ALIGN_SPACE_EVENLY:
        return ldGridAlignSpaceEvenly;
    case PICOUI_NATIVE_ALIGN_SPACE_AROUND:
        return ldGridAlignSpaceAround;
    case PICOUI_NATIVE_ALIGN_SPACE_BETWEEN:
        return ldGridAlignSpaceBetween;
    case PICOUI_NATIVE_ALIGN_START:
    default:
        return ldGridAlignStart;
    }
}

/**
 * @brief Native platform: signal to ld
 *
 * @param[in] signal signal
 * @return 0 on success, -1 on failure
 */

int tinyui_native_signal_to_ld(enum picoui_native_signal signal)
{
    switch (signal) {
    case PICOUI_NATIVE_SIGNAL_PRESS:
        return PICOUI_LD_SIGNAL_PRESS;
    case PICOUI_NATIVE_SIGNAL_HOLD_DOWN:
        return PICOUI_LD_SIGNAL_HOLD_DOWN;
    case PICOUI_NATIVE_SIGNAL_RELEASE:
        return PICOUI_LD_SIGNAL_RELEASE;
    case PICOUI_NATIVE_SIGNAL_CLICKED_ITEM:
        return PICOUI_LD_SIGNAL_CLICKED_ITEM;
    case PICOUI_NATIVE_SIGNAL_FINISHED:
        return PICOUI_LD_SIGNAL_FINISHED;
    case PICOUI_NATIVE_SIGNAL_VALUE_CHANGED:
        return PICOUI_LD_SIGNAL_VALUE_CHANGED;
    case PICOUI_NATIVE_SIGNAL_NONE:
    default:
        return PICOUI_LD_SIGNAL_NO_OPERATION;
    }
}

/**
 * @brief Native platform: readback policy to backend
 *
 * @param[in] policy policy
 * @return 0 on success, -1 on failure
 */

int tinyui_native_readback_policy_to_backend(enum picoui_native_readback_policy policy)
{
    switch (policy) {
    case PICOUI_NATIVE_READBACK_BACKEND_FIELD:
    case PICOUI_NATIVE_READBACK_BACKEND_COMMITTED:
        return PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE;
    case PICOUI_NATIVE_READBACK_NOT_APPLICABLE:
    default:
        return PICOUI_BACKEND_DATA_TRUTH_NOT_APPLICABLE;
    }
}
