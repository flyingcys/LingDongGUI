/*
 * TinyUI gauge unit tests — M3 Task 2 L4 harness.
 *
 * Validates real ldGauge_t angle/pointer/source/offset/auto mapping,
 * props rollback, and getters that follow backend truth.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldGauge.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/gauge.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

extern struct tinyui_gauge *
tinyui_gauge_test_create_with_props_fail_before_centre_offset(
    struct tinyui_widget *parent,
    const tinyui_gauge_props_t *props);

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static int float_near(float a, float b)
{
    return fabsf(a - b) < 0.01f;
}

static void bind_test_tiles(tinyui_image_source_t *source,
                            arm_2d_tile_t *img_tile,
                            arm_2d_tile_t *mask_tile)
{
    assert(source != 0);
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    if (img_tile != 0) {
        img_tile->tRegion.tSize.iWidth = 20;
        img_tile->tRegion.tSize.iHeight = 40;
        source->width = 20;
        source->height = 40;
        memcpy(source->_image_private, img_tile, sizeof(*img_tile));
    }
    if (mask_tile != 0) {
        mask_tile->tRegion.tSize.iWidth = 20;
        mask_tile->tRegion.tSize.iHeight = 40;
        memcpy(source->_mask_private, mask_tile, sizeof(*mask_tile));
    }
}

static void test_gauge_create_and_backend_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *gauge = tinyui_gauge_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    ldGauge_t *ld_gauge;

    assert(gauge != 0);
    backend = (struct tinyui_widget *)(void *)gauge;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_GAUGE);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeGauge);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(float_near(tinyui_gauge_get_angle(gauge), 0.0f));
    assert(tinyui_gauge_get_auto_move(gauge) == 0);
    assert(ld_gauge->isAutoMove == false);
}

static void test_gauge_create_with_props_pushes_fields(tinyui_obj_t *root)
{
    tinyui_gauge_props_t props;
    tinyui_obj_t *gauge;
    ldGauge_t *ld_gauge;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask = {0};
    arm_2d_tile_t pointer_tile = {0};
    arm_2d_tile_t pointer_mask = {0};
    tinyui_image_source_t bg_source;
    tinyui_image_source_t pointer_source;

    bind_test_tiles(&bg_source, &bg_tile, &bg_mask);
    bind_test_tiles(&pointer_source, &pointer_tile, &pointer_mask);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GAUGE_FIELD_ANGLE
        | TINYUI_GAUGE_FIELD_BG_SOURCE
        | TINYUI_GAUGE_FIELD_POINTER_SOURCE
        | TINYUI_GAUGE_FIELD_CENTRE_OFFSET_X
        | TINYUI_GAUGE_FIELD_CENTRE_OFFSET_Y
        | TINYUI_GAUGE_FIELD_POINTER_COLOR
        | TINYUI_GAUGE_FIELD_AUTO_MOVE;
    props.angle = 123.0f;
    props.bg_source = &bg_source;
    props.pointer_source = &pointer_source;
    props.centre_offset_x = 3;
    props.centre_offset_y = -4;
    props.pointer_color = 0xAA55CCU;
    props.auto_move = 1;

    gauge = tinyui_gauge_create_with_props(root, &props);
    assert(gauge != 0);
    ld_gauge = (ldGauge_t *)((struct tinyui_widget *)(void *)gauge)->ld_widget;
    assert(ld_gauge != 0);
    assert(float_near(tinyui_gauge_get_angle(gauge), 123.0f));
    assert(ld_gauge->centreOffsetX == 3);
    assert(ld_gauge->centreOffsetY == -4);
    assert(ld_gauge->maskColor == (ldColor)test_rgb_to_ld_color(0xAA55CCU));
    assert(ld_gauge->isAutoMove == true);
    assert(tinyui_gauge_get_auto_move(gauge) == 1);
}

static void test_gauge_angle_pointer_source_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *gauge = tinyui_gauge_create(root);
    ldGauge_t *ld_gauge;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask = {0};
    arm_2d_tile_t pointer_tile = {0};
    arm_2d_tile_t pointer_mask = {0};
    arm_2d_tile_t trail_bg_mask = {0};
    arm_2d_tile_t trail_ptr_mask = {0};
    arm_2d_tile_t progress_bg_mask = {0};
    arm_2d_tile_t progress_ptr_mask = {0};
    tinyui_image_source_t bg_source;
    tinyui_image_source_t pointer_source;
    tinyui_image_source_t trail_bg;
    tinyui_image_source_t trail_ptr;
    tinyui_image_source_t progress_bg;
    tinyui_image_source_t progress_ptr;

    assert(gauge != 0);
    ld_gauge = (ldGauge_t *)((struct tinyui_widget *)(void *)gauge)->ld_widget;
    assert(ld_gauge != 0);

    assert(tinyui_gauge_set_angle(gauge, 77.5f) == 0);
    assert(float_near(tinyui_gauge_get_angle(gauge), 77.5f));
    assert(ld_gauge->endAngle_x10 == 775);
    assert(ld_gauge->_nowAngle_x10 == 775);

    assert(tinyui_gauge_set_pointer_color(gauge, 0x102030U) == 0);
    assert(ld_gauge->maskColor == (ldColor)test_rgb_to_ld_color(0x102030U));
    assert(tinyui_gauge_get_pointer_color(gauge)
           == tinyui_ld_color_to_rgb((unsigned int)ld_gauge->maskColor));

    assert(tinyui_gauge_set_auto_move(gauge, 1) == 0);
    assert(ld_gauge->isAutoMove == true);
    assert(tinyui_gauge_get_auto_move(gauge) == 1);
    assert(tinyui_gauge_set_auto_move(gauge, 0) == 0);
    assert(ld_gauge->isAutoMove == false);

    assert(tinyui_gauge_set_centre_offset(gauge, 5, -6) == 0);
    assert(ld_gauge->centreOffsetX == 5);
    assert(ld_gauge->centreOffsetY == -6);

    bind_test_tiles(&bg_source, &bg_tile, &bg_mask);
    bind_test_tiles(&pointer_source, &pointer_tile, &pointer_mask);
    bind_test_tiles(&trail_bg, 0, &trail_bg_mask);
    bind_test_tiles(&trail_ptr, 0, &trail_ptr_mask);
    bind_test_tiles(&progress_bg, 0, &progress_bg_mask);
    bind_test_tiles(&progress_ptr, 0, &progress_ptr_mask);

    assert(tinyui_gauge_set_bg_source(gauge, &bg_source) == 0);
    assert(tinyui_gauge_set_background_image(gauge, &bg_source) == 0);
    assert(ld_gauge->ptBgImgTile == tinyui_image_source_get_image_tile(&bg_source));
    assert(ld_gauge->ptBgMaskTile == tinyui_image_source_get_mask_tile(&bg_source));

    assert(tinyui_gauge_set_pointer_source(gauge, &pointer_source) == 0);
    assert(tinyui_gauge_set_pointer_source_with_origin(gauge, &pointer_source, 4, 8) == 0);
    assert(tinyui_gauge_set_pointer_mask_source(gauge, &pointer_source, 2, 3) == 0);
    assert(tinyui_gauge_set_trail(gauge, &trail_bg, &trail_ptr) == 0);
    assert(tinyui_gauge_set_progress_bar(gauge, &progress_bg, &progress_ptr) == 0);

    /* getter follows LD */
    ld_gauge->_nowAngle_x10 = 330;
    assert(float_near(tinyui_gauge_get_angle(gauge), 33.0f));
    ld_gauge->isAutoMove = true;
    assert(tinyui_gauge_get_auto_move(gauge) == 1);
}

