#include "internal/app_legacy.h"
#include "widgets/progress_bar.h"
#include "internal/widget_legacy.h"
#include "widgets/window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldProgressBar.h"
#include "internal.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);
static struct tinyui_widget g_disposed_backend_snapshot;
static int g_disposed_backend_valid = 0;

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

static const struct tinyui_widget *tinyui_progress_bar_test_last_disposed_backend(void)
{
    if (g_disposed_backend_valid == 0) {
        return 0;
    }
    return &g_disposed_backend_snapshot;
}

__attribute__((weak)) struct tinyui_progress_bar *
tinyui_progress_bar_test_create_with_props_fail_before_inverted(
    struct tinyui_window *parent,
    const struct tinyui_progress_bar_props *props)
{
    (void)parent;
    (void)props;
    return 0;
}

__attribute__((weak)) void tinyui_progress_bar_test_destroy(
    struct tinyui_progress_bar *bar)
{
    (void)bar;
}

static void test_progress_bar_create_and_backend_mapping(struct tinyui_window *win)
{
    const char *progress_bar_widget_source_path =
        tinyui_test_repo_path_from_file(__FILE__, "tinyui/src/widgets/progress_bar.c");
    struct tinyui_progress_bar *bar = tinyui_progress_bar_create(win, "progress_direct_mapping");
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldProgressBar_t *ld_progress_bar;

    assert(bar != 0);
    backend = &bar->widget;
    assert(backend->ld_widget != 0);
    parent_backend = &win->widget;
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_BAR);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_progress_bar = (ldProgressBar_t *)backend->ld_widget;
    assert(ld_progress_bar != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
    assert(tinyui_widget_has_ld_binding(&bar->widget) == 1);
    assert(tinyui_test_source_contains(progress_bar_widget_source_path,
                                       "tinyui_progress_bar_test_create_with_props_fail_before_inverted") == 1);
    assert(tinyui_test_source_contains(progress_bar_widget_source_path,
                                       "tinyui_progress_bar_create_with_props_impl") == 1);
    assert(tinyui_test_source_contains(progress_bar_widget_source_path,
                                       "tinyui_backend_progress_bar_test_create_with_props_fail_before_inverted") == 0);
    assert(tinyui_test_source_contains(progress_bar_widget_source_path,
                                       "tinyui_progress_bar_test_take_last_dispose_snapshot(") == 0);
}

static void test_progress_bar_create_and_props(struct tinyui_window *win)
{
    int user_cookie = 42;
    struct tinyui_progress_bar_props props = {
        .id = "progress_with_props",
        .style_class = "meter",
        .user_data = &user_cookie,
        .percent = 65,
        .horizontal = 1,
    };
    struct tinyui_progress_bar *bar = tinyui_progress_bar_create(win, "progress");
    struct tinyui_progress_bar *bar_with_props = tinyui_progress_bar_create_with_props(win, &props);

    assert(bar != 0);
    assert(bar_with_props != 0);
    assert(tinyui_progress_bar_get_percent(bar) == 0);
    assert(tinyui_progress_bar_get_percent(bar_with_props) == 65);
    assert(tinyui_progress_bar_get_horizontal(bar) == 0);
    assert(tinyui_progress_bar_get_horizontal(bar_with_props) == 1);
}

static void test_progress_bar_rejects_invalid_inputs(struct tinyui_window *win)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_create(win, "progress_invalid");

    assert(bar != 0);
    assert(tinyui_progress_bar_create(0, "progress") == 0);
    assert(tinyui_progress_bar_create(win, 0) == 0);
    assert(tinyui_progress_bar_create_with_props(0,
                                                 &(struct tinyui_progress_bar_props){
                                                     .id = "bad_parent",
                                                     .percent = 0,
                                                 }) == 0);
    assert(tinyui_progress_bar_create_with_props(win, 0) == 0);
    assert(tinyui_progress_bar_create_with_props(win,
                                                 &(struct tinyui_progress_bar_props){
                                                     .percent = 0,
                                                 }) == 0);
    assert(tinyui_progress_bar_create_with_props(win,
                                                 &(struct tinyui_progress_bar_props){
                                                     .id = "bad_percent_low",
                                                     .percent = -1,
                                                 }) == 0);
    assert(tinyui_progress_bar_create_with_props(win,
                                                 &(struct tinyui_progress_bar_props){
                                                     .id = "bad_percent_high",
                                                     .percent = 101,
                                                 }) == 0);
    assert(tinyui_progress_bar_set_percent(0, 10) == -1);
    assert(tinyui_progress_bar_get_percent(0) == -1);
    assert(tinyui_progress_bar_set_horizontal(0, 1) == -1);
    assert(tinyui_progress_bar_get_horizontal(0) == -1);
    assert(tinyui_progress_bar_set_percent(bar, -3) == -1);
    assert(tinyui_progress_bar_set_percent(bar, 130) == -1);
    assert(tinyui_progress_bar_get_percent(bar) == 0);
}

