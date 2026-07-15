/*
 * TinyUI arc unit tests — M3 Task 2 L4 harness.
 *
 * Validates real ldArc_t angle/rotation/color/source mapping, props rollback,
 * and getters that follow backend truth rather than mirrors.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldArc.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/arc.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

extern struct tinyui_arc *
tinyui_arc_test_create_with_props_fail_before_parent_color(
    struct tinyui_widget *parent,
    const tinyui_arc_props_t *props);

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
        source->width = (uint16_t)img_tile->tRegion.tSize.iWidth;
        source->height = (uint16_t)img_tile->tRegion.tSize.iHeight;
        memcpy(source->_image_private, img_tile, sizeof(*img_tile));
    }
    if (mask_tile != 0) {
        memcpy(source->_mask_private, mask_tile, sizeof(*mask_tile));
    }
}

static void test_arc_create_and_backend_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *arc = tinyui_arc_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    ldArc_t *ld_arc;

    assert(arc != 0);
    backend = (struct tinyui_widget *)(void *)arc;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_ARC);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeArc);
    ld_arc = (ldArc_t *)backend->ld_widget;
    assert(float_near(tinyui_arc_get_background_start_angle(arc), 0.0f));
    assert(float_near(tinyui_arc_get_background_angle(arc), 360.0f));
    assert(float_near(tinyui_arc_get_foreground_angle(arc), 0.0f));
    assert(float_near(tinyui_arc_get_rotation_angle(arc), 0.0f));
    assert(ld_arc->parentColor == (ldColor)test_rgb_to_ld_color(0xF0F0F0U));
}

static void test_arc_create_with_props_pushes_fields(tinyui_obj_t *root)
{
    tinyui_arc_props_t props;
    tinyui_obj_t *arc;
    arm_2d_tile_t q_tile = {0};
    arm_2d_tile_t q_mask = {0};
    tinyui_image_source_t quarter_source;
    ldArc_t *ld_arc;

    bind_test_tiles(&quarter_source, &q_tile, &q_mask);
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_ARC_FIELD_BG_START_ANGLE
        | TINYUI_ARC_FIELD_BG_END_ANGLE
        | TINYUI_ARC_FIELD_FG_END_ANGLE
        | TINYUI_ARC_FIELD_ROTATION_ANGLE
        | TINYUI_ARC_FIELD_QUARTER_SOURCE
        | TINYUI_ARC_FIELD_PARENT_COLOR
        | TINYUI_ARC_FIELD_BG_COLOR
        | TINYUI_ARC_FIELD_FG_COLOR;
    props.bg_start_angle = 10.0f;
    props.bg_end_angle = 300.0f;
    props.fg_end_angle = 120.0f;
    props.rotation_angle = 45.0f;
    props.quarter_source = &quarter_source;
    props.parent_color = 0x101010U;
    props.bg_color = 0xABCDEFU;
    props.fg_color = 0x123456U;

    arc = tinyui_arc_create_with_props(root, &props);
    assert(arc != 0);
    ld_arc = (ldArc_t *)((struct tinyui_widget *)(void *)arc)->ld_widget;
    assert(ld_arc != 0);
    assert(float_near(tinyui_arc_get_background_start_angle(arc), 10.0f));
    assert(float_near(tinyui_arc_get_background_angle(arc), 300.0f));
    assert(float_near(tinyui_arc_get_foreground_angle(arc), 120.0f));
    assert(float_near(tinyui_arc_get_rotation_angle(arc), 45.0f));
    assert(ld_arc->ptImgTile == tinyui_image_source_get_image_tile(&quarter_source));
    assert(ld_arc->ptMaskTile == tinyui_image_source_get_mask_tile(&quarter_source));
    assert(ld_arc->parentColor == (ldColor)test_rgb_to_ld_color(0x101010U));
    assert(float_near(ldArcGetBackgroundStartAngle(ld_arc), 10.0f));
    assert(float_near(ldArcGetBackgroundAngle(ld_arc), 300.0f));
    assert(float_near(ldArcGetForegroundAngle(ld_arc), 120.0f));
    assert(float_near(ldArcGetRotationAngle(ld_arc), 45.0f));
    assert(ldArcGetBackgroundColor(ld_arc) == (ldColor)test_rgb_to_ld_color(0xABCDEFU));
    assert(ldArcGetForegroundColor(ld_arc) == (ldColor)test_rgb_to_ld_color(0x123456U));
}

static void test_arc_angle_color_source_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *arc = tinyui_arc_create(root);
    ldArc_t *ld_arc;
    arm_2d_tile_t q_tile = {0};
    arm_2d_tile_t q_mask = {0};
    tinyui_image_source_t quarter_source;

    assert(arc != 0);
    ld_arc = (ldArc_t *)((struct tinyui_widget *)(void *)arc)->ld_widget;
    assert(ld_arc != 0);

    assert(tinyui_arc_set_background_angle(arc, 20.0f, 200.0f) == 0);
    assert(float_near(ldArcGetBackgroundStartAngle(ld_arc), 20.0f));
    assert(float_near(ldArcGetBackgroundAngle(ld_arc), 200.0f));
    assert(float_near(tinyui_arc_get_background_start_angle(arc), 20.0f));
    assert(float_near(tinyui_arc_get_background_angle(arc), 200.0f));

    assert(tinyui_arc_set_foreground_angle(arc, 90.0f) == 0);
    assert(float_near(ldArcGetForegroundAngle(ld_arc), 90.0f));
    assert(float_near(tinyui_arc_get_foreground_angle(arc), 90.0f));

    assert(tinyui_arc_set_rotation_angle(arc, 30.0f) == 0);
    assert(float_near(ldArcGetRotationAngle(ld_arc), 30.0f));
    assert(float_near(tinyui_arc_get_rotation_angle(arc), 30.0f));

    assert(tinyui_arc_set_color(arc, 0x112233U, 0x445566U) == 0);
    assert(ldArcGetBackgroundColor(ld_arc) == (ldColor)test_rgb_to_ld_color(0x112233U));
    assert(ldArcGetForegroundColor(ld_arc) == (ldColor)test_rgb_to_ld_color(0x445566U));
    assert(tinyui_arc_get_background_color(arc)
           == tinyui_ld_color_to_rgb((unsigned int)ldArcGetBackgroundColor(ld_arc)));
    assert(tinyui_arc_get_foreground_color(arc)
           == tinyui_ld_color_to_rgb((unsigned int)ldArcGetForegroundColor(ld_arc)));

    assert(tinyui_arc_set_parent_color(arc, 0x778899U) == 0);
    assert(ld_arc->parentColor == (ldColor)test_rgb_to_ld_color(0x778899U));

    bind_test_tiles(&quarter_source, &q_tile, &q_mask);
    assert(tinyui_arc_set_quarter_source(arc, &quarter_source) == 0);
    assert(tinyui_arc_set_quarter_image(arc, &quarter_source) == 0);

    /* invalid */
    assert(tinyui_arc_set_background_angle(arc, -1.0f, 10.0f) == -1);
    assert(tinyui_arc_set_background_angle(arc, 50.0f, 10.0f) == -1);
    assert(tinyui_arc_set_foreground_angle(arc, -1.0f) == -1);
    assert(tinyui_arc_set_rotation_angle(arc, -1.0f) == -1);
}

