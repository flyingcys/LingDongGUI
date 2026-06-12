#include "app.h"
#include "progress_wheel.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldProgressWheel.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef assert
#undef assert
#endif
#define assert(cond)                                                                          \
    do {                                                                                      \
        if (!(cond)) {                                                                        \
            fprintf(stderr,                                                                   \
                    "Assertion failed: (%s), function %s, file %s, line %d.\n",              \
                    #cond,                                                                    \
                    __func__,                                                                 \
                    __FILE__,                                                                 \
                    __LINE__);                                                                \
            fflush(stderr);                                                                   \
            exit(1);                                                                          \
        }                                                                                     \
    } while (0)

extern int tinyui_widget_has_ld_binding(const struct picoui_widget *widget);
struct tinyui_progress_wheel_cfg_bridge {
    struct {
        const arm_2d_tile_t *ptileArcMask;
        const arm_2d_tile_t *ptileDotMask;
        int16_t iWheelDiameter;
        int16_t iRingWidth;
        COLOUR_INT tWheelColour;
        COLOUR_INT tDotColour;
        uint32_t bUseDirtyRegions : 1;
        uint32_t bIgnoreDot : 1;
        uint32_t u2StartPosition : 2;
    } tCFG;
};
struct tinyui_progress_wheel_test_dispose_snapshot {
    int kind;
    int cleanup_complete;
    int cleanup_incomplete;
    int detach_result;
    int unbind_result;
    int detached;
    int owner_cleared;
    int root_cleared;
    int parent_cleared;
    int next_sibling_cleared;
    int host_cleared;
    int event_bridge_cleared;
    int ld_pinfo_cleared;
};

__attribute__((weak)) void tinyui_progress_wheel_test_reset_state(void)
{
}

__attribute__((weak)) void tinyui_progress_wheel_test_fail_next_set_percent(void)
{
}

__attribute__((weak)) int tinyui_progress_wheel_test_take_last_dispose_snapshot(
    struct tinyui_progress_wheel_test_dispose_snapshot *snapshot)
{
    (void)snapshot;
    return -1;
}

static struct picoui_progress_wheel *test_progress_wheel_create_and_props(struct picoui_window *win)
{
    int user_cookie = 9;
    struct picoui_progress_wheel_props props = {
        .id = "wheel_props",
        .style_class = "wheel",
        .user_data = &user_cookie,
        .percent = 72,
    };
    struct picoui_progress_wheel *wheel =
        picoui_progress_wheel_create((struct picoui_widget *)win, "wheel");
    struct picoui_progress_wheel *with_props =
        picoui_progress_wheel_create_with_props((struct picoui_widget *)win, &props);

    assert(wheel != 0);
    assert(with_props != 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 0);
    assert(picoui_progress_wheel_get_percent(with_props) == props.percent);

    return with_props;
}

static void test_progress_wheel_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_progress_wheel *wheel =
        picoui_progress_wheel_create((struct picoui_widget *)win, "wheel_direct_mapping");
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &wheel->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);
    assert(((ldBase_t *)ld_progress_wheel)->pInfo == backend);
    assert(tinyui_widget_has_ld_binding(&wheel->widget) == 1);
}

static void test_progress_wheel_percent_bounds(struct picoui_window *win)
{
    struct picoui_progress_wheel *wheel =
        picoui_progress_wheel_create((struct picoui_widget *)win, "wheel_bounds");

    assert(wheel != 0);
    assert(picoui_progress_wheel_set_percent(wheel, 0) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 0);
    assert(picoui_progress_wheel_set_percent(wheel, 100) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 100);
    assert(picoui_progress_wheel_set_percent(wheel, -1) == -1);
    assert(picoui_progress_wheel_get_percent(wheel) == 100);
    assert(picoui_progress_wheel_set_percent(wheel, 101) == -1);
    assert(picoui_progress_wheel_get_percent(wheel) == 100);
}

static void test_progress_wheel_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_progress_wheel *wheel =
        picoui_progress_wheel_create((struct picoui_widget *)win, "wheel_invalid");

    assert(wheel != 0);
    assert(picoui_progress_wheel_create(0, "wheel") == 0);
    assert(picoui_progress_wheel_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_progress_wheel_create_with_props(0,
                                                   &(struct picoui_progress_wheel_props){
                                                       .id = "bad_parent",
                                                       .percent = 0,
                                                   }) == 0);
    assert(picoui_progress_wheel_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_progress_wheel_create_with_props((struct picoui_widget *)win,
                                                   &(struct picoui_progress_wheel_props){
                                                       .percent = 0,
                                                   }) == 0);
    assert(picoui_progress_wheel_create_with_props((struct picoui_widget *)win,
                                                   &(struct picoui_progress_wheel_props){
                                                       .id = "bad_percent_low",
                                                       .percent = -1,
                                                   }) == 0);
    assert(picoui_progress_wheel_create_with_props((struct picoui_widget *)win,
                                                   &(struct picoui_progress_wheel_props){
                                                       .id = "bad_percent_high",
                                                       .percent = 101,
                                                   }) == 0);
    assert(picoui_progress_wheel_set_percent(0, 10) == -1);
    assert(picoui_progress_wheel_get_percent(0) == -1);
    assert(picoui_progress_wheel_set_percent(wheel, -3) == -1);
    assert(picoui_progress_wheel_set_percent(wheel, 130) == -1);
    assert(picoui_progress_wheel_get_percent(wheel) == 0);
}

