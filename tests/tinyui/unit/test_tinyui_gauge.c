#include "app.h"
#include "gauge.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldGauge.h"
#include "backend.h"
#include "internal.h"
#include "picoui_test_support.h"

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

extern int tinyui_widget_has_ld_binding(const struct picoui_widget *widget);

struct picoui_gauge_test_dispose_snapshot {
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

__attribute__((weak)) int tinyui_gauge_test_take_last_dispose_snapshot(
    struct picoui_gauge_test_dispose_snapshot *snapshot)
{
    (void)snapshot;
    return -1;
}

__attribute__((weak)) struct picoui_gauge *
tinyui_gauge_test_create_with_props_fail_before_centre_offset(
    struct picoui_widget *parent,
    const struct picoui_gauge_props *props)
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

static void test_gauge_create_and_props(struct picoui_window *win)
{
    struct picoui_gauge_props props = {
        .id = "gauge_props",
        .angle = 45.0f,
        .pointer_color = 0x334455,
        .auto_move = 1,
    };
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge");
    struct picoui_gauge *with_props =
        picoui_gauge_create_with_props((struct picoui_widget *)win, &props);

    assert(gauge != 0);
    assert(with_props != 0);
    assert(picoui_gauge_get_angle(gauge) == 0.0f);
    assert(picoui_gauge_get_pointer_color(gauge) == 0x000000);
    assert(picoui_gauge_get_auto_move(gauge) == 0);
    assert(picoui_gauge_get_angle(with_props) == props.angle);
    assert(picoui_gauge_get_pointer_color(with_props) == test_rgb_round_trip(props.pointer_color));
    assert(picoui_gauge_get_auto_move(with_props) == props.auto_move);
}

static void test_gauge_internal_seam_names_are_gone(void)
{
    const char *source_path = "tinyui/src/widgets/gauge.c";
    const char *test_source_path = "tests/tinyui/unit/test_tinyui_gauge.c";

    assert(picoui_test_source_lacks_function_definition(source_path,
                                                        "picoui_gauge_props_are_valid") == 1);
    assert(picoui_test_source_lacks_function_definition(source_path,
                                                        "picoui_gauge_rgb_to_ld_color") == 1);
    assert(picoui_test_source_lacks_function_definition(source_path,
                                                        "picoui_gauge_ld_color_to_rgb") == 1);
    assert(picoui_test_source_lacks_function_definition(source_path,
                                                        "picoui_gauge_backend") == 1);
    assert(picoui_test_source_lacks_function_definition(source_path,
                                                        "picoui_gauge_get_ld") == 1);
    assert(picoui_test_source_lacks_function_definition(source_path,
                                                        "picoui_gauge_finish_detach_after_backend_failure") == 1);
    assert(picoui_test_source_lacks_function_definition(source_path,
                                                        "picoui_gauge_dispose_partial_impl") == 1);
    assert(picoui_test_source_lacks_function_definition(source_path,
                                                        "picoui_backend_gauge_test_take_last_dispose_snapshot") == 1);
    assert(picoui_test_source_lacks_function_definition(
               source_path,
               "picoui_backend_gauge_test_create_with_props_fail_before_centre_offset") == 1);
    assert(picoui_test_source_lacks_function_definition(
               test_source_path,
               "picoui_backend_gauge_test_take_last_dispose_snapshot") == 1);
    assert(picoui_test_source_lacks_function_definition(
               test_source_path,
               "picoui_backend_gauge_test_create_with_props_fail_before_centre_offset") == 1);
}

static void test_gauge_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_direct_mapping");
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    ldGauge_t *ld_gauge;

    assert(gauge != 0);
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_GAUGE);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &gauge->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);
    assert(((ldBase_t *)ld_gauge)->pInfo == backend);
    assert(tinyui_widget_has_ld_binding(&gauge->widget) == 1);
}

static void test_gauge_value_and_pointer_contract_match_backend_truth(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_angle");

    assert(gauge != 0);
    assert(picoui_gauge_set_angle(gauge, 90.0f) == 0);
    assert(picoui_gauge_get_angle(gauge) == 90.0f);
    assert(picoui_gauge_set_pointer_color(gauge, 0xAABBCC) == 0);
    assert(picoui_gauge_get_pointer_color(gauge) == test_rgb_round_trip(0xAABBCCU));
    assert(picoui_gauge_set_auto_move(gauge, 1) == 0);
    assert(picoui_gauge_get_auto_move(gauge) == 1);
    assert(picoui_gauge_set_auto_move(gauge, 0) == 0);
    assert(picoui_gauge_get_auto_move(gauge) == 0);
}

