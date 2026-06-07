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

#ifndef PICOUI_FONT_H
#define PICOUI_FONT_H

#include "picoui/native.h"

enum picoui_font_kind {
    PICOUI_FONT_KIND_FAMILY = 0,
    PICOUI_FONT_KIND_VRES = 1,
};

struct picoui_font {
    const char *family;
    int size;
    enum picoui_font_kind kind;
    unsigned int vres_addr;
};

/**
 * @brief Resolve a public font handle to native font pointer.
 *
 * @param[in] font Public font handle, nullable
 * @return Wrapped native font. When input is null or unsupported, returns default font.
 */
struct picoui_native_font picoui_font_resolve_native(const struct picoui_font *font);

/**
 * @brief Font: from vres
 *
 * @param[in] addr Address
 * @param[in] out Output parameter
 * @return 0 on success, -1 on failure
 */
int picoui_font_from_vres(unsigned int addr, struct picoui_font *out);

/**
 * @brief Destroy font widget
 *
 * @param[in] font font
 */
void picoui_font_destroy(struct picoui_font *font);

#endif
