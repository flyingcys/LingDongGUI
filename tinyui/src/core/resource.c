/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "internal.h"
#include "widgets/image.h"
#include "../../../examples/common/demo/widget/images/uiImages.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"
#include "../../../src/gui/ldBase.h"

#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_a1_font_t ARM_2D_FONT_16x24;

_Static_assert(sizeof(((tinyui_image_source_t *)0)->_image_private) >= sizeof(arm_2d_tile_t),
               "image private storage is too small for arm_2d_tile_t");
_Static_assert(sizeof(((tinyui_image_source_t *)0)->_mask_private) >= sizeof(arm_2d_tile_t),
               "mask private storage is too small for arm_2d_tile_t");

static void s_init_tile(arm_2d_tile_t *tile,
                        uint16_t width,
                        uint16_t height,
                        const void *pixels)
{
    memset(tile, 0, sizeof(*tile));
    tile->tRegion.tSize.iWidth = (int16_t)width;
    tile->tRegion.tSize.iHeight = (int16_t)height;
    tile->pchBuffer = (void *)pixels;
}

static void s_set_image_view(tinyui_image_source_t *source,
                             arm_2d_tile_t *tile,
                             uint32_t stride)
{
    source->width = (uint16_t)tile->tRegion.tSize.iWidth;
    source->height = (uint16_t)tile->tRegion.tSize.iHeight;
    source->stride = stride;
    source->pixels = (const uint16_t *)tile->pchBuffer;
}

static void s_set_mask_view(tinyui_image_source_t *source,
                            arm_2d_tile_t *tile,
                            uint32_t stride)
{
    source->mask = (const uint8_t *)tile->pchBuffer;
    source->mask_stride = stride;
}

arm_2d_font_t *tinyui_resolve_ld_font(const struct tinyui_font *font,
                                      int default_size)
{
    int size = default_size;

    if (font != 0 && font->kind == TINYUI_FONT_KIND_VRES && font->vres_addr != 0) {
        return (arm_2d_font_t *)ldBaseGetVresFont(font->vres_addr);
    }

    if (font != 0 && font->family != 0 && font->size > 0) {
        size = font->size;
        if (strcmp(font->family, "Arial") == 0) {
            return (arm_2d_font_t *)(size >= 16 ? FONT_ARIAL_16_A8 : FONT_ARIAL_12);
        }
        if (strcmp(font->family, "Sans") == 0 && size >= 20) {
            return (arm_2d_font_t *)&ARM_2D_FONT_16x24;
        }
        return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
    }

    if (size >= 16) {
        return (arm_2d_font_t *)FONT_ARIAL_16_A8;
    }
    if (size >= 12) {
        return (arm_2d_font_t *)FONT_ARIAL_12;
    }
    return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
}

int tinyui_ld_font_is_static(const arm_2d_font_t *font)
{
    return font == (const arm_2d_font_t *)FONT_ARIAL_12
        || font == (const arm_2d_font_t *)FONT_ARIAL_16_A8
        || font == (const arm_2d_font_t *)&ARM_2D_FONT_6x8
        || font == (const arm_2d_font_t *)&ARM_2D_FONT_16x24;
}