static void test_gauge_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_invalid");

    assert(gauge != 0);
    assert(picoui_gauge_create(0, "gauge") == 0);
    assert(picoui_gauge_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_gauge_create_with_props(0,
                                          &(struct picoui_gauge_props){
                                              .id = "bad_parent",
                                          }) == 0);
    assert(picoui_gauge_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_gauge_create_with_props((struct picoui_widget *)win,
                                          &(struct picoui_gauge_props){
                                              .angle = 30.0f,
                                          }) == 0);
    assert(picoui_gauge_set_angle(0, 0.0f) == -1);
    assert(picoui_gauge_set_pointer_color(0, 0x0) == -1);
    assert(picoui_gauge_set_auto_move(0, 0) == -1);
    assert(picoui_gauge_get_angle(0) == 0.0f);
    assert(picoui_gauge_get_pointer_color(0) == 0x000000);
    assert(picoui_gauge_get_auto_move(0) == -1);
    assert(picoui_gauge_set_auto_move(gauge, 7) == 0);
    assert(picoui_gauge_get_auto_move(gauge) == 1);
}

static void test_gauge_native_background_pointer_and_centre_offset_round_trip(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_native_resources");
    struct picoui_backend_widget *backend;
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
    struct picoui_image_source bg_source = {
        .img_tile = &bg_img,
        .mask_tile = &bg_mask,
    };
    struct picoui_image_source pointer_source = {
        .img_tile = &pointer_img,
        .mask_tile = &pointer_mask,
    };

    assert(gauge != 0);
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    assert(backend != 0);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);

    assert(picoui_gauge_set_bg_source(gauge, &bg_source) == 0);
    assert(picoui_gauge_set_pointer_source(gauge, &pointer_source) == 0);
    assert(picoui_gauge_set_centre_offset(gauge, -7, 9) == 0);

    assert(ld_gauge->ptBgImgTile == &bg_img);
    assert(ld_gauge->ptBgMaskTile == &bg_mask);
    assert(ld_gauge->ptPointerImgTile == &pointer_img);
    assert(ld_gauge->ptPointerMaskTile == &pointer_mask);
    assert(ld_gauge->centreOffsetX == -7);
    assert(ld_gauge->centreOffsetY == 9);

    assert(picoui_gauge_set_bg_source(0, &bg_source) == -1);
    assert(picoui_gauge_set_pointer_source(0, &pointer_source) == -1);
    assert(picoui_gauge_set_centre_offset(0, 1, 2) == -1);
}

static void test_gauge_native_trail_and_progress_bar_round_trip(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_native_trail");
    struct picoui_backend_widget *backend;
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
    struct picoui_image_source bg_trail_source = {
        .img_tile = &bg_trail_mask,
        .mask_tile = &bg_trail_mask,
    };
    struct picoui_image_source pointer_trail_source = {
        .img_tile = &pointer_trail_mask,
        .mask_tile = &pointer_trail_mask,
    };

    assert(gauge != 0);
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    assert(backend != 0);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);

    assert(picoui_gauge_set_trail(gauge, &bg_trail_source, &pointer_trail_source) == 0);
    assert(ld_gauge->ptBgTrailMaskTile == &bg_trail_mask);
    assert(ld_gauge->ptPointerTrailMaskTile == &pointer_trail_mask);
    assert(ld_gauge->isProgressBar == false);

    assert(picoui_gauge_set_progress_bar(gauge, &bg_trail_source, &pointer_trail_source) == 0);
    assert(ld_gauge->ptBgTrailMaskTile == &bg_trail_mask);
    assert(ld_gauge->ptPointerTrailMaskTile == &pointer_trail_mask);
    assert(ld_gauge->isProgressBar == true);

    assert(picoui_gauge_set_trail(0, &bg_trail_source, &pointer_trail_source) == -1);
    assert(picoui_gauge_set_progress_bar(0, &bg_trail_source, &pointer_trail_source) == -1);
}

