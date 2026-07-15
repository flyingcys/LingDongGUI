/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

/* Canonical image/font value descriptors (M3 Task 7). Include public resource
 * headers before internal/legacy so factories and resolve share one layout. */
#include "resource/font.h"
#include "resource/image_source.h"

#include "internal.h"
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

/* VRES acquire handle lives in the last mask-private slot so the image
 * private region can hold a full placed arm_2d_tile_t view. */
#define TINYUI_IMAGE_VRES_HANDLE_SLOT \
    ((sizeof(((tinyui_image_source_t *)0)->_mask_private) / sizeof(uintptr_t)) - 1U)

static void s_place_tile(arm_2d_tile_t *dst, const arm_2d_tile_t *src)
{
    if (dst == 0) {
        return;
    }
    if (src == 0) {
        memset(dst, 0, sizeof(*dst));
        return;
    }
    memcpy(dst, src, sizeof(*dst));
}

static void s_init_rgb565_tile(arm_2d_tile_t *tile,
                               uint16_t width,
                               uint16_t height,
                               const void *pixels)
{
    memset(tile, 0, sizeof(*tile));
    tile->tRegion.tSize.iWidth = (int16_t)width;
    tile->tRegion.tSize.iHeight = (int16_t)height;
    tile->tInfo.bIsRoot = true;
    tile->tInfo.bHasEnforcedColour = true;
    tile->tInfo.tColourInfo.chScheme = ARM_2D_COLOUR_RGB565;
    tile->pchBuffer = (uint8_t *)(uintptr_t)pixels;
}

static void s_init_a8_mask_tile(arm_2d_tile_t *tile,
                                uint16_t width,
                                uint16_t height,
                                const void *mask)
{
    memset(tile, 0, sizeof(*tile));
    tile->tRegion.tSize.iWidth = (int16_t)width;
    tile->tRegion.tSize.iHeight = (int16_t)height;
    tile->tInfo.bIsRoot = true;
    tile->tInfo.bHasEnforcedColour = true;
    tile->tInfo.tColourInfo.chScheme = ARM_2D_COLOUR_8BIT;
    tile->pchBuffer = (uint8_t *)(uintptr_t)mask;
}

static void s_set_image_view_from_tile(tinyui_image_source_t *source,
                                       const arm_2d_tile_t *tile,
                                       uint32_t stride)
{
    source->width = (uint16_t)tile->tRegion.tSize.iWidth;
    source->height = (uint16_t)tile->tRegion.tSize.iHeight;
    source->stride = stride;
    source->pixels = (const uint16_t *)(const void *)tile->pchBuffer;
}

static void s_set_mask_view_from_tile(tinyui_image_source_t *source,
                                      const arm_2d_tile_t *tile,
                                      uint32_t stride)
{
    source->mask = (const uint8_t *)(const void *)tile->pchBuffer;
    source->mask_stride = stride;
}

static arm_2d_font_t *s_builtin_font(tinyui_builtin_font_t builtin)
{
    switch (builtin) {
    case TINYUI_FONT_6X8:
        return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
    case TINYUI_FONT_16X24:
        return (arm_2d_font_t *)&ARM_2D_FONT_16x24;
    case TINYUI_FONT_ARIAL_12:
        return (arm_2d_font_t *)FONT_ARIAL_12;
    case TINYUI_FONT_ARIAL_16_A8:
        return (arm_2d_font_t *)FONT_ARIAL_16_A8;
    default:
        return 0;
    }
}

static arm_2d_font_t *s_default_font(int size)
{
    if (size >= 16) {
        return (arm_2d_font_t *)FONT_ARIAL_16_A8;
    }
    if (size >= 12) {
        return (arm_2d_font_t *)FONT_ARIAL_12;
    }
    return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
}

arm_2d_font_t *tinyui_resolve_ld_font(const struct tinyui_font *font,
                                      int default_size)
{
    if (font == 0) {
        return s_default_font(default_size);
    }

    if (font->kind == TINYUI_FONT_KIND_VRES) {
        if (font->value.vres_address == 0) {
            return 0;
        }
        return (arm_2d_font_t *)ldBaseGetVresFont(font->value.vres_address);
    }

    if (font->kind == TINYUI_FONT_KIND_BUILTIN) {
        arm_2d_font_t *resolved = s_builtin_font(font->value.builtin);
        if (resolved != 0) {
            return resolved;
        }
    }

    return s_default_font(default_size);
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
    arm_2d_tile_t *image_tile;
    arm_2d_tile_t *mask_tile;

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

    image_tile = tinyui_image_source_get_image_tile(out);
    s_init_rgb565_tile(image_tile, width, height, pixels);
    if (mask != 0) {
        mask_tile = tinyui_image_source_get_mask_tile(out);
        s_init_a8_mask_tile(mask_tile, width, height, mask);
    }
    return TINYUI_OK;
}