static void test_gauge_create_with_props_failure_rolls_back(tinyui_obj_t *root)
{
    tinyui_gauge_props_t props;
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *child_before = ldBaseGetChildList(root_ld);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GAUGE_FIELD_ANGLE | TINYUI_GAUGE_FIELD_AUTO_MOVE;
    props.angle = 10.0f;
    props.auto_move = 1;

    assert(tinyui_gauge_test_create_with_props_fail_before_centre_offset(
               (struct tinyui_widget *)(void *)root, &props)
           == 0);
    assert(ldBaseGetChildList(root_ld) == child_before);
}

static void test_gauge_rejects_null_and_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *gauge = tinyui_gauge_create(root);
    tinyui_image_source_t empty_source;

    assert(gauge != 0);
    memset(&empty_source, 0, sizeof(empty_source));

    assert(tinyui_gauge_create(0) == 0);
    assert(tinyui_gauge_set_angle(0, 1.0f) == -1);
    assert(tinyui_gauge_set_bg_source(gauge, &empty_source) == -1);
    assert(tinyui_gauge_set_pointer_source(gauge, &empty_source) == -1);
    assert(tinyui_gauge_set_pointer_color(0, 0) == -1);
    assert(tinyui_gauge_set_auto_move(0, 1) == -1);
    assert(tinyui_gauge_get_auto_move(0) == -1);
    assert(tinyui_gauge_set_centre_offset(0, 0, 0) == -1);
    assert(tinyui_gauge_set_trail(gauge, 0, 0) == -1);
    assert(tinyui_gauge_set_progress_bar(gauge, 0, 0) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_gauge_create_and_backend_mapping(root);
    test_gauge_create_with_props_pushes_fields(root);
    test_gauge_angle_pointer_source_native_parity(root);
    test_gauge_create_with_props_failure_rolls_back(root);
    test_gauge_rejects_null_and_invalid(root);

    tinyui_deinit();
    return 0;
}
