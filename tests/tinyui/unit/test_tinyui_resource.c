#include "tinyui.h"
#include "../../../examples/common/Arm-2D/Library/Include/arm_2d.h"

#include <assert.h>

static void test_builtin_image_source_success(void)
{
    struct tinyui_image_source source = {0};

    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, &source) == 0);
    assert(source.img_tile != 0);
    assert(source.mask_tile == 0);
    assert(source.kind == TINYUI_IMAGE_SOURCE_KIND_BUILTIN);
    assert(source.vres_addr == 0);
}

static void test_builtin_masked_image_source_success(void)
{
    struct tinyui_image_source source = {0};

    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_KEY_RELEASE, &source) == 0);
    assert(source.img_tile != 0);
    assert(source.mask_tile != 0);
    assert(source.kind == TINYUI_IMAGE_SOURCE_KIND_BUILTIN);
    assert(source.vres_addr == 0);
}

static void test_builtin_image_source_rejects_invalid_args(void)
{
    struct tinyui_image_source source = {0};

    assert(tinyui_image_source_from_builtin(0, &source) == -1);
    assert(tinyui_image_source_from_builtin((enum tinyui_builtin_image)9999, &source) == -1);
    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, 0) == -1);
}

static void test_builtin_image_source_destroy_clears_without_owning_tiles(void)
{
    struct tinyui_image_source source = {0};
    const arm_2d_tile_t *tile;

    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &source) == 0);
    assert(source.img_tile != 0);
    assert(source.mask_tile != 0);
    assert(source.kind == TINYUI_IMAGE_SOURCE_KIND_BUILTIN);
    tile = (const arm_2d_tile_t *)source.img_tile;
    assert(tile->tRegion.tSize.iWidth == 62);
    assert(tile->tRegion.tSize.iHeight == 62);

    tinyui_image_source_destroy(&source);
    assert(source.img_tile == 0);
    assert(source.mask_tile == 0);
    assert(source.kind == TINYUI_IMAGE_SOURCE_KIND_EMPTY);
    assert(source.vres_addr == 0);

    tinyui_image_source_destroy(&source);
    assert(source.img_tile == 0);
    assert(source.mask_tile == 0);
    assert(source.kind == TINYUI_IMAGE_SOURCE_KIND_EMPTY);
    assert(source.vres_addr == 0);
}

static void test_external_image_source_destroy_does_not_own_tiles(void)
{
    int image_tile;
    int mask_tile;
    struct tinyui_image_source source = {
        .img_tile = &image_tile,
        .mask_tile = &mask_tile,
        .kind = TINYUI_IMAGE_SOURCE_KIND_EXTERNAL,
        .vres_addr = 0,
    };

    tinyui_image_source_destroy(&source);
    assert(source.img_tile == 0);
    assert(source.mask_tile == 0);
    assert(source.kind == TINYUI_IMAGE_SOURCE_KIND_EMPTY);
    assert(source.vres_addr == 0);
}

int main(void)
{
    test_builtin_image_source_success();
    test_builtin_masked_image_source_success();
    test_builtin_image_source_rejects_invalid_args();
    test_builtin_image_source_destroy_clears_without_owning_tiles();
    test_external_image_source_destroy_does_not_own_tiles();
    return 0;
}
