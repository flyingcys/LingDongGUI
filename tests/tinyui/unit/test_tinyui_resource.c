/*
 * TinyUI M3 Task 7 — image/font value descriptor unit tests.
 *
 * Covers RGB565 borrow + private tile placement, builtin mapping, VRES
 * acquire/release ownership, ABI budgets, and zero-allocation factories
 * (except VRES which reuses ldBaseGetVres* once per init).
 */

#include "tinyui.h"
#include "internal.h"
#include "../../../examples/common/Arm-2D/Library/Include/arm_2d.h"
#include "../../../examples/common/demo/widget/images/uiImages.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"
#include "../../../src/gui/ldBase.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_a1_font_t ARM_2D_FONT_16x24;

_Static_assert(sizeof(((tinyui_image_source_t *)0)->_image_private) >= sizeof(arm_2d_tile_t),
               "image private storage is too small for arm_2d_tile_t");
_Static_assert(sizeof(((tinyui_image_source_t *)0)->_mask_private) >= sizeof(arm_2d_tile_t),
               "mask private storage is too small for arm_2d_tile_t");
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)
_Static_assert(sizeof(tinyui_image_source_t) <= 80,
               "image descriptor exceeds 32-bit ABI budget");
_Static_assert(sizeof(tinyui_font_t) <= 16,
               "font descriptor exceeds 32-bit ABI budget");
#endif

static void test_rgb565_borrows_pixels_and_builds_private_tile(void)
{
    uint16_t pixels[4] = { 0xF800U, 0x07E0U, 0x001FU, 0xFFFFU };
    uint8_t mask[4] = { 0xFFU, 0x80U, 0x40U, 0x00U };
    tinyui_image_source_t source;
    arm_2d_tile_t *image_tile;
    arm_2d_tile_t *mask_tile;
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;

    tinyui_test_allocator_reset();
    before = tinyui_test_allocator_snapshot();

    assert(tinyui_image_source_from_rgb565(pixels, 2, 2, 4, mask, 2, &source) == TINYUI_OK);
    after = tinyui_test_allocator_snapshot();
    assert(after.alloc_calls == before.alloc_calls);
    assert(after.calloc_calls == before.calloc_calls);
    assert(after.realloc_calls == before.realloc_calls);

    assert(source.kind == TINYUI_IMAGE_SOURCE_RGB565_MEMORY);
    assert(source.width == 2);
    assert(source.height == 2);
    assert(source.stride == 4);
    assert(source.pixels == pixels);
    assert(source.mask == mask);
    assert(source.mask_stride == 2);

    image_tile = tinyui_image_source_get_image_tile(&source);
    mask_tile = tinyui_image_source_get_mask_tile(&source);
    assert(image_tile != 0);
    assert(mask_tile != 0);
    assert(image_tile->tRegion.tSize.iWidth == 2);
    assert(image_tile->tRegion.tSize.iHeight == 2);
    assert(image_tile->pchBuffer == (uint8_t *)(void *)pixels);
    assert(mask_tile->pchBuffer == mask);

    pixels[0] = 0x0000U;
    assert(source.pixels[0] == 0x0000U);

    tinyui_image_source_deinit(&source);
    assert(source.kind == TINYUI_IMAGE_SOURCE_EMPTY);
    assert(source.pixels == 0);
    assert(source.mask == 0);
    tinyui_image_source_deinit(&source);
}

static void test_rgb565_rejects_bad_args_and_strides(void)
{
    uint16_t pixels[4] = { 0 };
    uint8_t mask[4] = { 0 };
    tinyui_image_source_t source;

    assert(tinyui_image_source_from_rgb565(0, 2, 2, 4, 0, 0, &source)
           == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_image_source_from_rgb565(pixels, 0, 2, 4, 0, 0, &source)
           == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_image_source_from_rgb565(pixels, 2, 0, 4, 0, 0, &source)
           == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_image_source_from_rgb565(pixels, 2, 2, 4, 0, 0, 0)
           == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_image_source_from_rgb565(pixels, 2, 2, 3, 0, 0, &source)
           == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_image_source_from_rgb565(pixels, 2, 2, 4, mask, 1, &source)
           == TINYUI_ERROR_OUT_OF_RANGE);
}

static void test_builtin_image_maps_static_resources(void)
{
    tinyui_image_source_t source;
    arm_2d_tile_t *image_tile;
    arm_2d_tile_t *mask_tile;
    const arm_2d_tile_t *legacy_img = IMAGE_ARC_QUARTER_PNG_Mask;
    const arm_2d_tile_t *legacy_mask = IMAGE_ARC_QUARTER_MASK_PNG_Mask;

    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, &source)
           == TINYUI_OK);
    assert(source.kind == TINYUI_IMAGE_SOURCE_BUILTIN);
    image_tile = tinyui_image_source_get_image_tile(&source);
    assert(image_tile != 0);
    assert(image_tile->pchBuffer == ((const arm_2d_tile_t *)IMAGE_LETTER_PAPER_BMP)->pchBuffer);
    tinyui_image_source_deinit(&source);

    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_KEY_RELEASE, &source)
           == TINYUI_OK);
    assert(source.kind == TINYUI_IMAGE_SOURCE_BUILTIN);
    image_tile = tinyui_image_source_get_image_tile(&source);
    mask_tile = tinyui_image_source_get_mask_tile(&source);
    assert(image_tile != 0);
    assert(mask_tile != 0);
    assert(image_tile->pchBuffer == ((const arm_2d_tile_t *)IMAGE_KEYRELEASE_PNG)->pchBuffer);
    assert(mask_tile->pchBuffer == ((const arm_2d_tile_t *)IMAGE_KEYRELEASE_PNG_Mask)->pchBuffer);
    tinyui_image_source_deinit(&source);

    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &source)
           == TINYUI_OK);
    image_tile = tinyui_image_source_get_image_tile(&source);
    mask_tile = tinyui_image_source_get_mask_tile(&source);
    assert(image_tile->tRegion.tSize.iWidth == legacy_img->tRegion.tSize.iWidth);
    assert(image_tile->tRegion.tSize.iHeight == legacy_img->tRegion.tSize.iHeight);
    assert(image_tile->pchBuffer == legacy_img->pchBuffer);
    assert(mask_tile->pchBuffer == legacy_mask->pchBuffer);
    tinyui_image_source_deinit(&source);
    assert(source.kind == TINYUI_IMAGE_SOURCE_EMPTY);

    assert(tinyui_image_source_from_builtin((tinyui_builtin_image_t)0, &source)
           == TINYUI_ERROR_OUT_OF_RANGE
           || tinyui_image_source_from_builtin((tinyui_builtin_image_t)0, &source)
                  == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_image_source_from_builtin((tinyui_builtin_image_t)9999, &source)
           == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, 0)
           == TINYUI_ERROR_INVALID_ARG);
}

