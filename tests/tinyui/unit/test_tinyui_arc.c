#include "app.h"
#include "picoui/arc.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldArc.h"
#include "backend.h"
#include "internal.h"
#include "picoui_test_support.h"

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

struct tinyui_arc_test_dispose_snapshot {
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

__attribute__((weak)) int tinyui_arc_test_take_last_dispose_snapshot(
    struct tinyui_arc_test_dispose_snapshot *snapshot)
{
    (void)snapshot;
    return -1;
}

__attribute__((weak)) struct picoui_arc *
tinyui_arc_test_create_with_props_fail_before_parent_color(
    struct picoui_widget *parent,
    const struct picoui_arc_props *props)
{
    (void)parent;
    (void)props;
    return 0;
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

static void assert_source_lacks_function_definition(const char *source_path,
                                                    const char *symbol)
{
    char needle[256];

    snprintf(needle, sizeof(needle), "static int %s(", symbol);
    if (strstr(symbol, "rgb_to_ld_color") != 0) {
        snprintf(needle, sizeof(needle), "static ldColor %s(", symbol);
    } else if (strstr(symbol, "ld_color_to_rgb") != 0) {
        snprintf(needle, sizeof(needle), "static unsigned int %s(", symbol);
    } else if (strstr(symbol, "backend") != 0) {
        snprintf(needle, sizeof(needle), "static struct picoui_backend_widget *%s(", symbol);
    } else if (strstr(symbol, "get_ld") != 0) {
        snprintf(needle, sizeof(needle), "static ldArc_t *%s(", symbol);
    }
    assert(picoui_test_source_contains(source_path, needle) == 0);
}

static void assert_source_has_function_definition(const char *source_path,
                                                  const char *prefix,
                                                  const char *symbol)
{
    char needle[256];

    snprintf(needle, sizeof(needle), "%s%s(", prefix, symbol);
    assert(picoui_test_source_contains(source_path, needle) == 1);
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
    assert(picoui_arc_get_background_color(with_props) == test_rgb_round_trip(props.bg_color));
    assert(picoui_arc_get_foreground_color(with_props) == test_rgb_round_trip(props.fg_color));
    assert(picoui_arc_get_background_color(with_props) != picoui_arc_get_foreground_color(with_props));
}

static void test_arc_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_arc *arc = picoui_arc_create((struct picoui_widget *)win, "arc_direct_mapping");
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    ldArc_t *ld_arc;

    assert(arc != 0);
    backend = (struct picoui_backend_widget *)arc->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_ARC);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &arc->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_arc = (ldArc_t *)backend->ld_widget;
    assert(ld_arc != 0);
    assert(((ldBase_t *)ld_arc)->pInfo == backend);
    assert(tinyui_widget_has_ld_binding(&arc->widget) == 1);
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
    assert(picoui_arc_get_background_color(arc) == test_rgb_round_trip(0xAABBCCU));
    assert(picoui_arc_get_foreground_color(arc) == test_rgb_round_trip(0x223344U));
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

static void test_arc_create_with_props_failure_rolls_back_attached_child(struct picoui_window *win)
{
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct tinyui_arc_test_dispose_snapshot snapshot = {0};
    struct picoui_arc *arc;

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    arc = tinyui_arc_test_create_with_props_fail_before_parent_color(
        (struct picoui_widget *)win,
        &(struct picoui_arc_props){
            .id = "arc_fail_parent_color",
            .bg_start_angle = 15.0f,
            .bg_end_angle = 220.0f,
            .fg_end_angle = 80.0f,
            .rotation_angle = 10.0f,
            .parent_color = 0x123456U,
            .bg_color = 0x654321U,
            .fg_color = 0xabcdefU,
        });

    assert(arc == 0);
    assert(tinyui_arc_test_take_last_dispose_snapshot(&snapshot) == 0);
    assert(snapshot.kind == PICOUI_BACKEND_WIDGET_ARC);
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
    assert(tinyui_arc_test_take_last_dispose_snapshot(&snapshot) == -1);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }
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
    const char *widget_source =
        picoui_test_repo_path_from_file(__FILE__, "tinyui/src/widgets/arc.c");
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(widget_source != 0);
    assert_source_lacks_function_definition(widget_source, "picoui_arc_props_are_valid");
    assert_source_lacks_function_definition(widget_source, "picoui_arc_rgb_to_ld_color");
    assert_source_lacks_function_definition(widget_source, "picoui_arc_ld_color_to_rgb");
    assert_source_lacks_function_definition(widget_source, "picoui_arc_backend");
    assert_source_lacks_function_definition(widget_source, "picoui_arc_get_ld");
    assert_source_lacks_function_definition(widget_source, "picoui_arc_finish_detach_after_backend_failure");
    assert_source_lacks_function_definition(widget_source, "picoui_arc_dispose_partial_impl");
    assert_source_lacks_function_definition(widget_source, "picoui_backend_arc_test_take_last_dispose_snapshot");
    assert_source_lacks_function_definition(widget_source, "picoui_backend_arc_test_create_with_props_fail_before_parent_color");
    assert_source_has_function_definition(widget_source, "static int ", "tinyui_arc_props_are_valid");
    assert_source_has_function_definition(widget_source, "static ldColor ", "tinyui_arc_rgb_to_ld_color");
    assert_source_has_function_definition(widget_source, "static unsigned int ", "tinyui_arc_ld_color_to_rgb");
    assert_source_has_function_definition(widget_source, "static struct picoui_backend_widget *", "tinyui_arc_backend");
    assert_source_has_function_definition(widget_source, "static ldArc_t *", "tinyui_arc_get_ld");
    assert_source_has_function_definition(widget_source, "static int ", "tinyui_arc_finish_detach_after_backend_failure");
    assert_source_has_function_definition(widget_source, "static void ", "tinyui_arc_dispose_partial_impl");
    assert_source_has_function_definition(widget_source, "int ", "tinyui_arc_test_take_last_dispose_snapshot");
    assert_source_has_function_definition(widget_source, "struct picoui_arc *", "tinyui_arc_test_create_with_props_fail_before_parent_color");

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_arc_create_and_backend_mapping(win);
    test_arc_create_and_props(win);
    test_arc_value_and_angle_readback_match_backend_truth(win);
    test_arc_rejects_invalid_inputs(win);
    test_arc_native_quarter_image_mask_and_parent_color_round_trip(win);
    test_arc_init_alias_matches_backend_truth(win);
    test_arc_create_with_props_failure_rolls_back_attached_child(win);
    test_arc_rejects_null_args(win);

    picoui_app_destroy(app);
    test_arc_destroy_releases_owned_tiles_without_freeing_external_sources();
    return 0;
}
