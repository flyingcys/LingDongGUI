#include "core/app.h"
#include "core/runtime.h"
#include "widgets/switch.h"
#include "widgets/window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/gui/ldSwitchInternal.h"
#include "../../../tinyui/src/core/internal.h"
#include "tinyui_test_support.h"
#include <assert.h>
#include <string.h>

static void test_switch_create_and_backend_mapping(struct tinyui_window *win)
{
    tinyui_obj_t *obj = tinyui_switch_create((tinyui_obj_t *)win, "switch_test");
    struct tinyui_switch *sw = (struct tinyui_switch *)obj;
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(sw != 0);
    backend = &sw->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_SWITCH);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeSwitch);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 48);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 24);
}

static void test_switch_create_builds_direct_backend_mapping(struct tinyui_window *win)
{
    tinyui_obj_t *obj = tinyui_switch_create((tinyui_obj_t *)win, "switch_direct");
    struct tinyui_switch *sw = (struct tinyui_switch *)obj;
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldSwitch_t *ld_switch;

    assert(sw != 0);
    backend = &sw->widget;
    parent_backend = &win->widget;
    assert(backend->ld_widget != 0);
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_SWITCH);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_switch = (ldSwitch_t *)backend->ld_widget;
    assert(ld_switch != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
}

static void test_switch_default_geometry_matches_capsule_track(void)
{
    ldSwitchGeometry_t off = ldSwitchResolveGeometry(48, 24, 4U, LD_SWITCH_DIRECTION_AUTO, 0U);
    ldSwitchGeometry_t on = ldSwitchResolveGeometry(48, 24, 4U, LD_SWITCH_DIRECTION_AUTO, 1000U);

    assert(off.isHorizontal == true);
    assert(off.track.iX == 0);
    assert(off.track.iY == 0);
    assert(off.track.iWidth == 48);
    assert(off.track.iHeight == 24);
    assert(off.knob.iWidth == 16);
    assert(off.knob.iHeight == 16);
    assert(off.knob.iX == 4);
    assert(off.knob.iY == 4);
    assert(off.indicator.iWidth == 0);
    assert(off.indicator.iHeight == 24);
    assert(on.knob.iX == 28);
    assert(on.knob.iY == 4);
    assert(on.indicator.iWidth == 48);
}

static void test_switch_create_with_props_pushes_fields(struct tinyui_window *win)
{
    struct tinyui_switch *sw = tinyui_switch_create_with_props(
        win,
        &(struct tinyui_switch_props){
            .id = "sw_props",
            .checked = 1,
            .disabled = 1,
            .horizontal = -1,
            .direction = -1,
        });
    struct tinyui_widget *backend;
    ldSwitch_t *ld_sw;

    assert(sw != 0);
    backend = &sw->widget;
    assert(backend->ld_widget != 0);
    ld_sw = (ldSwitch_t *)backend->ld_widget;
    assert(ld_sw != 0);
    assert(ld_sw->isChecked == true);
    assert(sw->checked == 1);
    assert(ld_sw->isDisabled == true);
    assert(sw->widget.enabled == 0);
}

static void test_switch_create_with_props_uses_sentinel_optional_fields(struct tinyui_window *win)
{
    struct tinyui_switch *default_sw = tinyui_switch_create_with_props(
        win,
        &(struct tinyui_switch_props){
            .id = "sw_default_optional",
            .horizontal = -1,
            .direction = -1,
            .disabled = -1,
        });
    struct tinyui_switch *configured_sw = tinyui_switch_create_with_props(
        win,
        &(struct tinyui_switch_props){
            .id = "sw_configured_optional",
            .off_source = 0,
            .on_source = 0,
            .knob_source = 0,
            .horizontal = 0,
            .direction = 2,
            .disabled = 1,
        });
    ldSwitch_t *default_ld_sw;
    ldSwitch_t *configured_ld_sw;
    int horizontal = -1;
    int direction = -1;
    int disabled = -1;

    assert(default_sw != 0);
    assert(default_sw->widget.ld_widget != 0);
    default_ld_sw = (ldSwitch_t *)default_sw->widget.ld_widget;
    assert(default_ld_sw != 0);
    assert(default_ld_sw->isHorizontal == true);
    assert(default_ld_sw->direction == LD_SWITCH_DIRECTION_AUTO);
    assert(default_ld_sw->isDisabled == false);

    assert(configured_sw != 0);
    assert(configured_sw->widget.ld_widget != 0);
    configured_ld_sw = (ldSwitch_t *)configured_sw->widget.ld_widget;
    assert(configured_ld_sw != 0);
    assert(tinyui_switch_get_horizontal(configured_sw, &horizontal) == 0);
    assert(horizontal == 0);
    assert(configured_ld_sw->isHorizontal == false);
    assert(tinyui_switch_get_direction(configured_sw, &direction) == 0);
    assert(direction == 2);
    assert(configured_ld_sw->direction == LD_SWITCH_DIRECTION_VERTICAL);
    assert(tinyui_switch_get_disabled(configured_sw, &disabled) == 0);
    assert(disabled == 1);
    assert(configured_ld_sw->isDisabled == true);
}

