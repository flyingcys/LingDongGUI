#include "app.h"
#include "progress_wheel.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldProgressWheel.h"
#include "internal.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);
extern void tinyui_progress_wheel_test_reset_state(void);
extern void tinyui_progress_wheel_test_fail_next_set_percent(void);
static char test_progress_wheel_source_path[PATH_MAX];
static struct tinyui_widget g_disposed_backend_snapshot;
static int g_disposed_backend_valid = 0;

static void init_progress_wheel_source_path(void)
{
    const char *source = __FILE__;
    const char *suffix = "tests/tinyui/unit/test_tinyui_progress_wheel.c";
    const char *match = strstr(source, suffix);
    size_t root_len;

    assert(match != 0);
    root_len = (size_t)(match - source);
    assert(root_len + strlen("tinyui/src/widgets/progress_wheel.c") < sizeof(test_progress_wheel_source_path));
    memcpy(test_progress_wheel_source_path, source, root_len);
    snprintf(test_progress_wheel_source_path + root_len,
             sizeof(test_progress_wheel_source_path) - root_len,
             "tinyui/src/widgets/progress_wheel.c");
}

static int test_source_has_symbol(const char *source_path, const char *symbol)
{
    char command[1024];

    assert(source_path != 0);
    assert(symbol != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(0 if sys.argv[2] in text else 1)\n"
             "PY",
             source_path,
             symbol);
    return system(command) == 0;
}

void tinyui_test_capture_destroyed_widget_snapshot(const struct tinyui_widget *widget)
{
    if (widget == 0) {
        memset(&g_disposed_backend_snapshot, 0, sizeof(g_disposed_backend_snapshot));
        g_disposed_backend_valid = 0;
        return;
    }

    g_disposed_backend_snapshot = *widget;
    g_disposed_backend_valid = 1;
}

static const struct tinyui_widget *tinyui_progress_wheel_test_last_disposed_backend(void)
{
    if (g_disposed_backend_valid == 0) {
        return 0;
    }
    return &g_disposed_backend_snapshot;
}

struct __attribute__((may_alias)) tinyui_progress_wheel_cfg_bridge {
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

static struct tinyui_progress_wheel *test_progress_wheel_create_and_props(struct tinyui_window *win)
{
    int user_cookie = 9;
    struct tinyui_progress_wheel_props props = {
        .id = "wheel_props",
        .style_class = "wheel",
        .user_data = &user_cookie,
        .percent = 72,
    };
    struct tinyui_progress_wheel *wheel =
        tinyui_progress_wheel_create((struct tinyui_widget *)win, "wheel");
    struct tinyui_progress_wheel *with_props =
        tinyui_progress_wheel_create_with_props((struct tinyui_widget *)win, &props);

    assert(wheel != 0);
    assert(with_props != 0);
    assert(tinyui_progress_wheel_get_percent(wheel) == 0);
    assert(tinyui_progress_wheel_get_percent(with_props) == props.percent);

