#include "core/app.h"
#include "widgets/gauge.h"
#include "core/widget.h"
#include "widgets/window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldGauge.h"
#include "internal.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *ldMalloc(uint32_t size)
{
    return malloc((size_t)size);
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    return calloc((size_t)num, (size_t)size);
}

void *ldRealloc(void *ptr, uint32_t newSize)
{
    return realloc(ptr, (size_t)newSize);
}

static unsigned int test_rgb_round_trip(unsigned int rgb)
{
    ldColor color = __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
    unsigned int red = ((unsigned int)color >> 11) & 0x1FU;
    unsigned int green = ((unsigned int)color >> 5) & 0x3FU;
    unsigned int blue = (unsigned int)color & 0x1FU;

    red = (red << 3) | (red >> 2);
    green = (green << 2) | (green >> 4);
    blue = (blue << 3) | (blue >> 2);
    return (red << 16) | (green << 8) | blue;
}

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

static const struct tinyui_widget *tinyui_gauge_test_last_disposed_backend(void)
{
    if (g_disposed_backend_valid == 0) {
        return 0;
    }
    return &g_disposed_backend_snapshot;
}

__attribute__((weak)) struct tinyui_gauge *
tinyui_gauge_test_create_with_props_fail_before_centre_offset(
    struct tinyui_widget *parent,
    const struct tinyui_gauge_props *props)
{
    (void)parent;
    (void)props;
    return 0;
}

struct tracked_free_entry {
    void *ptr;
    int count;
};

static struct tracked_free_entry g_tracked_frees[24];
static int g_tracked_free_count = 0;

static void tracked_free_reset(void)
{
    int i;

    for (i = 0; i < 24; ++i) {
        g_tracked_frees[i].ptr = NULL;
        g_tracked_frees[i].count = 0;
    }
    g_tracked_free_count = 0;
}

static void tracked_free_watch(void *ptr)
{
    assert(g_tracked_free_count < 24);
    g_tracked_frees[g_tracked_free_count].ptr = ptr;
    g_tracked_frees[g_tracked_free_count].count = 0;
    g_tracked_free_count++;
}

static int tracked_free_count_for(void *ptr)
{
    int i;

    for (i = 0; i < g_tracked_free_count; ++i) {
        if (g_tracked_frees[i].ptr == ptr) {
            return g_tracked_frees[i].count;
        }
    }
    return 0;
}

void ldFree(void *p)
{
    int i;

    if (p == NULL) {
        return;
    }

    for (i = 0; i < g_tracked_free_count; ++i) {
        if (g_tracked_frees[i].ptr == p) {
            g_tracked_frees[i].count++;
        }
    }
    free(p);
}

static void test_gauge_create_and_props(struct tinyui_window *win)
{
    struct tinyui_gauge_props props = {
        .id = "gauge_props",
        .angle = 45.0f,
        .pointer_color = 0x334455,
        .auto_move = 1,
    };
    struct tinyui_gauge *gauge = tinyui_gauge_create((struct tinyui_widget *)win, "gauge");
    struct tinyui_gauge *with_props =
        tinyui_gauge_create_with_props((struct tinyui_widget *)win, &props);

    assert(gauge != 0);
    assert(with_props != 0);
    assert(tinyui_gauge_get_angle(gauge) == 0.0f);
    assert(tinyui_gauge_get_pointer_color(gauge) == 0x000000);
    assert(tinyui_gauge_get_auto_move(gauge) == 0);
    assert(tinyui_gauge_get_angle(with_props) == props.angle);
    assert(tinyui_gauge_get_pointer_color(with_props) == test_rgb_round_trip(props.pointer_color));
    assert(tinyui_gauge_get_auto_move(with_props) == props.auto_move);
}

static void test_gauge_internal_seam_names_are_gone(void)
{
    const char *source_path = "tinyui/src/widgets/gauge.c";
    const char *test_source_path = "tests/tinyui/unit/test_tinyui_gauge.c";

    assert(source_path != NULL);
    assert(test_source_path != NULL);
    assert(tinyui_test_source_contains(source_path,
                                       "tinyui_gauge_test_take_last_dispose_snapshot(") == 0);
}

static void test_gauge_create_and_backend_mapping(struct tinyui_window *win)
{
    struct tinyui_gauge *gauge = tinyui_gauge_create((struct tinyui_widget *)win, "gauge_direct_mapping");
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldGauge_t *ld_gauge;

    assert(gauge != 0);
    backend = &gauge->widget;
    assert(backend != 0);
    parent_backend = &win->widget;
    assert(parent_backend != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_GAUGE);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
    assert(tinyui_widget_has_ld_binding(&gauge->widget) == 1);
}