static void test_progress_bar_percent_bounds(struct tinyui_window *win)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_create(win, "progress_bounds");

    assert(bar != 0);
    assert(tinyui_progress_bar_set_percent(bar, 0) == 0);
    assert(tinyui_progress_bar_get_percent(bar) == 0);
    assert(tinyui_progress_bar_set_percent(bar, 100) == 0);
    assert(tinyui_progress_bar_get_percent(bar) == 100);
    assert(tinyui_progress_bar_set_percent(bar, -1) == -1);
    assert(tinyui_progress_bar_get_percent(bar) == 100);
    assert(tinyui_progress_bar_set_percent(bar, 101) == -1);
    assert(tinyui_progress_bar_get_percent(bar) == 100);
}

static void test_progress_bar_horizontal_state(struct tinyui_window *win)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_create(win, "progress_horizontal");

    assert(bar != 0);
    assert(tinyui_progress_bar_get_horizontal(bar) == 0);
    assert(tinyui_progress_bar_set_horizontal(bar, 1) == 0);
    assert(tinyui_progress_bar_get_horizontal(bar) == 1);
    assert(tinyui_progress_bar_set_horizontal(bar, 7) == 0);
    assert(tinyui_progress_bar_get_horizontal(bar) == 1);
    assert(tinyui_progress_bar_set_horizontal(bar, 0) == 0);
    assert(tinyui_progress_bar_get_horizontal(bar) == 0);
    assert(tinyui_progress_bar_set_horizontal(bar, -5) == 0);
    assert(tinyui_progress_bar_get_horizontal(bar) == 1);
}

static void test_progress_bar_create_with_props_failure_rolls_back_attached_child(struct tinyui_window *win)
{
    ldBase_t *win_ld = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    struct tinyui_progress_bar *probe;
    struct tinyui_progress_bar *bar;
    const struct tinyui_widget *disposed_backend;

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_test_capture_destroyed_widget_snapshot(0);
    probe = tinyui_progress_bar_create(win, "progress_fail_inverted");
    assert(probe != 0);
    assert(tinyui_widget_destroy(&probe->widget) == 0);

    bar = tinyui_progress_bar_test_create_with_props_fail_before_inverted(
        win,
        &(struct tinyui_progress_bar_props){
            .id = "progress_fail_inverted",
            .percent = 15,
            .horizontal = 1,
            .inverted = 1,
        });

    assert(bar == 0);
    disposed_backend = tinyui_progress_bar_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_BAR);
    assert(disposed_backend->owner == 0);
    assert(disposed_backend->ld_event_bridge_scene == 0);
    assert(disposed_backend->ld_event_bridge_sender == 0);
    assert(disposed_backend->ld_widget == 0);
    if (tail_ld != 0) {
        assert(ldBaseGetNextSibling(tail_ld) == next_before_ld);
    } else {
        assert(ldBaseGetChildList(win_ld) == 0);
    }

    tinyui_test_capture_destroyed_widget_snapshot(0);
    assert(tinyui_progress_bar_create_with_props(
               win,
               &(struct tinyui_progress_bar_props){
                   .id = "progress_fail_inverted",
                   .percent = 15,
                   .horizontal = 1,
                   .inverted = 1,
               }) != 0);
}

static void test_progress_bar_dispose_partial_snapshot_marks_cleanup_complete(
    struct tinyui_window *win)
{
    struct tinyui_progress_bar *bar;
    const struct tinyui_widget *disposed_backend;

    tinyui_test_capture_destroyed_widget_snapshot(0);
    bar = tinyui_progress_bar_create(win, "progress_detach_fail");
    assert(bar != 0);

