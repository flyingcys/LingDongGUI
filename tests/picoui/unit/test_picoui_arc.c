#include "picoui/app.h"
#include "picoui/arc.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldArc.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>
#include <stdlib.h>

void *ldMalloc(uint32_t size)
{
    return malloc((size_t)size);
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    return calloc((size_t)num, (size_t)size);
}

struct tracked_free_entry {
    void *ptr;
    int count;
};

static struct tracked_free_entry g_tracked_frees[16];
static int g_tracked_free_count = 0;

static void tracked_free_reset(void)
{
    int i;

    for (i = 0; i < 16; ++i) {
        g_tracked_frees[i].ptr = NULL;
        g_tracked_frees[i].count = 0;
    }
    g_tracked_free_count = 0;
}

static void tracked_free_watch(void *ptr)
{
    assert(g_tracked_free_count < 16);
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

static void test_arc_create_and_props(struct picoui_window *win)
{
    struct picoui_arc_props props = {
        .id = "arc_props",
        .bg_start_angle = 30.0f,
        .bg_end_angle = 270.0f,
        .fg_end_angle = 60.0f,
        .rotation_angle = 15.0f,
        .bg_color = 0x112233,
        .fg_color = 0x445566,
    };
    struct picoui_arc *arc = picoui_arc_create((struct picoui_widget *)win, "arc");
    struct picoui_arc *with_props =
        picoui_arc_create_with_props((struct picoui_widget *)win, &props);

    assert(arc != 0);
    assert(with_props != 0);
    assert(picoui_arc_get_background_start_angle(arc) == 0.0f);
    assert(picoui_arc_get_background_angle(arc) == 360.0f);
    assert(picoui_arc_get_foreground_angle(arc) == 0.0f);
    assert(picoui_arc_get_rotation_angle(arc) == 0.0f);
    assert(picoui_arc_get_background_start_angle(with_props) == props.bg_start_angle);
    assert(picoui_arc_get_background_angle(with_props) == props.bg_end_angle - props.bg_start_angle);
    assert(picoui_arc_get_foreground_angle(with_props) == props.fg_end_angle);
    assert(picoui_arc_get_rotation_angle(with_props) == props.rotation_angle);
    assert(picoui_arc_get_background_color(with_props) != 0U);
    assert(picoui_arc_get_foreground_color(with_props) != 0U);
    assert(picoui_arc_get_background_color(with_props) != picoui_arc_get_foreground_color(with_props));
}

static void test_arc_value_and_angle_readback_match_backend_truth(struct picoui_window *win)
{
    struct picoui_arc *arc = picoui_arc_create((struct picoui_widget *)win, "arc_angles");

    assert(arc != 0);
    assert(picoui_arc_set_background_angle(arc, 45.0f, 315.0f) == 0);
    assert(picoui_arc_get_background_start_angle(arc) == 45.0f);
    assert(picoui_arc_get_background_angle(arc) == 270.0f);
    assert(picoui_arc_set_foreground_angle(arc, 135.0f) == 0);
    assert(picoui_arc_get_foreground_angle(arc) == 135.0f);
    assert(picoui_arc_set_rotation_angle(arc, 30.0f) == 0);
    assert(picoui_arc_get_rotation_angle(arc) == 30.0f);
    assert(picoui_arc_set_color(arc, 0xAABBCC, 0x223344) == 0);
    assert(picoui_arc_get_background_color(arc) == picoui_arc_get_background_color(arc));
    assert(picoui_arc_get_foreground_color(arc) == picoui_arc_get_foreground_color(arc));
    assert(picoui_arc_get_background_color(arc) != picoui_arc_get_foreground_color(arc));
}

static void test_arc_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_arc *arc = picoui_arc_create((struct picoui_widget *)win, "arc_invalid");

    assert(arc != 0);
    assert(picoui_arc_create(0, "arc") == 0);
    assert(picoui_arc_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_arc_create_with_props(0,
                                        &(struct picoui_arc_props){
                                            .id = "bad_parent",
                                        }) == 0);
    assert(picoui_arc_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_arc_create_with_props((struct picoui_widget *)win,
                                        &(struct picoui_arc_props){
                                            .bg_start_angle = -10.0f,
                                            .bg_end_angle = 180.0f,
                                        }) == 0);
    assert(picoui_arc_create_with_props((struct picoui_widget *)win,
                                        &(struct picoui_arc_props){
                                            .id = "bad_angle_order",
                                            .bg_start_angle = 40.0f,
                                            .bg_end_angle = 20.0f,
                                        }) == 0);
    assert(picoui_arc_set_background_angle(0, 0.0f, 0.0f) == -1);
    assert(picoui_arc_set_background_angle(arc, -1.0f, 180.0f) == -1);
    assert(picoui_arc_set_foreground_angle(0, 0.0f) == -1);
    assert(picoui_arc_set_foreground_angle(arc, -1.0f) == -1);
    assert(picoui_arc_set_rotation_angle(0, 0.0f) == -1);
    assert(picoui_arc_set_rotation_angle(arc, -1.0f) == -1);
    assert(picoui_arc_set_color(0, 0x0, 0x0) == -1);
    assert(picoui_arc_get_background_start_angle(0) == 0.0f);
    assert(picoui_arc_get_background_angle(0) == 0.0f);
    assert(picoui_arc_get_foreground_angle(0) == 0.0f);
    assert(picoui_arc_get_rotation_angle(0) == 0.0f);
}

