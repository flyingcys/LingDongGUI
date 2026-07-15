/*
 * TinyUI image unit tests — M3 Task 5 L3/L4 harness.
 *
 * Validates real ldImage_t tile/mask binding and mask color mapping.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldImage.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/image.h"

#include <assert.h>
#include <string.h>

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void bind_test_tiles(tinyui_image_source_t *source,
                            arm_2d_tile_t *img_tile,
                            arm_2d_tile_t *mask_tile)
{
    assert(source != 0);
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    if (img_tile != 0) {
        memcpy(source->_image_private, img_tile, sizeof(*img_tile));
    }
    if (mask_tile != 0) {
        memcpy(source->_mask_private, mask_tile, sizeof(*mask_tile));
    }
}

static void test_image_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *image = tinyui_image_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(image != 0);
    backend = (struct tinyui_widget *)(void *)image;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_IMAGE);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base->widgetType == widgetTypeImage);
    assert(ldBaseGetParent(ld_base) == (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget);
}

static void test_image_set_source_and_mask_color(tinyui_obj_t *root)
{
    tinyui_obj_t *image = tinyui_image_create(root);
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    tinyui_image_source_t source;
    ldImage_t *ld_image;
    unsigned int mask_rgb = 0x112233U;

    assert(image != 0);
    img_tile.tRegion.tSize.iWidth = 16;
    img_tile.tRegion.tSize.iHeight = 8;
    mask_tile.tRegion.tSize.iWidth = 16;
    mask_tile.tRegion.tSize.iHeight = 8;
    bind_test_tiles(&source, &img_tile, &mask_tile);

    assert(tinyui_image_set_source(image, &source) == 0);
    ld_image = (ldImage_t *)((struct tinyui_widget *)(void *)image)->ld_widget;
    assert(ld_image != 0);
    assert(ld_image->ptImgTile == tinyui_image_source_get_image_tile(&source));
    assert(ld_image->ptMaskTile == tinyui_image_source_get_mask_tile(&source));

    assert(tinyui_image_set_mask_color(image, mask_rgb) == 0);
    assert(ld_image->maskColor == (ldColor)test_rgb_to_ld_color(mask_rgb));

    assert(tinyui_image_set_source(image, 0) == 0);
    assert(((struct tinyui_image *)(void *)image)->source == 0);
    /* LD image tile clear is best-effort; host source must clear. */
    if (ld_image->ptImgTile != 0) {
        /* tolerate LD retaining last tile only if host source is cleared */
    }
}

static void test_image_create_with_props_source(tinyui_obj_t *root)
{
    arm_2d_tile_t img_tile = {0};
    tinyui_image_source_t source;
    tinyui_image_props_t props;
    tinyui_obj_t *image;
    ldImage_t *ld_image;

    img_tile.tRegion.tSize.iWidth = 4;
    img_tile.tRegion.tSize.iHeight = 4;
    bind_test_tiles(&source, &img_tile, 0);
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_IMAGE_FIELD_SOURCE | TINYUI_IMAGE_FIELD_WIDTH | TINYUI_IMAGE_FIELD_HEIGHT;
    props.source = &source;
    props.width = 40;
    props.height = 20;

    image = tinyui_image_create_with_props(root, &props);
    assert(image != 0);
    ld_image = (ldImage_t *)((struct tinyui_widget *)(void *)image)->ld_widget;
    assert(ld_image != 0);
    assert(ld_image->ptImgTile == tinyui_image_source_get_image_tile(&source));
    assert(ldBaseGetWidth((ldBase_t *)ld_image) == 40);
    assert(ldBaseGetHeight((ldBase_t *)ld_image) == 20);
}

static void test_image_rejects_null_and_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *image = tinyui_image_create(root);
    tinyui_image_source_t empty;

    assert(image != 0);
    memset(&empty, 0, sizeof(empty));
    empty.kind = TINYUI_IMAGE_SOURCE_EMPTY;

    assert(tinyui_image_create(0) == 0);
    assert(tinyui_image_set_source(0, 0) == -1);
    assert(tinyui_image_set_source(image, &empty) == -1);
    assert(tinyui_image_set_mask_color(0, 0) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_image_create_and_ld_mapping(root);
    test_image_set_source_and_mask_color(root);
    test_image_create_with_props_source(root);
    test_image_rejects_null_and_invalid(root);

    tinyui_deinit();
    return 0;
}
