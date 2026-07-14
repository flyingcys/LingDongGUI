#ifndef TINYUI_IMAGE_SOURCE_H
#define TINYUI_IMAGE_SOURCE_H

#include "core/result.h"

#include <stdint.h>

typedef enum tinyui_image_source_kind {
    TINYUI_IMAGE_SOURCE_EMPTY,
    TINYUI_IMAGE_SOURCE_RGB565_MEMORY,
    TINYUI_IMAGE_SOURCE_BUILTIN,
    TINYUI_IMAGE_SOURCE_VRES,
} tinyui_image_source_kind_t;

typedef enum tinyui_builtin_image {
    TINYUI_BUILTIN_IMAGE_LETTER_PAPER = 1,
    TINYUI_BUILTIN_IMAGE_KEY_RELEASE,
    TINYUI_BUILTIN_IMAGE_KEY_PRESS,
    TINYUI_BUILTIN_IMAGE_PROGRESS_BG,
    TINYUI_BUILTIN_IMAGE_PROGRESS_FG,
    TINYUI_BUILTIN_IMAGE_SLIDER_BG,
    TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR,
    TINYUI_BUILTIN_IMAGE_WEATHER,
    TINYUI_BUILTIN_IMAGE_NOTE,
    TINYUI_BUILTIN_IMAGE_BOOK,
    TINYUI_BUILTIN_IMAGE_CHART,
    TINYUI_BUILTIN_IMAGE_GAUGE_BG,
    TINYUI_BUILTIN_IMAGE_GAUGE_POINTER,
    TINYUI_BUILTIN_IMAGE_ARC_QUARTER,
} tinyui_builtin_image_t;

typedef struct tinyui_image_source tinyui_image_source_t;

struct tinyui_image_source {
    tinyui_image_source_kind_t kind;
    uint16_t width;
    uint16_t height;
    uint32_t stride;
    const uint16_t *pixels;
    const uint8_t *mask;
    uint32_t mask_stride;
    uintptr_t _image_private[6];
    uintptr_t _mask_private[6];
};

tinyui_result_t tinyui_image_source_from_rgb565(const uint16_t *pixels,
                                                uint16_t width,
                                                uint16_t height,
                                                uint32_t stride,
                                                const uint8_t *mask,
                                                uint32_t mask_stride,
                                                tinyui_image_source_t *out);

tinyui_result_t tinyui_image_source_from_builtin(tinyui_builtin_image_t image,
                                                  tinyui_image_source_t *out);

tinyui_result_t tinyui_image_source_from_vres(uint32_t address,
                                              tinyui_image_source_t *out);

void tinyui_image_source_deinit(tinyui_image_source_t *source);

#endif