static void test_gauge_value_and_pointer_contract_match_backend_truth(struct tinyui_window *win)
{
    struct tinyui_gauge *gauge = tinyui_gauge_create((struct tinyui_widget *)win, "gauge_angle");

    assert(gauge != 0);
    assert(tinyui_gauge_set_angle(gauge, 90.0f) == 0);
    assert(tinyui_gauge_get_angle(gauge) == 90.0f);
    assert(tinyui_gauge_set_pointer_color(gauge, 0xAABBCC) == 0);
    assert(tinyui_gauge_get_pointer_color(gauge) == test_rgb_round_trip(0xAABBCCU));
    assert(tinyui_gauge_set_auto_move(gauge, 1) == 0);
    assert(tinyui_gauge_get_auto_move(gauge) == 1);
    assert(tinyui_gauge_set_auto_move(gauge, 0) == 0);
    assert(tinyui_gauge_get_auto_move(gauge) == 0);
}

static void test_gauge_rejects_invalid_inputs(struct tinyui_window *win)
{
    struct tinyui_gauge *gauge = tinyui_gauge_create((struct tinyui_widget *)win, "gauge_invalid");

    assert(gauge != 0);
    assert(tinyui_gauge_create(0, "gauge") == 0);
    assert(tinyui_gauge_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_gauge_create_with_props(0,
                                          &(struct tinyui_gauge_props){
                                              .id = "bad_parent",
                                          }) == 0);
    assert(tinyui_gauge_create_with_props((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_gauge_create_with_props((struct tinyui_widget *)win,
                                          &(struct tinyui_gauge_props){
                                              .angle = 30.0f,
                                          }) == 0);
    assert(tinyui_gauge_set_angle(0, 0.0f) == -1);
    assert(tinyui_gauge_set_pointer_color(0, 0x0) == -1);
    assert(tinyui_gauge_set_auto_move(0, 0) == -1);
    assert(tinyui_gauge_get_angle(0) == 0.0f);
    assert(tinyui_gauge_get_pointer_color(0) == 0x000000);
    assert(tinyui_gauge_get_auto_move(0) == -1);
    assert(tinyui_gauge_set_auto_move(gauge, 7) == 0);
    assert(tinyui_gauge_get_auto_move(gauge) == 1);
}

static void test_gauge_native_background_pointer_and_centre_offset_round_trip(struct tinyui_window *win)
{
    struct tinyui_gauge *gauge = tinyui_gauge_create((struct tinyui_widget *)win, "gauge_native_resources");
    struct tinyui_widget *backend;
    ldGauge_t *ld_gauge;
    arm_2d_tile_t bg_img = {
        .tRegion = {
            .tSize = { .iWidth = 30, .iHeight = 30 },
        },
    };
    arm_2d_tile_t bg_mask = {
        .tRegion = {
            .tSize = { .iWidth = 30, .iHeight = 30 },
        },
    };
    arm_2d_tile_t pointer_img = {
        .tRegion = {
            .tSize = { .iWidth = 11, .iHeight = 26 },
        },
    };
    arm_2d_tile_t pointer_mask = {
        .tRegion = {
            .tSize = { .iWidth = 11, .iHeight = 26 },
        },
    };
    struct tinyui_image_source bg_source = {
        .img_tile = &bg_img,
        .mask_tile = &bg_mask,
    };
    struct tinyui_image_source pointer_source = {
        .img_tile = &pointer_img,
        .mask_tile = &pointer_mask,
    };

    assert(gauge != 0);
    backend = &gauge->widget;
    assert(backend->ld_widget != 0);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);

    assert(tinyui_gauge_set_bg_source(gauge, &bg_source) == 0);
    assert(tinyui_gauge_set_pointer_source(gauge, &pointer_source) == 0);
    assert(tinyui_gauge_set_centre_offset(gauge, -7, 9) == 0);

    assert(ld_gauge->ptBgImgTile == &bg_img);
    assert(ld_gauge->ptBgMaskTile == &bg_mask);
    assert(ld_gauge->ptPointerImgTile == &pointer_img);
    assert(ld_gauge->ptPointerMaskTile == &pointer_mask);
    assert(ld_gauge->centreOffsetX == -7);
    assert(ld_gauge->centreOffsetY == 9);

    assert(tinyui_gauge_set_bg_source(0, &bg_source) == -1);
    assert(tinyui_gauge_set_pointer_source(0, &pointer_source) == -1);
    assert(tinyui_gauge_set_centre_offset(0, 1, 2) == -1);
}

static void test_gauge_native_trail_and_progress_bar_round_trip(struct tinyui_window *win)
{
    struct tinyui_gauge *gauge = tinyui_gauge_create((struct tinyui_widget *)win, "gauge_native_trail");
    struct tinyui_widget *backend;
    ldGauge_t *ld_gauge;
    arm_2d_tile_t bg_trail_mask = {
        .tRegion = {
            .tSize = { .iWidth = 40, .iHeight = 40 },
        },
    };
    arm_2d_tile_t pointer_trail_mask = {
        .tRegion = {
            .tSize = { .iWidth = 13, .iHeight = 29 },
        },
    };
    struct tinyui_image_source bg_trail_source = {
        .img_tile = &bg_trail_mask,
        .mask_tile = &bg_trail_mask,
    };
    struct tinyui_image_source pointer_trail_source = {
        .img_tile = &pointer_trail_mask,
        .mask_tile = &pointer_trail_mask,
    };

    assert(gauge != 0);
    backend = &gauge->widget;
    assert(backend->ld_widget != 0);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);

    assert(tinyui_gauge_set_trail(gauge, &bg_trail_source, &pointer_trail_source) == 0);
    assert(ld_gauge->ptBgTrailMaskTile == &bg_trail_mask);
    assert(ld_gauge->ptPointerTrailMaskTile == &pointer_trail_mask);
    assert(ld_gauge->isProgressBar == false);

    assert(tinyui_gauge_set_progress_bar(gauge, &bg_trail_source, &pointer_trail_source) == 0);
    assert(ld_gauge->ptBgTrailMaskTile == &bg_trail_mask);
    assert(ld_gauge->ptPointerTrailMaskTile == &pointer_trail_mask);
    assert(ld_gauge->isProgressBar == true);

    assert(tinyui_gauge_set_trail(0, &bg_trail_source, &pointer_trail_source) == -1);
    assert(tinyui_gauge_set_progress_bar(0, &bg_trail_source, &pointer_trail_source) == -1);
}

