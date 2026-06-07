#include "picoui/picoui.h"
#include "../../../examples/common/Arm-2D/Library/Include/arm_2d_types.h"
#include "internal.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

int picoui_native_image_render_buffer(const struct picoui_image_source *source,
                                      unsigned int mask_color,
                                      unsigned int *buffer,
                                      int width,
                                      int height,
                                      struct picoui_rect *dirty_rect);

static void test_argb_tile_render_writes_expected_pixels(void)
{
    unsigned int tile_pixels[4] = {
        0xFF112233U,
        0xFF445566U,
        0xFF778899U,
        0xFFABCDEFU,
    };
    arm_2d_tile_t tile = {
        .tInfo = {
            .bIsRoot = true,
            .bHasEnforcedColour = true,
            .tColourInfo = {
                .chScheme = ARM_2D_COLOUR_32BIT,
            },
        },
        .tRegion = {
            .tSize = {
                .iWidth = 2,
                .iHeight = 2,
            },
        },
        .pwBuffer = tile_pixels,
    };
    struct picoui_image_source source = {
        .img_tile = &tile,
    };
    unsigned int buffer[4] = {0};
    struct picoui_rect dirty_rect = {-1, -1, 0, 0};

    assert(picoui_native_image_render_buffer(&source, 0, buffer, 2, 2, &dirty_rect) == 0);
    assert(buffer[0] == tile_pixels[0]);
    assert(buffer[1] == tile_pixels[1]);
    assert(buffer[2] == tile_pixels[2]);
    assert(buffer[3] == tile_pixels[3]);
    assert(dirty_rect.x == 0);
    assert(dirty_rect.y == 0);
    assert(dirty_rect.width == 2);
    assert(dirty_rect.height == 2);
}

static void test_masked_tile_render_applies_mask_color(void)
{
    unsigned int tile_pixels[4] = {
        0xFF010203U,
        0xFF040506U,
        0xFF070809U,
        0xFF0A0B0CU,
    };
    unsigned int mask_pixels[4] = {
        0x00000000U,
        0xFFFFFFFFU,
        0xFFFFFFFFU,
        0x00000000U,
    };
    arm_2d_tile_t tile = {
        .tInfo = {
            .bIsRoot = true,
            .bHasEnforcedColour = true,
            .tColourInfo = {
                .chScheme = ARM_2D_COLOUR_32BIT,
            },
        },
        .tRegion = {
            .tSize = {
                .iWidth = 2,
                .iHeight = 2,
            },
        },
        .pwBuffer = tile_pixels,
    };
    arm_2d_tile_t mask = {
        .tInfo = {
            .bIsRoot = true,
            .bHasEnforcedColour = true,
            .tColourInfo = {
                .chScheme = ARM_2D_COLOUR_32BIT,
            },
        },
        .tRegion = {
            .tSize = {
                .iWidth = 2,
                .iHeight = 2,
            },
        },
        .pwBuffer = mask_pixels,
    };
    struct picoui_image_source source = {
        .img_tile = &tile,
        .mask_tile = &mask,
    };
    unsigned int buffer[4] = {0};
    struct picoui_rect dirty_rect = {-1, -1, 0, 0};

    assert(picoui_native_image_render_buffer(&source, 0x00336699U, buffer, 2, 2, &dirty_rect) == 0);
    assert(buffer[0] == tile_pixels[0]);
    assert(buffer[1] == 0xFF336699U);
    assert(buffer[2] == 0xFF336699U);
    assert(buffer[3] == tile_pixels[3]);
    assert(dirty_rect.width == 2);
    assert(dirty_rect.height == 2);
}

static void test_invalid_source_fails_without_writing_buffer(void)
{
    unsigned int buffer[4] = {
        0xAAAAAAAAU,
        0xBBBBBBBBU,
        0xCCCCCCCCU,
        0xDDDDDDDDU,
    };
    unsigned int snapshot[4];
    struct picoui_rect dirty_rect = {7, 7, 7, 7};
    struct picoui_image_source invalid_source = {0};

    memcpy(snapshot, buffer, sizeof(buffer));
    assert(picoui_native_image_render_buffer(&invalid_source, 0x00112233U, buffer, 2, 2, &dirty_rect) == -1);
    assert(memcmp(buffer, snapshot, sizeof(buffer)) == 0);
}

int main(void)
{
    test_argb_tile_render_writes_expected_pixels();
    test_masked_tile_render_applies_mask_color();
    test_invalid_source_fails_without_writing_buffer();
    return 0;
}