    tinyui_progress_bar_test_destroy(bar);
    disposed_backend = tinyui_progress_bar_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_BAR);
    assert(disposed_backend->owner == 0);
    assert(disposed_backend->ld_event_bridge_scene == 0);
    assert(disposed_backend->ld_event_bridge_sender == 0);
    assert(disposed_backend->ld_widget == 0);
}

static void test_progress_bar_release_contract_covers_theme_and_config_boundary(
    struct tinyui_window *win)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_create_with_props(
        win,
        &(struct tinyui_progress_bar_props){
            .id = "progress_release_ready",
            .style_class = "meter",
            .percent = 40,
            .horizontal = 1,
        });
    struct tinyui_widget *backend;
    ldProgressBar_t *ld_progress_bar;

    assert(bar != 0);
    backend = &bar->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_PROGRESS_BAR);
    assert(backend->style_class != 0);
    assert(strcmp(backend->style_class, "meter") == 0);
    ld_progress_bar = (ldProgressBar_t *)backend->ld_widget;
    assert(ld_progress_bar != 0);

    assert(tinyui_progress_bar_get_percent(bar) == 40);
    assert(tinyui_progress_bar_get_horizontal(bar) == 1);
    assert(ld_progress_bar->permille == 400U);
    assert(ld_progress_bar->isHorizontal == true);
    assert(ld_progress_bar->bgColor != ld_progress_bar->fgColor);
    assert(ld_progress_bar->frameColorSize == 1);
    assert(ld_progress_bar->frameColor != ld_progress_bar->bgColor);
}

static void test_progress_bar_native_skin_color_and_inverted_round_trip(struct tinyui_window *win)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_create(win, "progress_native_skin");
    struct tinyui_widget *backend;
    ldProgressBar_t *ld_progress_bar;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask = {0};
    arm_2d_tile_t fg_tile = {0};
    arm_2d_tile_t fg_mask = {0};
    arm_2d_tile_t frame_tile = {0};
    arm_2d_tile_t frame_mask = {0};
    struct tinyui_image_source bg_source = {
        .img_tile = &bg_tile,
        .mask_tile = &bg_mask,
    };
    struct tinyui_image_source fg_source = {
        .img_tile = &fg_tile,
        .mask_tile = &fg_mask,
    };
    struct tinyui_image_source frame_source = {
        .img_tile = &frame_tile,
        .mask_tile = &frame_mask,
    };
    struct tinyui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = &bg_mask,
    };

    assert(bar != 0);
    backend = &bar->widget;
    assert(backend->ld_widget != 0);
    ld_progress_bar = (ldProgressBar_t *)backend->ld_widget;
    assert(ld_progress_bar != 0);

    assert(tinyui_progress_bar_set_bg_source(bar, &bg_source) == 0);
    assert(tinyui_progress_bar_set_fg_source(bar, &fg_source) == 0);
    assert(tinyui_progress_bar_set_frame_source(bar, &frame_source) == 0);
    assert(ld_progress_bar->ptBgImgTile == &bg_tile);
    assert(ld_progress_bar->ptBgMaskTile == &bg_mask);
    assert(ld_progress_bar->ptFgImgTile == &fg_tile);
    assert(ld_progress_bar->ptFgMaskTile == &fg_mask);
    assert(ld_progress_bar->ptFrameImgTile == &frame_tile);
    assert(ld_progress_bar->ptFrameMaskTile == &frame_mask);

    assert(tinyui_progress_bar_set_color(bar, 0x102030u, 0xa0b0c0u) == 0);
    assert(tinyui_progress_bar_set_frame_color(bar, 0x556677u, 3) == 0);
    assert(tinyui_progress_bar_set_inverted(bar, 1) == 0);
    assert(tinyui_progress_bar_get_inverted(bar) == 1);

    assert(ld_progress_bar->ptBgImgTile == 0);
    assert(ld_progress_bar->ptFgImgTile == 0);
    assert(ld_progress_bar->ptFrameImgTile == 0);
    assert(ld_progress_bar->frameColorSize == 3);
    assert(ld_progress_bar->isInverted == true);

    assert(tinyui_progress_bar_set_bg_source(bar, &invalid_source) == -1);
    assert(tinyui_progress_bar_set_frame_color(bar, 0x112233u, -1) == -1);
    assert(tinyui_progress_bar_set_inverted(0, 1) == -1);
    assert(tinyui_progress_bar_get_inverted(0) == -1);

    assert(ld_progress_bar->ptBgImgTile == 0);
    assert(ld_progress_bar->ptFgImgTile == 0);
    assert(ld_progress_bar->frameColorSize == 3);
    assert(ld_progress_bar->isInverted == true);
}