static void test_gauge_init_and_shared_base_aliases_round_trip(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_init((struct picoui_widget *)win, "gauge_alias");
    struct picoui_backend_widget *backend;
    ldGauge_t *ld_gauge;

    assert(gauge != 0);
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    assert(backend != 0);
    ld_gauge = (ldGauge_t *)backend->ld_widget;
    assert(ld_gauge != 0);

    assert(picoui_gauge_get_angle(gauge) == 0.0f);

    assert(picoui_widget_set_pos(&gauge->widget, 14, 18) == 0);
    assert(((ldBase_t *)ld_gauge)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 14);
    assert(((ldBase_t *)ld_gauge)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 18);

    assert(picoui_widget_set_visible(&gauge->widget, 0) == 0);
    assert(((ldBase_t *)ld_gauge)->isHidden == true);
    assert(picoui_widget_set_opacity(&gauge->widget, 71) == 0);
    assert(((ldBase_t *)ld_gauge)->opacity == 71);
    assert(picoui_widget_set_selectable(&gauge->widget, 1) == 0);
    assert(((ldBase_t *)ld_gauge)->isSelectable == true);
    assert(picoui_widget_set_selected(&gauge->widget, 1) == 0);
    assert(((ldBase_t *)ld_gauge)->isSelected == true);
    assert(picoui_widget_set_selectable(&gauge->widget, 0) == 0);
    assert(((ldBase_t *)ld_gauge)->isSelectable == false);
    assert(picoui_widget_set_corner(&gauge->widget, 6) == 0);
    assert(((ldBase_t *)ld_gauge)->isCorner == true);
}

static void test_gauge_create_with_props_failure_rolls_back_attached_child(struct picoui_window *win)
{
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct picoui_gauge_test_dispose_snapshot snapshot = {0};
    struct picoui_gauge *gauge;

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    gauge = tinyui_gauge_test_create_with_props_fail_before_centre_offset(
        (struct picoui_widget *)win,
        &(struct picoui_gauge_props){
            .id = "gauge_fail_centre_offset",
            .angle = 70.0f,
            .centre_offset_x = 4,
            .centre_offset_y = -6,
            .pointer_color = 0x334455U,
            .auto_move = 1,
        });

    assert(gauge == 0);
    assert(tinyui_gauge_test_take_last_dispose_snapshot(&snapshot) == 0);
    assert(snapshot.kind == PICOUI_BACKEND_WIDGET_GAUGE);
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
    assert(tinyui_gauge_test_take_last_dispose_snapshot(&snapshot) == -1);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }
}

static void test_gauge_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_gauge_create(0, "id") == 0);
    assert(picoui_gauge_create(win, 0) == 0);
    assert(picoui_gauge_set_angle(0, 90.0f) == -1);
    assert(picoui_gauge_get_angle(0) == 0.0f);
    assert(picoui_gauge_set_pointer_color(0, 0xFFFFFFU) == -1);
    assert(picoui_gauge_get_pointer_color(0) == 0x000000);
    assert(picoui_gauge_set_auto_move(0, 1) == -1);
    assert(picoui_gauge_get_auto_move(0) == -1);
}

static void test_gauge_destroy_releases_owned_tiles_without_freeing_external_sources(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_gauge *gauge;
    struct picoui_backend_widget *backend;
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
    struct picoui_image_source bg_source = {
        .img_tile = &external_bg_img_tile,
        .mask_tile = &external_bg_mask_tile,
    };
    struct picoui_image_source pointer_source = {
        .img_tile = &external_pointer_img_tile,
        .mask_tile = &external_pointer_mask_tile,
    };

    tracked_free_reset();

    assert(app != 0);
    win = picoui_window_create(app, "gauge_destroy_root");
    assert(win != 0);
    gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_destroy");
    assert(gauge != 0);
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    assert(backend != 0);
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

    assert(picoui_gauge_set_bg_source(gauge, &bg_source) == 0);
    assert(picoui_gauge_set_pointer_source(gauge, &pointer_source) == 0);
    picoui_app_destroy(app);

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
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
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

    picoui_app_destroy(app);
    test_gauge_destroy_releases_owned_tiles_without_freeing_external_sources();
    return 0;
}