    return with_props;
}

static void test_progress_wheel_create_and_backend_mapping(struct tinyui_window *win)
{
    struct tinyui_progress_wheel *wheel =
        tinyui_progress_wheel_create((struct tinyui_widget *)win, "wheel_direct_mapping");
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    backend = &wheel->widget;
    assert(backend->ld_widget != 0);
    parent_backend = &win->widget;
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
    assert(tinyui_widget_has_ld_binding(&wheel->widget) == 1);
}

static void test_progress_wheel_percent_bounds(struct tinyui_window *win)
{
    struct tinyui_progress_wheel *wheel =
        tinyui_progress_wheel_create((struct tinyui_widget *)win, "wheel_bounds");

    assert(wheel != 0);
    assert(tinyui_progress_wheel_set_percent(wheel, 0) == 0);
    assert(tinyui_progress_wheel_get_percent(wheel) == 0);
    assert(tinyui_progress_wheel_set_percent(wheel, 100) == 0);
    assert(tinyui_progress_wheel_get_percent(wheel) == 100);
    assert(tinyui_progress_wheel_set_percent(wheel, -1) == -1);
    assert(tinyui_progress_wheel_get_percent(wheel) == 100);
    assert(tinyui_progress_wheel_set_percent(wheel, 101) == -1);
    assert(tinyui_progress_wheel_get_percent(wheel) == 100);
}

static void test_progress_wheel_rejects_invalid_inputs(struct tinyui_window *win)
{
    struct tinyui_progress_wheel *wheel =
        tinyui_progress_wheel_create((struct tinyui_widget *)win, "wheel_invalid");

    assert(wheel != 0);
    assert(tinyui_progress_wheel_create(0, "wheel") == 0);
    assert(tinyui_progress_wheel_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_progress_wheel_create_with_props(0,
                                                   &(struct tinyui_progress_wheel_props){
                                                       .id = "bad_parent",
                                                       .percent = 0,
                                                   }) == 0);
    assert(tinyui_progress_wheel_create_with_props((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_progress_wheel_create_with_props((struct tinyui_widget *)win,
                                                   &(struct tinyui_progress_wheel_props){
                                                       .percent = 0,
                                                   }) == 0);
    assert(tinyui_progress_wheel_create_with_props((struct tinyui_widget *)win,
                                                   &(struct tinyui_progress_wheel_props){
                                                       .id = "bad_percent_low",
                                                       .percent = -1,
                                                   }) == 0);
    assert(tinyui_progress_wheel_create_with_props((struct tinyui_widget *)win,
                                                   &(struct tinyui_progress_wheel_props){
                                                       .id = "bad_percent_high",
                                                       .percent = 101,
                                                   }) == 0);
    assert(tinyui_progress_wheel_set_percent(0, 10) == -1);
    assert(tinyui_progress_wheel_get_percent(0) == -1);
    assert(tinyui_progress_wheel_set_percent(wheel, -3) == -1);
    assert(tinyui_progress_wheel_set_percent(wheel, 130) == -1);
    assert(tinyui_progress_wheel_get_percent(wheel) == 0);
}

static void test_progress_wheel_release_contract_covers_animation_and_style_boundary(
    struct tinyui_progress_wheel *wheel)
{
    struct tinyui_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;
    struct tinyui_progress_wheel_cfg_bridge *bridge;

    assert(wheel != 0);
    backend = &wheel->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL);
    assert(backend->style_class == (const char *)"wheel");
    assert(backend->user_data != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);
    bridge = (struct tinyui_progress_wheel_cfg_bridge *)&ld_progress_wheel->tWheel;

    assert(tinyui_progress_wheel_get_percent(wheel) == 72);
    assert(ld_progress_wheel->iProgress == 720);
    assert(bridge->tCFG.bUseDirtyRegions == false);
    assert(bridge->tCFG.tWheelColour != GLCD_COLOR_WHITE);
    assert(bridge->tCFG.bIgnoreDot == false);
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 1);
}

static void test_progress_wheel_native_color_and_dot_enable_round_trip(struct tinyui_progress_wheel *wheel)
{
    struct tinyui_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;
    struct tinyui_progress_wheel_cfg_bridge *bridge;

    assert(wheel != 0);
    backend = &wheel->widget;
    assert(backend->ld_widget != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);
    bridge = (struct tinyui_progress_wheel_cfg_bridge *)&ld_progress_wheel->tWheel;

    assert(tinyui_progress_wheel_set_wheel_color(wheel, 0x123456U) == 0);
    assert(tinyui_progress_wheel_set_dot_color(wheel, 0xABCDEFU) == 0);
    assert(tinyui_progress_wheel_set_dot_enabled(wheel, 0) == 0);
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 0);

    assert(bridge->tCFG.tWheelColour != GLCD_COLOR_WHITE);
    assert(bridge->tCFG.tDotColour != GLCD_COLOR_WHITE);
    assert(bridge->tCFG.bIgnoreDot == true);

