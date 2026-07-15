/*
 * TinyUI progress_bar unit tests — M3 Task 2 L4 harness.
 *
 * Validates real ldProgressBar_t percent/orientation/color/source mapping,
 * bounds, props rollback, and getters that follow backend truth.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldProgressBar.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/progress_bar.h"

#include <assert.h>
#include <stdio.h>
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
        source->width = (uint16_t)img_tile->tRegion.tSize.iWidth;
        source->height = (uint16_t)img_tile->tRegion.tSize.iHeight;
        memcpy(source->_image_private, img_tile, sizeof(*img_tile));
    }
    if (mask_tile != 0) {
        memcpy(source->_mask_private, mask_tile, sizeof(*mask_tile));
    }
}

static void test_progress_bar_create_and_backend_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *bar = tinyui_progress_bar_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    ldProgressBar_t *ld_bar;

    assert(bar != 0);
    backend = (struct tinyui_widget *)(void *)bar;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_BAR);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeProgressBar);
    ld_bar = (ldProgressBar_t *)backend->ld_widget;
    assert(ld_bar->permille == 0);
    assert(ld_bar->isHorizontal == false);
    assert(ld_bar->isInverted == false);
    assert(tinyui_progress_bar_get_percent(bar) == 0);
    assert(tinyui_progress_bar_get_horizontal(bar) == 0);
    assert(tinyui_progress_bar_get_inverted(bar) == 0);
}

static void test_progress_bar_create_with_props_pushes_fields(tinyui_obj_t *root)
{
    tinyui_progress_bar_props_t props;
    tinyui_obj_t *bar;
    ldProgressBar_t *ld_bar;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_PROGRESS_BAR_FIELD_PERCENT
        | TINYUI_PROGRESS_BAR_FIELD_HORIZONTAL
        | TINYUI_PROGRESS_BAR_FIELD_INVERTED;
    props.percent = 64;
    props.horizontal = 1;
    props.inverted = 1;

    bar = tinyui_progress_bar_create_with_props(root, &props);
    assert(bar != 0);
    ld_bar = (ldProgressBar_t *)((struct tinyui_widget *)(void *)bar)->ld_widget;
    assert(ld_bar != 0);
    assert(ld_bar->permille == 640);
    assert(ld_bar->isHorizontal == true);
    assert(ld_bar->isInverted == true);
    assert(tinyui_progress_bar_get_percent(bar) == 64);
    assert(tinyui_progress_bar_get_horizontal(bar) == 1);
    assert(tinyui_progress_bar_get_inverted(bar) == 1);
}

static void test_progress_bar_percent_orientation_color_source_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *bar = tinyui_progress_bar_create(root);
    arm_2d_tile_t bg_img = {0};
    arm_2d_tile_t bg_mask = {0};
    arm_2d_tile_t fg_img = {0};
    arm_2d_tile_t fg_mask = {0};
    arm_2d_tile_t frame_img = {0};
    arm_2d_tile_t frame_mask = {0};
    tinyui_image_source_t bg_source;
    tinyui_image_source_t fg_source;
    tinyui_image_source_t frame_source;
    ldProgressBar_t *ld_bar;

    assert(bar != 0);
    ld_bar = (ldProgressBar_t *)((struct tinyui_widget *)(void *)bar)->ld_widget;
    assert(ld_bar != 0);

    assert(tinyui_progress_bar_set_percent(bar, 0) == 0);
    assert(ld_bar->permille == 0);
    assert(tinyui_progress_bar_get_percent(bar) == 0);
    assert(tinyui_progress_bar_set_percent(bar, 100) == 0);
    assert(ld_bar->permille == 1000);
    assert(tinyui_progress_bar_get_percent(bar) == 100);
    assert(tinyui_progress_bar_set_percent(bar, -1) == -1);
    assert(ld_bar->permille == 1000);
    assert(tinyui_progress_bar_set_percent(bar, 101) == -1);
    assert(ld_bar->permille == 1000);

    assert(tinyui_progress_bar_set_horizontal(bar, 1) == 0);
    assert(ld_bar->isHorizontal == true);
    assert(tinyui_progress_bar_get_horizontal(bar) == 1);
    assert(tinyui_progress_bar_set_horizontal(bar, 0) == 0);
    assert(ld_bar->isHorizontal == false);
    assert(tinyui_progress_bar_get_horizontal(bar) == 0);

    assert(tinyui_progress_bar_set_inverted(bar, 1) == 0);
    assert(ld_bar->isInverted == true);
    assert(tinyui_progress_bar_get_inverted(bar) == 1);
    assert(tinyui_progress_bar_set_inverted(bar, 0) == 0);
    assert(ld_bar->isInverted == false);

    assert(tinyui_progress_bar_set_color(bar, 0x123456U, 0xABCDEFU) == 0);
    assert(ld_bar->bgColor == (ldColor)test_rgb_to_ld_color(0x123456U));
    assert(ld_bar->fgColor == (ldColor)test_rgb_to_ld_color(0xABCDEFU));
    assert(tinyui_progress_bar_set_frame_color(bar, 0x654321U, 3) == 0);
    assert(ld_bar->frameColor == (ldColor)test_rgb_to_ld_color(0x654321U));
    assert(ld_bar->frameColorSize == 3);

    bg_img.tRegion.tSize.iWidth = 16;
    bg_img.tRegion.tSize.iHeight = 8;
    bg_mask.tRegion.tSize.iWidth = 16;
    bg_mask.tRegion.tSize.iHeight = 8;
    fg_img.tRegion.tSize.iWidth = 16;
    fg_img.tRegion.tSize.iHeight = 8;
    fg_mask.tRegion.tSize.iWidth = 16;
    fg_mask.tRegion.tSize.iHeight = 8;
    frame_img.tRegion.tSize.iWidth = 18;
    frame_img.tRegion.tSize.iHeight = 10;
    frame_mask.tRegion.tSize.iWidth = 18;
    frame_mask.tRegion.tSize.iHeight = 10;
    bind_test_tiles(&bg_source, &bg_img, &bg_mask);
    bind_test_tiles(&fg_source, &fg_img, &fg_mask);
    bind_test_tiles(&frame_source, &frame_img, &frame_mask);

    assert(tinyui_progress_bar_set_bg_source(bar, &bg_source) == 0);
    assert(ld_bar->ptBgImgTile == tinyui_image_source_get_image_tile(&bg_source));
    assert(ld_bar->ptBgMaskTile == tinyui_image_source_get_mask_tile(&bg_source));
    assert(tinyui_progress_bar_set_fg_source(bar, &fg_source) == 0);
    assert(ld_bar->ptFgImgTile == tinyui_image_source_get_image_tile(&fg_source));
    assert(ld_bar->ptFgMaskTile == tinyui_image_source_get_mask_tile(&fg_source));
    assert(tinyui_progress_bar_set_image(bar, &bg_source, &fg_source) == 0);
    assert(tinyui_progress_bar_set_frame_source(bar, &frame_source) == 0);
    assert(ld_bar->ptFrameImgTile == tinyui_image_source_get_image_tile(&frame_source));
    assert(ld_bar->ptFrameMaskTile == tinyui_image_source_get_mask_tile(&frame_source));

    ld_bar->permille = 370;
    assert(tinyui_progress_bar_get_percent(bar) == 37);
    ld_bar->isHorizontal = true;
    assert(tinyui_progress_bar_get_horizontal(bar) == 1);
    ld_bar->isInverted = true;
    assert(tinyui_progress_bar_get_inverted(bar) == 1);
}

static void test_progress_bar_props_invalid_rolls_back(tinyui_obj_t *root)
{
    tinyui_progress_bar_props_t props;
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *child_before = ldBaseGetChildList(root_ld);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_PROGRESS_BAR_FIELD_PERCENT;
    props.percent = 140;
    assert(tinyui_progress_bar_create_with_props(root, &props) == 0);
    assert(ldBaseGetChildList(root_ld) == child_before);
}

static void test_progress_bar_rejects_null_and_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *bar = tinyui_progress_bar_create(root);
    tinyui_image_source_t empty_source;

    assert(bar != 0);
    memset(&empty_source, 0, sizeof(empty_source));

    assert(tinyui_progress_bar_create(0) == 0);
    assert(tinyui_progress_bar_set_percent(0, 10) == -1);
    assert(tinyui_progress_bar_get_percent(0) == -1);
    assert(tinyui_progress_bar_set_percent(bar, -3) == -1);
    assert(tinyui_progress_bar_set_percent(bar, 130) == -1);
    assert(tinyui_progress_bar_set_horizontal(0, 1) == -1);
    assert(tinyui_progress_bar_get_horizontal(0) == -1);
    assert(tinyui_progress_bar_set_inverted(0, 1) == -1);
    assert(tinyui_progress_bar_get_inverted(0) == -1);
    assert(tinyui_progress_bar_set_color(0, 0U, 0U) == -1);
    assert(tinyui_progress_bar_set_color(bar, 0x1000000U, 0U) == -1);
    assert(tinyui_progress_bar_set_frame_color(bar, 0x1000000U, 1) == -1);
    assert(tinyui_progress_bar_set_frame_color(bar, 0U, -1) == -1);
    assert(tinyui_progress_bar_set_bg_source(0, &empty_source) == -1);
    assert(tinyui_progress_bar_set_bg_source(bar, 0) == -1);
    assert(tinyui_progress_bar_set_bg_source(bar, &empty_source) == -1);
    assert(tinyui_progress_bar_set_fg_source(bar, &empty_source) == -1);
    assert(tinyui_progress_bar_set_frame_source(bar, &empty_source) == -1);
    assert(tinyui_progress_bar_set_image(bar, &empty_source, &empty_source) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_progress_bar_create_and_backend_mapping(root);
    test_progress_bar_create_with_props_pushes_fields(root);
    test_progress_bar_percent_orientation_color_source_parity(root);
    test_progress_bar_props_invalid_rolls_back(root);
    test_progress_bar_rejects_null_and_invalid(root);

    tinyui_deinit();
    return 0;
}