static void test_gauge_init_and_shared_base_aliases_round_trip(struct tinyui_window *win)
{
    struct tinyui_gauge *gauge = tinyui_gauge_init((struct tinyui_widget *)win, "gauge_alias");
    struct tinyui_widget *backend;
    ldGauge_t *ld_gauge;

    assert(gauge != 0);
    backend = &gauge->widget;
    assert(backend->ld_widget != 0);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);

    assert(tinyui_gauge_get_angle(gauge) == 0.0f);

    assert(tinyui_widget_set_pos(&gauge->widget, 14, 18) == 0);
    assert(((ldBase_t *)ld_gauge)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 14);
    assert(((ldBase_t *)ld_gauge)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 18);

    assert(tinyui_widget_set_visible(&gauge->widget, 0) == 0);
    assert(((ldBase_t *)ld_gauge)->isHidden == true);
    assert(tinyui_widget_set_opacity(&gauge->widget, 71) == 0);
    assert(((ldBase_t *)ld_gauge)->opacity == 71);
    assert(tinyui_widget_set_selectable(&gauge->widget, 1) == 0);
    assert(((ldBase_t *)ld_gauge)->isSelectable == true);
    assert(tinyui_widget_set_selected(&gauge->widget, 1) == 0);
    assert(((ldBase_t *)ld_gauge)->isSelected == true);
    assert(tinyui_widget_set_selectable(&gauge->widget, 0) == 0);
    assert(((ldBase_t *)ld_gauge)->isSelectable == false);
    assert(tinyui_widget_set_corner(&gauge->widget, 6) == 0);
    assert(((ldBase_t *)ld_gauge)->isCorner == true);
}

static void test_gauge_create_with_props_failure_rolls_back_attached_child(struct tinyui_window *win)
{
    ldBase_t *win_ld = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    struct tinyui_gauge *gauge;
    const struct tinyui_widget *disposed_backend;

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_test_capture_destroyed_widget_snapshot(0);
    gauge = tinyui_gauge_test_create_with_props_fail_before_centre_offset(
        (struct tinyui_widget *)win,
        &(struct tinyui_gauge_props){
            .id = "gauge_fail_centre_offset",
            .angle = 70.0f,
            .centre_offset_x = 4,
            .centre_offset_y = -6,
            .pointer_color = 0x334455U,
            .auto_move = 1,
        });

    assert(gauge == 0);
    disposed_backend = tinyui_gauge_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == TINYUI_BACKEND_WIDGET_GAUGE);
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
}