    assert(tinyui_progress_wheel_set_dot_enabled(wheel, 1) == 0);
    assert(tinyui_progress_wheel_get_dot_enabled(wheel) == 1);
    assert(bridge->tCFG.bIgnoreDot == false);

    assert(tinyui_progress_wheel_set_wheel_color(0, 0x111111U) == -1);
    assert(tinyui_progress_wheel_set_dot_color(0, 0x222222U) == -1);
    assert(tinyui_progress_wheel_set_dot_enabled(0, 1) == -1);
    assert(tinyui_progress_wheel_get_dot_enabled(0) == -1);
}

static void test_progress_wheel_progress_alias_round_trip(struct tinyui_progress_wheel *wheel)
{
    struct tinyui_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    backend = &wheel->widget;
    assert(backend->ld_widget != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);

    assert(tinyui_progress_wheel_set_progress(wheel, 37) == 0);
    __asm__ volatile("" ::: "memory");
    assert(tinyui_progress_wheel_get_percent(wheel) == 37);
    assert(ld_progress_wheel->iProgress == 370);
}

static void test_progress_wheel_create_with_props_failure_rolls_back_attached_child(
    struct tinyui_window *win)
{
    ldBase_t *win_ld = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    struct tinyui_progress_wheel *probe;
    const struct tinyui_widget *disposed_backend;

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_progress_wheel_test_reset_state();
    tinyui_test_capture_destroyed_widget_snapshot(0);
    probe = tinyui_progress_wheel_create((struct tinyui_widget *)win, "wheel_fail_percent");
    assert(probe != 0);
    assert(tinyui_widget_destroy(&probe->widget) == 0);

    tinyui_progress_wheel_test_fail_next_set_percent();
    assert(tinyui_progress_wheel_create_with_props(
               (struct tinyui_widget *)win,
               &(struct tinyui_progress_wheel_props){
                   .id = "wheel_fail_percent",
                   .style_class = "wheel-fail",
                   .percent = 24,
               }) == 0);
    disposed_backend = tinyui_progress_wheel_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL);
    assert(disposed_backend->owner == 0);
    assert(disposed_backend->ld_event_bridge_scene == 0);
    assert(disposed_backend->ld_event_bridge_sender == 0);
    assert(disposed_backend->ld_event_bridge_next == 0);
    assert(disposed_backend->ld_widget == 0);
    if (tail_ld != 0) {
        assert(ldBaseGetNextSibling(tail_ld) == next_before_ld);
    } else {
        assert(ldBaseGetChildList(win_ld) == 0);
    }

    tinyui_progress_wheel_test_reset_state();
    assert(tinyui_progress_wheel_create_with_props(
               (struct tinyui_widget *)win,
               &(struct tinyui_progress_wheel_props){
                   .id = "wheel_fail_percent",
                   .style_class = "wheel-fail",
                   .percent = 24,
               }) != 0);
}

static void test_progress_wheel_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_progress_wheel_create(0, "id") == 0);
    assert(tinyui_progress_wheel_create(win, 0) == 0);
    assert(tinyui_progress_wheel_set_percent(0, 50) == -1);
    assert(tinyui_progress_wheel_set_wheel_color(0, 0xFFFFFFU) == -1);
    assert(tinyui_progress_wheel_set_dot_color(0, 0x000000U) == -1);
    assert(tinyui_progress_wheel_set_dot_enabled(0, 1) == -1);
}

static void test_progress_wheel_create_uses_shared_leaf_helper(void)
{
    assert(test_source_has_symbol(test_progress_wheel_source_path, "tinyui_widget_create_leaf"));
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_progress_wheel *wheel_with_props;

    init_progress_wheel_source_path();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
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
    test_progress_wheel_create_uses_shared_leaf_helper();

    tinyui_app_destroy(app);
    return 0;
}
