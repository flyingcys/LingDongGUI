/*
 * TinyUI progress_wheel unit tests — M3 Task 2 L4 harness.
 *
 * Validates real ldProgressWheel_t percent/dot/color mapping, bounds,
 * props rollback, and getters that follow backend truth rather than mirrors.
 */

#define __PROGRESS_WHEEL_IMPLEMENT__
#include "progress_wheel.h"

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldProgressWheel.h"
#include "internal.h"
#include "widgets/progress_wheel.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

extern void tinyui_progress_wheel_test_fail_next_set_percent(void);

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void test_progress_wheel_create_and_backend_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *wheel = tinyui_progress_wheel_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    backend = (struct tinyui_widget *)(void *)wheel;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeProgressWheel);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel->iProgress == 0);
    assert(ld_progress_wheel->tWheel.tCFG.bIgnoreDot == false);
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 1);
}

static void test_progress_wheel_create_with_props_pushes_fields(tinyui_obj_t *root)
{
    tinyui_progress_wheel_props_t props;
    tinyui_obj_t *wheel;
    ldProgressWheel_t *ld_progress_wheel;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_PROGRESS_WHEEL_FIELD_PERCENT
        | TINYUI_PROGRESS_WHEEL_FIELD_DOT_ENABLED;
    props.percent = 72;
    props.dot_enabled = 0;

    wheel = tinyui_progress_wheel_create_with_props(root, &props);
    assert(wheel != 0);
    ld_progress_wheel =
        (ldProgressWheel_t *)((struct tinyui_widget *)(void *)wheel)->ld_widget;
    assert(ld_progress_wheel != 0);
    assert(ld_progress_wheel->iProgress == 720);
    assert(tinyui_progress_wheel_get_percent(wheel) == 72);
    assert(ld_progress_wheel->tWheel.tCFG.bIgnoreDot == true);
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 0);
}

static void test_progress_wheel_percent_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *wheel = tinyui_progress_wheel_create(root);
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    ld_progress_wheel =
        (ldProgressWheel_t *)((struct tinyui_widget *)(void *)wheel)->ld_widget;
    assert(ld_progress_wheel != 0);

    assert(tinyui_progress_wheel_set_percent(wheel, 0) == 0);
    assert(ld_progress_wheel->iProgress == 0);
    assert(tinyui_progress_wheel_get_percent(wheel) == 0);

    assert(tinyui_progress_wheel_set_percent(wheel, 100) == 0);
    assert(ld_progress_wheel->iProgress == 1000);
    assert(tinyui_progress_wheel_get_percent(wheel) == 100);

    assert(tinyui_progress_wheel_set_percent(wheel, -1) == -1);
    assert(ld_progress_wheel->iProgress == 1000);
    assert(tinyui_progress_wheel_get_percent(wheel) == 100);

    assert(tinyui_progress_wheel_set_percent(wheel, 101) == -1);
    assert(ld_progress_wheel->iProgress == 1000);
    assert(tinyui_progress_wheel_get_percent(wheel) == 100);

    /* getter must follow LD, not stale wrapper cache */
    ld_progress_wheel->iProgress = 370;
    assert(tinyui_progress_wheel_get_percent(wheel) == 37);

    assert(tinyui_progress_wheel_set_progress(wheel, 55) == 0);
    assert(ld_progress_wheel->iProgress == 550);
    assert(tinyui_progress_wheel_get_percent(wheel) == 55);
}

static void test_progress_wheel_color_and_dot_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *wheel = tinyui_progress_wheel_create(root);
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    ld_progress_wheel =
        (ldProgressWheel_t *)((struct tinyui_widget *)(void *)wheel)->ld_widget;
    assert(ld_progress_wheel != 0);

    assert(tinyui_progress_wheel_set_wheel_color(wheel, 0x123456U) == 0);
    assert(ld_progress_wheel->tWheel.tCFG.tWheelColour
           == (ldColor)test_rgb_to_ld_color(0x123456U));

    assert(tinyui_progress_wheel_set_dot_color(wheel, 0xABCDEFU) == 0);
    assert(ld_progress_wheel->tWheel.tCFG.tDotColour
           == (ldColor)test_rgb_to_ld_color(0xABCDEFU));

    assert(tinyui_progress_wheel_set_dot_enabled(wheel, 0) == 0);
    assert(ld_progress_wheel->tWheel.tCFG.bIgnoreDot == true);
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 0);

    assert(tinyui_progress_wheel_set_dot_enabled(wheel, 1) == 0);
    assert(ld_progress_wheel->tWheel.tCFG.bIgnoreDot == false);
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 1);

    /* getter must follow LD, not stale wrapper cache */
    ld_progress_wheel->tWheel.tCFG.bIgnoreDot = true;
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 0);
    ld_progress_wheel->tWheel.tCFG.bIgnoreDot = false;
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 1);
}

static void test_progress_wheel_create_with_props_failure_rolls_back(tinyui_obj_t *root)
{
    tinyui_progress_wheel_props_t props;
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *child_before = ldBaseGetChildList(root_ld);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_PROGRESS_WHEEL_FIELD_PERCENT;
    props.percent = 40;
    tinyui_progress_wheel_test_fail_next_set_percent();
    assert(tinyui_progress_wheel_create_with_props(root, &props) == 0);
    assert(ldBaseGetChildList(root_ld) == child_before);
}

static void test_progress_wheel_rejects_null_and_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *wheel = tinyui_progress_wheel_create(root);

    assert(wheel != 0);
    assert(tinyui_progress_wheel_create(0) == 0);
    assert(tinyui_progress_wheel_set_percent(0, 10) == -1);
    assert(tinyui_progress_wheel_get_percent(0) == -1);
    assert(tinyui_progress_wheel_set_percent(wheel, -3) == -1);
    assert(tinyui_progress_wheel_set_percent(wheel, 130) == -1);
    assert(tinyui_progress_wheel_set_wheel_color(0, 0x111111U) == -1);
    assert(tinyui_progress_wheel_set_dot_color(0, 0x222222U) == -1);
    assert(tinyui_progress_wheel_set_dot_enabled(0, 1) == -1);
    assert(tinyui_progress_wheel_get_dot_enabled(0) == -1);
    assert(tinyui_progress_wheel_set_wheel_color(wheel, 0x1000000U) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_progress_wheel_create_and_backend_mapping(root);
    test_progress_wheel_create_with_props_pushes_fields(root);
    test_progress_wheel_percent_native_parity(root);
    test_progress_wheel_color_and_dot_native_parity(root);
    test_progress_wheel_create_with_props_failure_rolls_back(root);
    test_progress_wheel_rejects_null_and_invalid(root);

    tinyui_deinit();
    return 0;
}
