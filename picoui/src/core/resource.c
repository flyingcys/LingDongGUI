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
#include "picoui/native.h"
#include "picoui/image.h"
#include "../../../src/gui/ldBase.h"

#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_a1_font_t ARM_2D_FONT_16x24;

static void *picoui_font_default_pointer(void)
{
    return (void *)&ARM_2D_FONT_6x8;
}

/**
 * @brief Image source: from vres
 *
 * @param[in] addr Address
 * @param[in] out Output parameter
 * @return 0 on success, -1 on failure
 */

int picoui_image_source_from_vres(unsigned int addr, struct picoui_image_source *out)
{
    if (addr == 0 || out == 0) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->img_tile = ldBaseGetVresImage(addr);
    if (out->img_tile == 0) {
        return -1;
    }
    out->vres_addr = addr;

    return 0;
}

/**
 * @brief Font: from vres
 *
 * @param[in] addr Address
 * @param[in] out Output parameter
 * @return 0 on success, -1 on failure
 */

int picoui_font_from_vres(unsigned int addr, struct picoui_font *out)
{
    if (addr == 0 || out == 0) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->kind = PICOUI_FONT_KIND_VRES;
    out->vres_addr = addr;
    return 0;
}

struct picoui_native_font picoui_font_resolve_native(const struct picoui_font *font)
{
    if (font != 0 && font->kind == PICOUI_FONT_KIND_VRES && font->vres_addr != 0) {
        return picoui_native_font_wrap(ldBaseGetVresFont(font->vres_addr));
    }

    if (font != 0 && font->family != 0 && font->size >= 20 && strcmp(font->family, "Sans") == 0) {
        return picoui_native_font_wrap((void *)&ARM_2D_FONT_16x24);
    }

    return picoui_native_font_wrap(picoui_font_default_pointer());
}

/**
 * @brief Destroy image source widget
 *
 * @param[out] source Image source
 */

void picoui_image_source_destroy(struct picoui_image_source *source)
{
    if (source == 0) {
        return;
    }

    if (source->img_tile != 0) {
        ldFree(source->img_tile);
    }
    memset(source, 0, sizeof(*source));
}

/**
 * @brief Destroy font widget
 *
 * @param[out] font font
 */

void picoui_font_destroy(struct picoui_font *font)
{
    if (font == 0) {
        return;
    }

    memset(font, 0, sizeof(*font));
}