tinyui_result_t tinyui_image_source_from_rgb565(const uint16_t *pixels,
                                                uint16_t width,
                                                uint16_t height,
                                                uint32_t stride,
                                                const uint8_t *mask,
                                                uint32_t mask_stride,
                                                tinyui_image_source_t *out)
{
    if (out == 0 || pixels == 0 || width == 0 || height == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    if (stride < (uint32_t)width * sizeof(uint16_t)
        || (mask != 0 && mask_stride < width)) {
        return TINYUI_ERROR_OUT_OF_RANGE;
    }

    memset(out, 0, sizeof(*out));
    out->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    out->width = width;
    out->height = height;
    out->stride = stride;
    out->pixels = pixels;
    out->mask = mask;
    out->mask_stride = mask_stride;
    s_init_tile(tinyui_image_source_get_image_tile(out), width, height, pixels);
    s_set_image_view(out, tinyui_image_source_get_image_tile(out), stride);
    if (mask != 0) {
        s_init_tile(tinyui_image_source_get_mask_tile(out), width, height, mask);
        s_set_mask_view(out, tinyui_image_source_get_mask_tile(out), mask_stride);
    }
    return TINYUI_OK;
}

tinyui_result_t tinyui_image_source_from_vres(uint32_t address,
                                              tinyui_image_source_t *out)
{
    arm_2d_tile_t *tile;

    if (address == 0 || out == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    tile = (arm_2d_tile_t *)ldBaseGetVresImage(address);
    if (tile == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));
    out->kind = TINYUI_IMAGE_SOURCE_VRES;
    s_set_image_view(out, tile, (uint32_t)tile->tRegion.tSize.iWidth * sizeof(uint16_t));
    return TINYUI_OK;
}

tinyui_result_t tinyui_image_source_from_builtin(tinyui_builtin_image_t image,
                                                  tinyui_image_source_t *out)
{
    arm_2d_tile_t *image_tile = 0;
    arm_2d_tile_t *mask_tile = 0;

    if (out == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    switch (image) {
    case TINYUI_BUILTIN_IMAGE_LETTER_PAPER:
        image_tile = (arm_2d_tile_t *)IMAGE_LETTER_PAPER_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_KEY_RELEASE:
        image_tile = (arm_2d_tile_t *)IMAGE_KEYRELEASE_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_KEYRELEASE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_KEY_PRESS:
        image_tile = (arm_2d_tile_t *)IMAGE_KEYPRESS_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_KEYPRESS_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_PROGRESS_BG:
        image_tile = (arm_2d_tile_t *)IMAGE_PROGRESSBARBG_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_PROGRESS_FG:
        image_tile = (arm_2d_tile_t *)IMAGE_PROGRESSBARFG_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_SLIDER_BG:
        image_tile = (arm_2d_tile_t *)IMAGE_SLIDER_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_SLIDER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR:
        image_tile = (arm_2d_tile_t *)IMAGE_INDICATOR_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_INDICATOR_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_WEATHER:
        image_tile = (arm_2d_tile_t *)IMAGE_WEATHER_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_WEATHER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_NOTE:
        image_tile = (arm_2d_tile_t *)IMAGE_NOTE_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_NOTE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_BOOK:
        image_tile = (arm_2d_tile_t *)IMAGE_BOOK_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_BOOK_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_CHART:
        image_tile = (arm_2d_tile_t *)IMAGE_CHART_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_CHART_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_GAUGE_BG:
        image_tile = (arm_2d_tile_t *)IMAGE_GAUGE_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_GAUGE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_GAUGE_POINTER:
        image_tile = (arm_2d_tile_t *)IMAGE_GAUGEPOINTER_PNG;
        mask_tile = (arm_2d_tile_t *)IMAGE_GAUGEPOINTER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_ARC_QUARTER:
        image_tile = (arm_2d_tile_t *)IMAGE_ARC_QUARTER_PNG_Mask;
        mask_tile = (arm_2d_tile_t *)IMAGE_ARC_QUARTER_MASK_PNG_Mask;
        break;
    default:
        return TINYUI_ERROR_OUT_OF_RANGE;
    }
    if (image_tile == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));
    out->kind = TINYUI_IMAGE_SOURCE_BUILTIN;
    s_set_image_view(out,
                     image_tile,
                     (uint32_t)image_tile->tRegion.tSize.iWidth * sizeof(uint16_t));
    if (mask_tile != 0) {
        s_set_mask_view(out,
                        mask_tile,
                        (uint32_t)mask_tile->tRegion.tSize.iWidth);
    }
    return TINYUI_OK;
}

tinyui_result_t tinyui_font_from_builtin(tinyui_builtin_font_t builtin,
                                         tinyui_font_t *out)
{
    if (out == 0 || builtin > TINYUI_FONT_ARIAL_16_A8) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));
#ifdef TINYUI_WIDGET_H
    out->kind = TINYUI_FONT_KIND_FAMILY;
    switch (builtin) {
    case TINYUI_FONT_6X8:
        out->family = "Sans";
        out->size = 8;
        break;
    case TINYUI_FONT_16X24:
        out->family = "Sans";
        out->size = 24;
        break;
    case TINYUI_FONT_ARIAL_12:
        out->family = "Arial";
        out->size = 12;
        break;
    case TINYUI_FONT_ARIAL_16_A8:
        out->family = "Arial";
        out->size = 16;
        break;
    }
#else
    out->kind = TINYUI_FONT_KIND_BUILTIN;
    out->value.builtin = builtin;
#endif
    return TINYUI_OK;
}

tinyui_result_t tinyui_font_from_vres(uint32_t address, tinyui_font_t *out)
{
    if (address == 0 || out == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));
#ifdef TINYUI_WIDGET_H
    out->kind = TINYUI_FONT_KIND_VRES;
    out->vres_addr = address;
#else
    out->kind = TINYUI_FONT_KIND_VRES;
    out->value.vres_address = address;
#endif
    return TINYUI_OK;
}

void tinyui_image_source_deinit(tinyui_image_source_t *source)
{
    if (source != 0) {
        memset(source, 0, sizeof(*source));
    }
}

void tinyui_font_deinit(tinyui_font_t *font)
{
    if (font != 0) {
        memset(font, 0, sizeof(*font));
    }
}
