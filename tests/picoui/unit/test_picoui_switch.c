#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/gui/ldSwitchInternal.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

static void test_switch_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_switch *sw = picoui_switch_create(win, "switch_test");
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(sw != 0);
    backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_SWITCH);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeSwitch);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 48);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 24);
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

static void test_switch_create_with_props_pushes_fields(struct picoui_window *win)
{
    struct picoui_switch *sw = picoui_switch_create_with_props(
        win,
        &(struct picoui_switch_props){
            .id = "sw_props",
            .checked = 1,
        });
    struct picoui_backend_widget *backend;
    ldSwitch_t *ld_sw;

    assert(sw != 0);
    backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    assert(backend != 0);
    ld_sw = (ldSwitch_t *)backend->ld_widget;
    assert(ld_sw != 0);
    assert(ld_sw->isChecked == true);
    assert(sw->checked == 1);
}

static void test_switch_set_checked_round_trip(struct picoui_window *win)
{
    struct picoui_switch *sw = picoui_switch_create(win, "sw_checked");
    struct picoui_backend_widget *backend;
    ldSwitch_t *ld_sw;

    assert(sw != 0);
    backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    assert(backend != 0);
    ld_sw = (ldSwitch_t *)backend->ld_widget;
    assert(ld_sw != 0);

    assert(picoui_switch_set_checked(sw, 1) == 0);
    assert(ld_sw->isChecked == true);
    assert(sw->checked == 1);
    assert(picoui_switch_is_checked(sw) == 1);

    assert(picoui_switch_set_checked(sw, 0) == 0);
    assert(ld_sw->isChecked == false);
    assert(sw->checked == 0);
    assert(picoui_switch_is_checked(sw) == 0);
}

static void test_switch_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_switch_create(0, "id") == 0);
    assert(picoui_switch_create(win, 0) == 0);
    assert(picoui_switch_set_checked(0, 1) == -1);
    assert(picoui_switch_is_checked(0) == 0);
    assert(picoui_switch_set_on_toggled(0, 0, 0) == -1);
    assert(picoui_switch_set_horizontal(0, 1) == -1);
    assert(picoui_switch_set_direction(0, 0) == -1);
    assert(picoui_switch_set_disabled(0, 1) == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_switch_create_and_backend_mapping(win);
    test_switch_default_geometry_matches_capsule_track();
    test_switch_create_with_props_pushes_fields(win);
    test_switch_set_checked_round_trip(win);
    test_switch_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