static void test_arc_create_with_props_failure_rolls_back(tinyui_obj_t *root)
{
    tinyui_arc_props_t props;
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *child_before = ldBaseGetChildList(root_ld);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_ARC_FIELD_BG_START_ANGLE | TINYUI_ARC_FIELD_BG_END_ANGLE
        | TINYUI_ARC_FIELD_FG_END_ANGLE | TINYUI_ARC_FIELD_ROTATION_ANGLE;
    props.bg_start_angle = 0.0f;
    props.bg_end_angle = 180.0f;
    props.fg_end_angle = 90.0f;
    props.rotation_angle = 0.0f;

    assert(tinyui_arc_test_create_with_props_fail_before_parent_color(
               (struct tinyui_widget *)(void *)root, &props)
           == 0);
    assert(ldBaseGetChildList(root_ld) == child_before);
}

static void test_arc_rejects_null_and_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *arc = tinyui_arc_create(root);
    tinyui_image_source_t empty_source;

    assert(arc != 0);
    memset(&empty_source, 0, sizeof(empty_source));

    assert(tinyui_arc_create(0) == 0);
    assert(tinyui_arc_set_background_angle(0, 0.0f, 1.0f) == -1);
    assert(tinyui_arc_set_foreground_angle(0, 1.0f) == -1);
    assert(tinyui_arc_set_rotation_angle(0, 1.0f) == -1);
    assert(tinyui_arc_set_color(0, 0, 0) == -1);
    assert(tinyui_arc_set_parent_color(0, 0) == -1);
    assert(tinyui_arc_set_parent_color(arc, 0x1000000U) == -1);
    assert(tinyui_arc_set_quarter_source(arc, &empty_source) == -1);
    assert(tinyui_arc_set_quarter_image(arc, 0) == -1);
    assert(tinyui_arc_get_background_start_angle(0) == 0.0f);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_arc_create_and_backend_mapping(root);
    test_arc_create_with_props_pushes_fields(root);
    test_arc_angle_color_source_native_parity(root);
    test_arc_create_with_props_failure_rolls_back(root);
    test_arc_rejects_null_and_invalid(root);

    tinyui_deinit();
    return 0;
}