static void test_arc_native_quarter_image_mask_and_parent_color_round_trip(struct picoui_window *win)
{
    struct picoui_arc *arc = picoui_arc_create((struct picoui_widget *)win, "arc_native_resources");
    struct picoui_backend_widget *backend;
    ldArc_t *ld_arc;
    arm_2d_tile_t quarter_img = {
        .tRegion = {
            .tSize = { .iWidth = 24, .iHeight = 24 },
        },
    };
    arm_2d_tile_t quarter_mask = {
        .tRegion = {
            .tSize = { .iWidth = 24, .iHeight = 24 },
        },
    };
    struct picoui_image_source quarter_source = {
        .img_tile = &quarter_img,
        .mask_tile = &quarter_mask,
    };

    assert(arc != 0);
    backend = (struct picoui_backend_widget *)arc->widget.backend_widget;
    assert(backend != 0);
    ld_arc = (ldArc_t *)backend->ld_widget;
    assert(ld_arc != 0);

    assert(picoui_arc_set_quarter_source(arc, &quarter_source) == 0);
    assert(picoui_arc_set_parent_color(arc, 0x223344U) == 0);

    assert(ld_arc->ptImgTile == &quarter_img);
    assert(ld_arc->ptMaskTile == &quarter_mask);
    assert(ld_arc->parentColor == (ldColor)0x223344U);

    assert(picoui_arc_set_quarter_source(0, &quarter_source) == -1);
    assert(picoui_arc_set_parent_color(0, 0x111111U) == -1);
}

static void test_arc_init_alias_matches_backend_truth(struct picoui_window *win)
{
    struct picoui_arc *arc = picoui_arc_init((struct picoui_widget *)win, "arc_alias");

    assert(arc != 0);
    assert(picoui_arc_get_background_start_angle(arc) == 0.0f);
    assert(picoui_arc_get_background_angle(arc) == 360.0f);
    assert(picoui_arc_get_foreground_angle(arc) == 0.0f);
    assert(picoui_arc_get_rotation_angle(arc) == 0.0f);
}

static void test_arc_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_arc_create(0, "id") == 0);
    assert(picoui_arc_create(win, 0) == 0);
    assert(picoui_arc_set_background_angle(0, 0, 0) == -1);
}

static void test_arc_destroy_releases_owned_tiles_without_freeing_external_sources(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_arc *arc;
    struct picoui_backend_widget *backend;
    ldArc_t *ld_arc;
    arm_2d_tile_t *default_img_tile;
    arm_2d_tile_t *default_mask_tile;
    arm_2d_tile_t external_img_tile = {
        .tRegion = {
            .tSize = { .iWidth = 24, .iHeight = 24 },
        },
    };
    arm_2d_tile_t external_mask_tile = {
        .tRegion = {
            .tSize = { .iWidth = 24, .iHeight = 24 },
        },
    };
    struct picoui_image_source external_source = {
        .img_tile = &external_img_tile,
        .mask_tile = &external_mask_tile,
    };

    tracked_free_reset();

    assert(app != 0);
    win = picoui_window_create(app, "arc_destroy_root");
    assert(win != 0);
    arc = picoui_arc_create((struct picoui_widget *)win, "arc_destroy");
    assert(arc != 0);
    backend = (struct picoui_backend_widget *)arc->widget.backend_widget;
    assert(backend != 0);
    ld_arc = (ldArc_t *)backend->ld_widget;
    assert(ld_arc != 0);

    default_img_tile = ld_arc->ptImgTile;
    default_mask_tile = ld_arc->ptMaskTile;
    tracked_free_watch(default_img_tile);
    tracked_free_watch(default_mask_tile);
    tracked_free_watch(&external_img_tile);
    tracked_free_watch(&external_mask_tile);

    assert(picoui_arc_set_quarter_source(arc, &external_source) == 0);
    picoui_app_destroy(app);

    assert(tracked_free_count_for(default_img_tile) == 1);
    assert(tracked_free_count_for(default_mask_tile) == 1);
    assert(tracked_free_count_for(&external_img_tile) == 0);
    assert(tracked_free_count_for(&external_mask_tile) == 0);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_arc_create_and_props(win);
    test_arc_value_and_angle_readback_match_backend_truth(win);
    test_arc_rejects_invalid_inputs(win);
    test_arc_native_quarter_image_mask_and_parent_color_round_trip(win);
    test_arc_init_alias_matches_backend_truth(win);
    test_arc_rejects_null_args(win);

    picoui_app_destroy(app);
    test_arc_destroy_releases_owned_tiles_without_freeing_external_sources();
    return 0;
}
