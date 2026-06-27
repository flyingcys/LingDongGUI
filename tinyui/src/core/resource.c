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
#include "widgets/image.h"
#include "../../../examples/common/demo/widget/images/uiImages.h"
#include "../../../src/gui/ldBase.h"

#include <string.h>

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;

/**
 * @brief Image source: from vres
 *
 * @param[in] addr Address
 * @param[in] out Output parameter
 * @return 0 on success, -1 on failure
 */

int tinyui_image_source_from_vres(unsigned int addr, struct tinyui_image_source *out)
{
    if (addr == 0 || out == 0) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->img_tile = ldBaseGetVresImage(addr);
    if (out->img_tile == 0) {
        return -1;
    }
    out->kind = TINYUI_IMAGE_SOURCE_KIND_VRES;
    out->vres_addr = addr;

    return 0;
}

int tinyui_image_source_from_builtin(enum tinyui_builtin_image image,
                                     struct tinyui_image_source *out)
{
    void *img_tile = 0;
    void *mask_tile = 0;

    if (out == 0) {
        return -1;
    }

    switch (image) {
    case TINYUI_BUILTIN_IMAGE_LETTER_PAPER:
        img_tile = IMAGE_LETTER_PAPER_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_KEY_RELEASE:
        img_tile = IMAGE_KEYRELEASE_PNG;
        mask_tile = IMAGE_KEYRELEASE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_KEY_PRESS:
        img_tile = IMAGE_KEYPRESS_PNG;
        mask_tile = IMAGE_KEYPRESS_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_PROGRESS_BG:
        img_tile = IMAGE_PROGRESSBARBG_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_PROGRESS_FG:
        img_tile = IMAGE_PROGRESSBARFG_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_SLIDER_BG:
        img_tile = IMAGE_SLIDER_PNG;
        mask_tile = IMAGE_SLIDER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR:
        img_tile = IMAGE_INDICATOR_PNG;
        mask_tile = IMAGE_INDICATOR_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_WEATHER:
        img_tile = IMAGE_WEATHER_PNG;
        mask_tile = IMAGE_WEATHER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_NOTE:
        img_tile = IMAGE_NOTE_PNG;
        mask_tile = IMAGE_NOTE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_BOOK:
        img_tile = IMAGE_BOOK_PNG;
        mask_tile = IMAGE_BOOK_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_CHART:
        img_tile = IMAGE_CHART_PNG;
        mask_tile = IMAGE_CHART_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_GAUGE_BG:
        img_tile = IMAGE_GAUGE_PNG;
        mask_tile = IMAGE_GAUGE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_GAUGE_POINTER:
        img_tile = IMAGE_GAUGEPOINTER_PNG;
        mask_tile = IMAGE_GAUGEPOINTER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_ARC_QUARTER:
        img_tile = (void *)&c_tileQuaterArcGRAY8;
        mask_tile = (void *)&c_tileQuaterArcMask;
        break;
    default:
        return -1;
    }

    if (img_tile == 0) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->img_tile = img_tile;
    out->mask_tile = mask_tile;
    out->kind = TINYUI_IMAGE_SOURCE_KIND_BUILTIN;
    return 0;
}

/**
 * @brief Font: from vres
 *
 * @param[in] addr Address
 * @param[in] out Output parameter
 * @return 0 on success, -1 on failure
 */

int tinyui_font_from_vres(unsigned int addr, struct tinyui_font *out)
{
    if (addr == 0 || out == 0) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->kind = TINYUI_FONT_KIND_VRES;
    out->vres_addr = addr;
    return 0;
}

/**
 * @brief Destroy image source widget
 *
 * @param[out] source Image source
 */

void tinyui_image_source_destroy(struct tinyui_image_source *source)
{
    if (source == 0) {
        return;
    }

    if (source->kind == TINYUI_IMAGE_SOURCE_KIND_VRES
        && source->img_tile != 0) {
        ldFree(source->img_tile);
    }
    memset(source, 0, sizeof(*source));
}

/**
 * @brief Destroy font widget
 *
 * @param[out] font font
 */

void tinyui_font_destroy(struct tinyui_font *font)
{
    if (font == 0) {
        return;
    }

    memset(font, 0, sizeof(*font));
}