static void test_progress_wheel_release_contract_covers_animation_and_style_boundary(
    struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;
    struct tinyui_progress_wheel_cfg_bridge *bridge;

    assert(wheel != 0);
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL);
    assert(backend->style_class == (const char *)"wheel");
    assert(backend->user_data != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);
    bridge = (struct tinyui_progress_wheel_cfg_bridge *)&ld_progress_wheel->tWheel;

    assert(picoui_progress_wheel_get_percent(wheel) == 72);
    assert(ld_progress_wheel->iProgress == 720);
    assert(bridge->tCFG.bUseDirtyRegions == false);
    assert(bridge->tCFG.tWheelColour != GLCD_COLOR_WHITE);
    assert(bridge->tCFG.bIgnoreDot == false);
    assert(picoui_progress_wheel_get_dot_enabled(wheel) == 1);
}

static void test_progress_wheel_native_color_and_dot_enable_round_trip(struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;
    struct tinyui_progress_wheel_cfg_bridge *bridge;

    assert(wheel != 0);
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    assert(backend != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);
    bridge = (struct tinyui_progress_wheel_cfg_bridge *)&ld_progress_wheel->tWheel;

    assert(picoui_progress_wheel_set_wheel_color(wheel, 0x123456U) == 0);
    assert(picoui_progress_wheel_set_dot_color(wheel, 0xABCDEFU) == 0);
    assert(picoui_progress_wheel_set_dot_enabled(wheel, 0) == 0);
    assert(picoui_progress_wheel_get_dot_enabled(wheel) == 0);

    assert(bridge->tCFG.tWheelColour != GLCD_COLOR_WHITE);
    assert(bridge->tCFG.tDotColour != GLCD_COLOR_WHITE);
    assert(bridge->tCFG.bIgnoreDot == true);

    assert(picoui_progress_wheel_set_dot_enabled(wheel, 1) == 0);
    assert(picoui_progress_wheel_get_dot_enabled(wheel) == 1);
    assert(bridge->tCFG.bIgnoreDot == false);

    assert(picoui_progress_wheel_set_wheel_color(0, 0x111111U) == -1);
    assert(picoui_progress_wheel_set_dot_color(0, 0x222222U) == -1);
    assert(picoui_progress_wheel_set_dot_enabled(0, 1) == -1);
    assert(picoui_progress_wheel_get_dot_enabled(0) == -1);
}

static void test_progress_wheel_progress_alias_round_trip(struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    assert(backend != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);

    assert(picoui_progress_wheel_set_progress(wheel, 37) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 37);
    assert(ld_progress_wheel->iProgress == 370);
}

static void test_progress_wheel_create_with_props_failure_rolls_back_attached_child(
    struct picoui_window *win)
{
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct picoui_progress_wheel *probe;
    struct tinyui_progress_wheel_test_dispose_snapshot snapshot = {0};

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    tinyui_progress_wheel_test_reset_state();
    probe = picoui_progress_wheel_create((struct picoui_widget *)win, "wheel_fail_percent");
    assert(probe != 0);
    assert(picoui_widget_destroy(&probe->widget) == 0);

    tinyui_progress_wheel_test_fail_next_set_percent();
    assert(picoui_progress_wheel_create_with_props(
               (struct picoui_widget *)win,
               &(struct picoui_progress_wheel_props){
                   .id = "wheel_fail_percent",
                   .style_class = "wheel-fail",
                   .percent = 24,
               }) == 0);
    assert(tinyui_progress_wheel_test_take_last_dispose_snapshot(&snapshot) == 0);
    assert(snapshot.kind == PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL);
    assert(snapshot.cleanup_complete == 1);
    assert(snapshot.cleanup_incomplete == 0);
    assert(snapshot.detach_result == 0);
    assert(snapshot.unbind_result == 0);
    assert(snapshot.detached == 1);
    assert(snapshot.owner_cleared == 1);
    assert(snapshot.root_cleared == 1);
    assert(snapshot.parent_cleared == 1);
    assert(snapshot.next_sibling_cleared == 1);
    assert(snapshot.host_cleared == 1);
    assert(snapshot.event_bridge_cleared == 1);
    assert(snapshot.ld_pinfo_cleared == 1);
    assert(tinyui_progress_wheel_test_take_last_dispose_snapshot(&snapshot) == -1);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }

    tinyui_progress_wheel_test_reset_state();
    assert(picoui_progress_wheel_create_with_props(
               (struct picoui_widget *)win,
               &(struct picoui_progress_wheel_props){
                   .id = "wheel_fail_percent",
                   .style_class = "wheel-fail",
                   .percent = 24,
               }) != 0);
}

static void test_progress_wheel_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_progress_wheel_create(0, "id") == 0);
    assert(picoui_progress_wheel_create(win, 0) == 0);
    assert(picoui_progress_wheel_set_percent(0, 50) == -1);
    assert(picoui_progress_wheel_set_wheel_color(0, 0xFFFFFFU) == -1);
    assert(picoui_progress_wheel_set_dot_color(0, 0x000000U) == -1);
    assert(picoui_progress_wheel_set_dot_enabled(0, 1) == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_progress_wheel *wheel_with_props;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_progress_wheel_create_and_backend_mapping(win);
    wheel_with_props = test_progress_wheel_create_and_props(win);
    test_progress_wheel_percent_bounds(win);
    test_progress_wheel_rejects_invalid_inputs(win);
    test_progress_wheel_release_contract_covers_animation_and_style_boundary(wheel_with_props);
    test_progress_wheel_native_color_and_dot_enable_round_trip(wheel_with_props);
    test_progress_wheel_progress_alias_round_trip(wheel_with_props);
    test_progress_wheel_create_with_props_failure_rolls_back_attached_child(win);
    test_progress_wheel_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