static void test_gauge_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_gauge_create(0, "id") == 0);
    assert(tinyui_gauge_create(win, 0) == 0);
    assert(tinyui_gauge_set_angle(0, 90.0f) == -1);
    assert(tinyui_gauge_get_angle(0) == 0.0f);
    assert(tinyui_gauge_set_pointer_color(0, 0xFFFFFFU) == -1);
    assert(tinyui_gauge_get_pointer_color(0) == 0x000000);
    assert(tinyui_gauge_set_auto_move(0, 1) == -1);
    assert(tinyui_gauge_get_auto_move(0) == -1);
}

static void test_gauge_destroy_releases_owned_tiles_without_freeing_external_sources(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_gauge *gauge;
    struct tinyui_widget *backend;
    ldGauge_t *ld_gauge;
    arm_2d_tile_t *default_bg_img_tile;
    arm_2d_tile_t *default_bg_mask_tile;
    arm_2d_tile_t *default_pointer_img_tile;
    arm_2d_tile_t *default_pointer_mask_tile;
    arm_2d_tile_t external_bg_img_tile = {
        .tRegion = {
            .tSize = { .iWidth = 30, .iHeight = 30 },
        },
    };
    arm_2d_tile_t external_bg_mask_tile = {
        .tRegion = {
            .tSize = { .iWidth = 30, .iHeight = 30 },
        },
    };
    arm_2d_tile_t external_pointer_img_tile = {
        .tRegion = {
            .tSize = { .iWidth = 11, .iHeight = 26 },
        },
    };
    arm_2d_tile_t external_pointer_mask_tile = {
        .tRegion = {
            .tSize = { .iWidth = 11, .iHeight = 26 },
        },
    };
    struct tinyui_image_source bg_source = {
        .img_tile = &external_bg_img_tile,
        .mask_tile = &external_bg_mask_tile,
    };
    struct tinyui_image_source pointer_source = {
        .img_tile = &external_pointer_img_tile,
        .mask_tile = &external_pointer_mask_tile,
    };

    tracked_free_reset();

    assert(app != 0);
    win = tinyui_window_create(app, "gauge_destroy_root");
    assert(win != 0);
    gauge = tinyui_gauge_create((struct tinyui_widget *)win, "gauge_destroy");
    assert(gauge != 0);
    backend = &gauge->widget;
    assert(backend->ld_widget != 0);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);

    default_bg_img_tile = ld_gauge->ptBgImgTile;
    default_bg_mask_tile = ld_gauge->ptBgMaskTile;
    default_pointer_img_tile = ld_gauge->ptPointerImgTile;
    default_pointer_mask_tile = ld_gauge->ptPointerMaskTile;
    tracked_free_watch(default_bg_img_tile);
    tracked_free_watch(default_bg_mask_tile);
    tracked_free_watch(default_pointer_img_tile);
    tracked_free_watch(default_pointer_mask_tile);
    tracked_free_watch(&external_bg_img_tile);
    tracked_free_watch(&external_bg_mask_tile);
    tracked_free_watch(&external_pointer_img_tile);
    tracked_free_watch(&external_pointer_mask_tile);

    assert(tinyui_gauge_set_bg_source(gauge, &bg_source) == 0);
    assert(tinyui_gauge_set_pointer_source(gauge, &pointer_source) == 0);
    tinyui_app_destroy(app);

    assert(tracked_free_count_for(default_bg_img_tile) == 1);
    assert(tracked_free_count_for(default_bg_mask_tile) == 1);
    assert(tracked_free_count_for(default_pointer_img_tile) == 1);
    assert(tracked_free_count_for(default_pointer_mask_tile) == 1);
    assert(tracked_free_count_for(&external_bg_img_tile) == 0);
    assert(tracked_free_count_for(&external_bg_mask_tile) == 0);
    assert(tracked_free_count_for(&external_pointer_img_tile) == 0);
    assert(tracked_free_count_for(&external_pointer_mask_tile) == 0);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_gauge_create_and_backend_mapping(win);
    test_gauge_create_and_props(win);
    test_gauge_value_and_pointer_contract_match_backend_truth(win);
    test_gauge_rejects_invalid_inputs(win);
    test_gauge_internal_seam_names_are_gone();
    test_gauge_native_background_pointer_and_centre_offset_round_trip(win);
    test_gauge_native_trail_and_progress_bar_round_trip(win);
    test_gauge_init_and_shared_base_aliases_round_trip(win);
    test_gauge_create_with_props_failure_rolls_back_attached_child(win);
    test_gauge_rejects_null_args(win);

    tinyui_app_destroy(app);
    test_gauge_destroy_releases_owned_tiles_without_freeing_external_sources();
    return 0;
}