static void test_progress_bar_init_image_and_shared_base_aliases_round_trip(struct tinyui_window *win)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_create(win, "progress_alias");
    struct tinyui_widget *backend;
    ldProgressBar_t *ld_progress_bar;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask = {0};
    arm_2d_tile_t fg_tile = {0};
    arm_2d_tile_t fg_mask = {0};
    arm_2d_tile_t frame_tile = {0};
    arm_2d_tile_t frame_mask = {0};
    struct tinyui_image_source bg_source = {
        .img_tile = &bg_tile,
        .mask_tile = &bg_mask,
    };
    struct tinyui_image_source fg_source = {
        .img_tile = &fg_tile,
        .mask_tile = &fg_mask,
    };
    struct tinyui_image_source frame_source = {
        .img_tile = &frame_tile,
        .mask_tile = &frame_mask,
    };

    assert(bar != 0);
    backend = &bar->widget;
    assert(backend->ld_widget != 0);
    ld_progress_bar = (ldProgressBar_t *)backend->ld_widget;
    assert(ld_progress_bar != 0);

    assert(tinyui_progress_bar_set_image(bar, &bg_source, &fg_source) == 0);
    assert(ld_progress_bar->ptBgImgTile == &bg_tile);
    assert(ld_progress_bar->ptBgMaskTile == &bg_mask);
    assert(ld_progress_bar->ptFgImgTile == &fg_tile);
    assert(ld_progress_bar->ptFgMaskTile == &fg_mask);

    assert(tinyui_progress_bar_set_frame_source(bar, &frame_source) == 0);
    assert(ld_progress_bar->ptFrameImgTile == &frame_tile);
    assert(ld_progress_bar->ptFrameMaskTile == &frame_mask);

    assert(tinyui_widget_set_pos(&bar->widget, 9, 12) == 0);
    assert(((ldBase_t *)ld_progress_bar)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 9);
    assert(((ldBase_t *)ld_progress_bar)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 12);

    assert(tinyui_widget_set_visible(&bar->widget, 0) == 0);
    assert(((ldBase_t *)ld_progress_bar)->isHidden == true);
    assert(tinyui_widget_set_opacity(&bar->widget, 66) == 0);
    assert(((ldBase_t *)ld_progress_bar)->opacity == 66);
    assert(tinyui_widget_set_selectable(&bar->widget, 1) == 0);
    assert(((ldBase_t *)ld_progress_bar)->isSelectable == true);
    assert(tinyui_widget_set_selected(&bar->widget, 1) == 0);
    assert(((ldBase_t *)ld_progress_bar)->isSelected == true);
    assert(tinyui_widget_set_selectable(&bar->widget, 0) == 0);
    assert(((ldBase_t *)ld_progress_bar)->isSelectable == false);
    assert(tinyui_widget_set_corner(&bar->widget, 7) == 0);
    assert(((ldBase_t *)ld_progress_bar)->isCorner == true);
}

static void test_progress_bar_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_progress_bar_create(0, "id") == 0);
    assert(tinyui_progress_bar_create(win, 0) == 0);
    assert(tinyui_progress_bar_set_percent(0, 50) == -1);
    assert(tinyui_progress_bar_get_percent(0) == -1);
    assert(tinyui_progress_bar_set_horizontal(0, 1) == -1);
    assert(tinyui_progress_bar_set_inverted(0, 1) == -1);
    assert(tinyui_progress_bar_get_inverted(0) == -1);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_progress_bar_create_and_backend_mapping(win);
    test_progress_bar_create_and_props(win);
    test_progress_bar_percent_bounds(win);
    test_progress_bar_horizontal_state(win);
    test_progress_bar_create_with_props_failure_rolls_back_attached_child(win);
    test_progress_bar_dispose_partial_snapshot_marks_cleanup_complete(win);
    test_progress_bar_rejects_invalid_inputs(win);
    test_progress_bar_release_contract_covers_theme_and_config_boundary(win);
    test_progress_bar_native_skin_color_and_inverted_round_trip(win);
    test_progress_bar_init_image_and_shared_base_aliases_round_trip(win);
    test_progress_bar_rejects_null_args(win);

    tinyui_app_destroy(app);
    return 0;
}