tinyui_result_t tinyui_image_source_from_vres(uint32_t address,
                                              tinyui_image_source_t *out)
{
    arm_2d_vres_t *vres;
    arm_2d_tile_t *image_tile;

    if (address == 0 || out == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    vres = ldBaseGetVresImage(address);
    if (vres == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    memset(out, 0, sizeof(*out));
    out->kind = TINYUI_IMAGE_SOURCE_VRES;
    image_tile = tinyui_image_source_get_image_tile(out);
    s_place_tile(image_tile, &vres->tTile);
    s_set_image_view_from_tile(out,
                               image_tile,
                               (uint32_t)image_tile->tRegion.tSize.iWidth * sizeof(uint16_t));
    out->_mask_private[TINYUI_IMAGE_VRES_HANDLE_SLOT] = (uintptr_t)vres;
    return TINYUI_OK;
}

tinyui_result_t tinyui_image_source_from_builtin(tinyui_builtin_image_t image,
                                                  tinyui_image_source_t *out)
{
    const arm_2d_tile_t *image_tile = 0;
    const arm_2d_tile_t *mask_tile = 0;
    arm_2d_tile_t *dst_image;
    arm_2d_tile_t *dst_mask;

    if (out == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    switch (image) {
    case TINYUI_BUILTIN_IMAGE_LETTER_PAPER:
        image_tile = (const arm_2d_tile_t *)IMAGE_LETTER_PAPER_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_KEY_RELEASE:
        image_tile = (const arm_2d_tile_t *)IMAGE_KEYRELEASE_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_KEYRELEASE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_KEY_PRESS:
        image_tile = (const arm_2d_tile_t *)IMAGE_KEYPRESS_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_KEYPRESS_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_PROGRESS_BG:
        image_tile = (const arm_2d_tile_t *)IMAGE_PROGRESSBARBG_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_PROGRESS_FG:
        image_tile = (const arm_2d_tile_t *)IMAGE_PROGRESSBARFG_BMP;
        break;
    case TINYUI_BUILTIN_IMAGE_SLIDER_BG:
        image_tile = (const arm_2d_tile_t *)IMAGE_SLIDER_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_SLIDER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR:
        image_tile = (const arm_2d_tile_t *)IMAGE_INDICATOR_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_INDICATOR_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_WEATHER:
        image_tile = (const arm_2d_tile_t *)IMAGE_WEATHER_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_WEATHER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_NOTE:
        image_tile = (const arm_2d_tile_t *)IMAGE_NOTE_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_NOTE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_BOOK:
        image_tile = (const arm_2d_tile_t *)IMAGE_BOOK_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_BOOK_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_CHART:
        image_tile = (const arm_2d_tile_t *)IMAGE_CHART_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_CHART_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_GAUGE_BG:
        image_tile = (const arm_2d_tile_t *)IMAGE_GAUGE_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_GAUGE_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_GAUGE_POINTER:
        image_tile = (const arm_2d_tile_t *)IMAGE_GAUGEPOINTER_PNG;
        mask_tile = (const arm_2d_tile_t *)IMAGE_GAUGEPOINTER_PNG_Mask;
        break;
    case TINYUI_BUILTIN_IMAGE_ARC_QUARTER:
        image_tile = (const arm_2d_tile_t *)IMAGE_ARC_QUARTER_PNG_Mask;
        mask_tile = (const arm_2d_tile_t *)IMAGE_ARC_QUARTER_MASK_PNG_Mask;
        break;
    default:
        return TINYUI_ERROR_OUT_OF_RANGE;
    }
    if (image_tile == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    memset(out, 0, sizeof(*out));
    out->kind = TINYUI_IMAGE_SOURCE_BUILTIN;
    dst_image = tinyui_image_source_get_image_tile(out);
    s_place_tile(dst_image, image_tile);
    s_set_image_view_from_tile(out,
                               dst_image,
                               (uint32_t)dst_image->tRegion.tSize.iWidth * sizeof(uint16_t));
    if (mask_tile != 0) {
        dst_mask = tinyui_image_source_get_mask_tile(out);
        s_place_tile(dst_mask, mask_tile);
        s_set_mask_view_from_tile(out,
                                  dst_mask,
                                  (uint32_t)dst_mask->tRegion.tSize.iWidth);
    }
    return TINYUI_OK;
}

tinyui_result_t tinyui_font_from_builtin(tinyui_builtin_font_t builtin,
                                         tinyui_font_t *out)
{
    if (out == 0 || s_builtin_font(builtin) == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));
    out->kind = TINYUI_FONT_KIND_BUILTIN;
    out->value.builtin = builtin;
    return TINYUI_OK;
}

tinyui_result_t tinyui_font_from_vres(uint32_t address, tinyui_font_t *out)
{
    if (address == 0 || out == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));
    out->kind = TINYUI_FONT_KIND_VRES;
    out->value.vres_address = address;
    return TINYUI_OK;
}

void tinyui_image_source_deinit(tinyui_image_source_t *source)
{
    if (source == 0) {
        return;
    }
    if (source->kind == TINYUI_IMAGE_SOURCE_VRES) {
        void *handle = (void *)source->_mask_private[TINYUI_IMAGE_VRES_HANDLE_SLOT];
        if (handle != 0) {
            ldFree(handle);
            source->_mask_private[TINYUI_IMAGE_VRES_HANDLE_SLOT] = 0;
        }
    }
    memset(source, 0, sizeof(*source));
}

void tinyui_font_deinit(tinyui_font_t *font)
{
    if (font == 0) {
        return;
    }
    memset(font, 0, sizeof(*font));
}