static void test_vres_image_acquire_release_once_when_available(void)
{
    tinyui_image_source_t source;
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after_init;
    struct tinyui_test_allocator_stats after_deinit;

    assert(tinyui_image_source_from_vres(0, &source) == TINYUI_ERROR_INVALID_ARG);

    tinyui_test_allocator_reset();
    before = tinyui_test_allocator_snapshot();
    if (tinyui_image_source_from_vres(0x1000U, &source) != TINYUI_OK) {
        after_init = tinyui_test_allocator_snapshot();
        assert(after_init.alloc_calls == before.alloc_calls);
        assert(after_init.calloc_calls == before.calloc_calls);
        return;
    }

    after_init = tinyui_test_allocator_snapshot();
    assert(source.kind == TINYUI_IMAGE_SOURCE_VRES);
    assert(tinyui_image_source_get_image_tile(&source) != 0);
    assert((after_init.alloc_calls + after_init.calloc_calls)
           == (before.alloc_calls + before.calloc_calls + 1U));

    tinyui_image_source_deinit(&source);
    after_deinit = tinyui_test_allocator_snapshot();
    assert(source.kind == TINYUI_IMAGE_SOURCE_EMPTY);
    assert(after_deinit.free_calls == after_init.free_calls + 1U);

    tinyui_image_source_deinit(&source);
    assert(tinyui_test_allocator_snapshot().free_calls == after_deinit.free_calls);
}

static void test_font_builtin_and_vres_value_descriptors(void)
{
    tinyui_font_t font;
    arm_2d_font_t *resolved;

    assert(tinyui_font_from_builtin(TINYUI_FONT_6X8, &font) == TINYUI_OK);
    assert(font.kind == TINYUI_FONT_KIND_BUILTIN);
    assert(font.value.builtin == TINYUI_FONT_6X8);
    resolved = tinyui_resolve_ld_font(&font, 12);
    assert(resolved == (arm_2d_font_t *)&ARM_2D_FONT_6x8);

    assert(tinyui_font_from_builtin(TINYUI_FONT_16X24, &font) == TINYUI_OK);
    assert(tinyui_resolve_ld_font(&font, 12) == (arm_2d_font_t *)&ARM_2D_FONT_16x24);

    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_12, &font) == TINYUI_OK);
    assert(tinyui_resolve_ld_font(&font, 12) == (arm_2d_font_t *)FONT_ARIAL_12);

    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_16_A8, &font) == TINYUI_OK);
    assert(tinyui_resolve_ld_font(&font, 12) == (arm_2d_font_t *)FONT_ARIAL_16_A8);

    assert(tinyui_font_from_builtin((tinyui_builtin_font_t)99, &font)
           == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_font_from_vres(0, &font) == TINYUI_ERROR_INVALID_ARG);

    assert(tinyui_font_from_vres(0x2000U, &font) == TINYUI_OK);
    assert(font.kind == TINYUI_FONT_KIND_VRES);
    assert(font.value.vres_address == 0x2000U);
    (void)tinyui_resolve_ld_font(&font, 12);

    tinyui_font_deinit(&font);
    assert(font.value.vres_address == 0);
    tinyui_font_deinit(&font);
}

static void test_argb8888_is_not_a_supported_image_memory_format(void)
{
    const char *header = tinyui_test_repo_path_from_file(__FILE__,
                                                         "tinyui/include/resource/image_source.h");
    const char *source = tinyui_test_repo_path_from_file(__FILE__,
                                                         "tinyui/src/core/resource.c");

    assert(tinyui_test_source_contains(header, "tinyui_image_source_from_rgb565"));
    assert(!tinyui_test_source_contains(header, "from_argb"));
    assert(!tinyui_test_source_contains(header, "ARGB8888"));
    assert(tinyui_test_source_contains(source, "TINYUI_ERROR_NOT_SUPPORTED")
           || tinyui_test_source_contains(source, "RGB565"));
    (void)TINYUI_ERROR_NOT_SUPPORTED;
}

int main(void)
{
    test_rgb565_borrows_pixels_and_builds_private_tile();
    test_rgb565_rejects_bad_args_and_strides();
    test_builtin_image_maps_static_resources();
    test_vres_image_acquire_release_once_when_available();
    test_font_builtin_and_vres_value_descriptors();
    test_argb8888_is_not_a_supported_image_memory_format();
    return 0;
}