static void test_switch_set_checked_round_trip(struct tinyui_window *win)
{
    tinyui_obj_t *obj = tinyui_switch_create((tinyui_obj_t *)win, "sw_checked");
    struct tinyui_switch *sw = (struct tinyui_switch *)obj;
    struct tinyui_widget *backend;
    ldSwitch_t *ld_sw;

    assert(sw != 0);
    backend = &sw->widget;
    assert(backend->ld_widget != 0);
    ld_sw = (ldSwitch_t *)backend->ld_widget;
    assert(ld_sw != 0);

    assert(tinyui_switch_set_checked(sw, 1) == 0);
    assert(ld_sw->isChecked == true);
    assert(sw->checked == 1);
    assert(tinyui_switch_is_checked(sw) == 1);

    assert(tinyui_switch_set_checked(sw, 0) == 0);
    assert(ld_sw->isChecked == false);
    assert(sw->checked == 0);
    assert(tinyui_switch_is_checked(sw) == 0);
}

static void test_switch_set_disabled_round_trip(struct tinyui_window *win)
{
    tinyui_obj_t *obj = tinyui_switch_create((tinyui_obj_t *)win, "sw_disabled");
    struct tinyui_switch *sw = (struct tinyui_switch *)obj;
    struct tinyui_widget *backend;
    ldSwitch_t *ld_sw;
    int disabled = -1;

    assert(sw != 0);
    backend = &sw->widget;
    assert(backend->ld_widget != 0);
    ld_sw = (ldSwitch_t *)backend->ld_widget;
    assert(ld_sw != 0);

    assert(sw->widget.enabled == 1);
    assert(tinyui_switch_set_disabled(sw, 1) == 0);
    assert(tinyui_switch_get_disabled(sw, &disabled) == 0);
    assert(disabled == 1);
    assert(sw->widget.enabled == 0);
    assert(ld_sw->isDisabled == true);

    assert(tinyui_switch_set_disabled(sw, 0) == 0);
    assert(tinyui_switch_get_disabled(sw, &disabled) == 0);
    assert(disabled == 0);
    assert(sw->widget.enabled == 1);
    assert(ld_sw->isDisabled == false);
}

static void test_switch_rejects_null_args(struct tinyui_window *win)
{
    (void)win;
    assert(tinyui_switch_create(0, "id") == 0);
    assert(tinyui_switch_set_checked(0, 1) == -1);
    assert(tinyui_switch_is_checked(0) == 0);
    assert(tinyui_switch_set_on_toggled(0, 0, 0) == -1);
    assert(tinyui_switch_set_horizontal(0, 1) == -1);
    assert(tinyui_switch_set_direction(0, 0) == -1);
    assert(tinyui_switch_set_disabled(0, 1) == -1);
}

static void test_switch_props_source_no_longer_uses_has_flags(void)
{
    const char *header_source = "tinyui/include/switch.h";
    const char *widget_source = "tinyui/src/widgets/switch.c";

    assert(tinyui_test_source_contains(header_source, "has_off_source") == 0);
    assert(tinyui_test_source_contains(header_source, "has_on_source") == 0);
    assert(tinyui_test_source_contains(header_source, "has_knob_source") == 0);
    assert(tinyui_test_source_contains(header_source, "has_horizontal") == 0);
    assert(tinyui_test_source_contains(header_source, "has_direction") == 0);
    assert(tinyui_test_source_contains(header_source, "has_disabled") == 0);
    assert(tinyui_test_source_contains(widget_source, "props->has_off_source") == 0);
    assert(tinyui_test_source_contains(widget_source, "props->has_on_source") == 0);
    assert(tinyui_test_source_contains(widget_source, "props->has_knob_source") == 0);
    assert(tinyui_test_source_contains(widget_source, "props->has_horizontal") == 0);
    assert(tinyui_test_source_contains(widget_source, "props->has_direction") == 0);
    assert(tinyui_test_source_contains(widget_source, "props->has_disabled") == 0);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_switch_create_and_backend_mapping(win);
    test_switch_create_builds_direct_backend_mapping(win);
    test_switch_default_geometry_matches_capsule_track();
    test_switch_create_with_props_pushes_fields(win);
    test_switch_create_with_props_uses_sentinel_optional_fields(win);
    test_switch_set_checked_round_trip(win);
    test_switch_set_disabled_round_trip(win);
    test_switch_rejects_null_args(win);
    test_switch_props_source_no_longer_uses_has_flags();

    tinyui_app_destroy(app);
    return 0;
}
