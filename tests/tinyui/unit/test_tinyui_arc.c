#include "core/app.h"
#include "widgets/arc.h"
#include "core/widget.h"
#include "widgets/window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldArc.h"
#include "../../../examples/common/demo/widget/images/uiImages.h"
#include "internal.h"
#include "tinyui_test_support.h"

#include <assert.h>
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

static const struct tinyui_widget *tinyui_arc_test_last_disposed_backend(void)
{
    if (g_disposed_backend_valid == 0) {
        return 0;
    }
    return &g_disposed_backend_snapshot;
}

__attribute__((weak)) struct tinyui_arc *
tinyui_arc_test_create_with_props_fail_before_parent_color(
    struct tinyui_widget *parent,
    const struct tinyui_arc_props *props)
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
    assert(tinyui_test_source_lacks_function_definition(source_path, symbol) == 1);
}

static void assert_source_has_function_definition(const char *source_path,
                                                  const char *prefix,
                                                  const char *symbol)
{
    (void)prefix;
    assert(tinyui_test_source_has_function_definition(source_path, symbol) == 1);
}

static arm_2d_tile_t make_rgb565_tile(uint16_t *buffer, int16_t width, int16_t height)
{
    arm_2d_tile_t tile = {0};

    tile.bIsRoot = true;
    tile.tInfo.tColourInfo.chScheme = ARM_2D_COLOUR_RGB565;
    tile.tRegion.tSize.iWidth = width;
    tile.tRegion.tSize.iHeight = height;
    tile.phwBuffer = buffer;
    return tile;
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

static void test_arc_create_and_props(struct tinyui_window *win)
{
    struct tinyui_arc_props props = {
        .id = "arc_props",
        .bg_start_angle = 30.0f,
        .bg_end_angle = 270.0f,
        .fg_end_angle = 60.0f,
        .rotation_angle = 15.0f,
        .bg_color = 0x112233,
        .fg_color = 0x445566,
    };
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win, "arc");
    struct tinyui_arc *with_props =
        tinyui_arc_create_with_props((struct tinyui_widget *)win, &props);

    assert(arc != 0);
    assert(with_props != 0);
    assert(tinyui_arc_get_background_start_angle(arc) == 0.0f);
    assert(tinyui_arc_get_background_angle(arc) == 360.0f);
    assert(tinyui_arc_get_foreground_angle(arc) == 0.0f);
    assert(tinyui_arc_get_rotation_angle(arc) == 0.0f);
    assert(tinyui_arc_get_background_start_angle(with_props) == props.bg_start_angle);
    assert(tinyui_arc_get_background_angle(with_props) == props.bg_end_angle - props.bg_start_angle);
    assert(tinyui_arc_get_foreground_angle(with_props) == props.fg_end_angle);
    assert(tinyui_arc_get_rotation_angle(with_props) == props.rotation_angle);
    assert(tinyui_arc_get_background_color(with_props) == test_rgb_round_trip(props.bg_color));
    assert(tinyui_arc_get_foreground_color(with_props) == test_rgb_round_trip(props.fg_color));
    assert(tinyui_arc_get_background_color(with_props) != tinyui_arc_get_foreground_color(with_props));
}

static void test_arc_create_and_backend_mapping(struct tinyui_window *win)
{
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_direct_mapping");
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldArc_t *ld_arc;
    const arm_2d_tile_t *legacy_img_tile = IMAGE_ARC_QUARTER_PNG_Mask;
    const arm_2d_tile_t *legacy_mask_tile = IMAGE_ARC_QUARTER_MASK_PNG_Mask;

    assert(arc != 0);
    backend = &arc->widget;
    assert(backend->ld_widget != 0);
    parent_backend = &win->widget;
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_ARC);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_arc = (ldArc_t *)backend->ld_widget;
    assert(ld_arc != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
    assert(tinyui_widget_has_ld_binding(&arc->widget) == 1);
    assert(ld_arc->ptImgTile != 0);
    assert(ld_arc->ptMaskTile != 0);
    assert(ld_arc->ptImgTile->tRegion.tSize.iWidth == 53);
    assert(ld_arc->ptImgTile->tRegion.tSize.iHeight == 53);
    assert(ld_arc->ptMaskTile->tRegion.tSize.iWidth == 53);
    assert(ld_arc->ptMaskTile->tRegion.tSize.iHeight == 53);
    assert(ld_arc->ptImgTile->pchBuffer == legacy_img_tile->pchBuffer);
    assert(ld_arc->ptMaskTile->pchBuffer == legacy_mask_tile->pchBuffer);
}

static void test_arc_value_and_angle_readback_match_backend_truth(struct tinyui_window *win)
{
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_angles");

    assert(arc != 0);
    assert(tinyui_arc_set_background_angle(arc, 45.0f, 315.0f) == 0);
    assert(tinyui_arc_get_background_start_angle(arc) == 45.0f);
    assert(tinyui_arc_get_background_angle(arc) == 270.0f);
    assert(tinyui_arc_set_foreground_angle(arc, 135.0f) == 0);
    assert(tinyui_arc_get_foreground_angle(arc) == 135.0f);
    assert(tinyui_arc_set_rotation_angle(arc, 30.0f) == 0);
    assert(tinyui_arc_get_rotation_angle(arc) == 30.0f);
    assert(tinyui_arc_set_color(arc, 0xAABBCC, 0x223344) == 0);
    assert(tinyui_arc_get_background_color(arc) == test_rgb_round_trip(0xAABBCCU));
    assert(tinyui_arc_get_foreground_color(arc) == test_rgb_round_trip(0x223344U));
    assert(tinyui_arc_get_background_color(arc) != tinyui_arc_get_foreground_color(arc));
}

static void test_arc_rejects_invalid_inputs(struct tinyui_window *win)
{
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_invalid");

    assert(arc != 0);
    assert(tinyui_arc_create(0, "arc") == 0);
    assert(tinyui_arc_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_arc_create_with_props(0,
                                        &(struct tinyui_arc_props){
                                            .id = "bad_parent",
                                        }) == 0);
    assert(tinyui_arc_create_with_props((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_arc_create_with_props((struct tinyui_widget *)win,
                                        &(struct tinyui_arc_props){
                                            .bg_start_angle = -10.0f,
                                            .bg_end_angle = 180.0f,
                                        }) == 0);
    assert(tinyui_arc_create_with_props((struct tinyui_widget *)win,
                                        &(struct tinyui_arc_props){
                                            .id = "bad_angle_order",
                                            .bg_start_angle = 40.0f,
                                            .bg_end_angle = 20.0f,
                                        }) == 0);
    assert(tinyui_arc_set_background_angle(0, 0.0f, 0.0f) == -1);
    assert(tinyui_arc_set_background_angle(arc, -1.0f, 180.0f) == -1);
    assert(tinyui_arc_set_foreground_angle(0, 0.0f) == -1);
    assert(tinyui_arc_set_foreground_angle(arc, -1.0f) == -1);
    assert(tinyui_arc_set_rotation_angle(0, 0.0f) == -1);
    assert(tinyui_arc_set_rotation_angle(arc, -1.0f) == -1);
    assert(tinyui_arc_set_color(0, 0x0, 0x0) == -1);
    assert(tinyui_arc_get_background_start_angle(0) == 0.0f);
    assert(tinyui_arc_get_background_angle(0) == 0.0f);
    assert(tinyui_arc_get_foreground_angle(0) == 0.0f);
    assert(tinyui_arc_get_rotation_angle(0) == 0.0f);
}

static void test_arc_native_quarter_image_mask_and_parent_color_round_trip(struct tinyui_window *win)
{
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_native_resources");
    struct tinyui_widget *backend;
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
    struct tinyui_image_source quarter_source = {
        .img_tile = &quarter_img,
        .mask_tile = &quarter_mask,
    };

    assert(arc != 0);
    backend = &arc->widget;
    assert(backend->ld_widget != 0);
    ld_arc = (ldArc_t *)backend->ld_widget;
    assert(ld_arc != 0);

    assert(tinyui_arc_set_quarter_source(arc, &quarter_source) == 0);
    assert(tinyui_arc_set_parent_color(arc, 0x223344U) == 0);

    assert(ld_arc->ptImgTile == &quarter_img);
    assert(ld_arc->ptMaskTile == &quarter_mask);
    assert(ld_arc->parentColor == (ldColor)tinyui_rgb_to_ld_color(0x223344U));

    assert(tinyui_arc_set_quarter_source(0, &quarter_source) == -1);
    assert(tinyui_arc_set_parent_color(0, 0x111111U) == -1);
}

static void test_arc_parent_color_uses_ld_color_encoding(struct tinyui_window *win)
{
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_parent_color_encoding");
    ldArc_t *ld_arc;

    assert(arc != 0);
    ld_arc = (ldArc_t *)arc->widget.ld_widget;
    assert(ld_arc != 0);

    assert(tinyui_arc_set_parent_color(arc, 0xF0F0F0U) == 0);
    assert(ld_arc->parentColor == (ldColor)tinyui_rgb_to_ld_color(0xF0F0F0U));
    assert(ld_arc->parentColor != (ldColor)0xF0F0F0U);
}

static void test_arc_create_with_props_preserves_default_parent_color_when_unspecified(struct tinyui_window *win)
{
    struct tinyui_arc *arc =
        tinyui_arc_create_with_props((struct tinyui_widget *)win,
                                     &(struct tinyui_arc_props){
                                         .id = "arc_props_default_parent_color",
                                         .bg_start_angle = 30.0f,
                                         .bg_end_angle = 300.0f,
                                         .fg_end_angle = 45.0f,
                                         .rotation_angle = 12.0f,
                                         .bg_color = 0xCCD5E3U,
                                         .fg_color = 0x2B6CB0U,
                                     });
    ldArc_t *ld_arc;

    assert(arc != 0);
    ld_arc = (ldArc_t *)arc->widget.ld_widget;
    assert(ld_arc != 0);
    assert(arc->parent_color == 0xF0F0F0U);
    assert(ld_arc->parentColor == (ldColor)tinyui_rgb_to_ld_color(0xF0F0F0U));
}

static void test_arc_create_with_props_accepts_black_parent_color(struct tinyui_window *win)
{
    struct tinyui_arc *arc =
        tinyui_arc_create_with_props((struct tinyui_widget *)win,
                                     &(struct tinyui_arc_props){
                                         .id = "arc_props_black_parent_color",
                                         .bg_start_angle = 30.0f,
                                         .bg_end_angle = 300.0f,
                                         .fg_end_angle = 45.0f,
                                         .rotation_angle = 12.0f,
                                         .has_parent_color = 1,
                                         .parent_color = 0x000000U,
                                         .bg_color = 0xCCD5E3U,
                                         .fg_color = 0x2B6CB0U,
                                     });
    ldArc_t *ld_arc;

    assert(arc != 0);
    ld_arc = (ldArc_t *)arc->widget.ld_widget;
    assert(ld_arc != 0);
    assert(arc->parent_color == 0x000000U);
    assert(ld_arc->parentColor == (ldColor)tinyui_rgb_to_ld_color(0x000000U));
}

static void test_arc_legacy_demo0_parity_parameters_map_to_backend(struct tinyui_window *win)
{
    struct tinyui_image_source quarter_source = {
        .img_tile = IMAGE_ARC_QUARTER_PNG_Mask,
        .mask_tile = IMAGE_ARC_QUARTER_MASK_PNG_Mask,
    };
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_legacy_demo0_parity");
    ldArc_t *ld_arc;

    assert(arc != 0);
    ld_arc = (ldArc_t *)arc->widget.ld_widget;
    assert(ld_arc != 0);

    assert(tinyui_arc_set_quarter_source(arc, &quarter_source) == 0);
    assert(tinyui_arc_set_background_angle(arc, 0.0f, 350.0f) == 0);
    assert(tinyui_arc_set_foreground_angle(arc, 30.0f) == 0);
    assert(tinyui_arc_set_parent_color(arc, 0xF0F0F0U) == 0);
    assert(tinyui_arc_set_color(arc, 0xADD8E6U, 0x90EE90U) == 0);

    assert(ld_arc->ptImgTile == IMAGE_ARC_QUARTER_PNG_Mask);
    assert(ld_arc->ptMaskTile == IMAGE_ARC_QUARTER_MASK_PNG_Mask);
    assert(ld_arc->ptImgTile->tRegion.tSize.iWidth == 53);
    assert(ld_arc->ptImgTile->tRegion.tSize.iHeight == 53);
    assert(ld_arc->ptMaskTile->tRegion.tSize.iWidth == 53);
    assert(ld_arc->ptMaskTile->tRegion.tSize.iHeight == 53);
    assert(ldArcGetBackgroundStartAngle(ld_arc) == 0.0f);
    assert(ldArcGetBackgroundAngle(ld_arc) == 350.0f);
    assert(ldArcGetForegroundAngle(ld_arc) == 30.0f);
    assert(ldArcGetRotationAngle(ld_arc) == 0.0f);
    assert(ldArcGetBackgroundColor(ld_arc) == (ldColor)tinyui_rgb_to_ld_color(0xADD8E6U));
    assert(ldArcGetForegroundColor(ld_arc) == (ldColor)tinyui_rgb_to_ld_color(0x90EE90U));
    assert(ld_arc->parentColor == (ldColor)tinyui_rgb_to_ld_color(0xF0F0F0U));
}

static void test_arc_builtin_quarter_source_matches_legacy_backend_tiles(struct tinyui_window *win)
{
    struct tinyui_image_source source = {0};
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_builtin_quarter_legacy");
    ldArc_t *ld_arc;

    assert(arc != 0);
    ld_arc = (ldArc_t *)arc->widget.ld_widget;
    assert(ld_arc != 0);

    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &source) == 0);
    assert(tinyui_arc_set_quarter_source(arc, &source) == 0);
    assert(ld_arc->ptImgTile == IMAGE_ARC_QUARTER_PNG_Mask);
    assert(ld_arc->ptMaskTile == IMAGE_ARC_QUARTER_MASK_PNG_Mask);
    assert(ld_arc->ptImgTile->tRegion.tSize.iWidth == 53);
    assert(ld_arc->ptImgTile->tRegion.tSize.iHeight == 53);
    assert(ld_arc->ptMaskTile->tRegion.tSize.iWidth == 53);
    assert(ld_arc->ptMaskTile->tRegion.tSize.iHeight == 53);
}

static void test_arc_legacy_demo0_source_preserves_initial_rotation_phase(void)
{
    assert(tinyui_test_source_contains("tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c",
                                       "runtime->angle = 120.0f;") == 1);
    assert(tinyui_test_source_contains("tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c",
                                       "tinyui_arc_set_rotation_angle(arc, 120.0f)") == 0);
}

static void test_arc_show_prepares_when_first_active_pfb_is_not_new_frame(struct tinyui_window *win)
{
    static uint16_t target_buffer[128 * 128];
    arm_2d_tile_t target = make_rgb565_tile(target_buffer, 128, 128);
    struct tinyui_image_source quarter_source = {
        .img_tile = IMAGE_ARC_QUARTER_PNG_Mask,
        .mask_tile = IMAGE_ARC_QUARTER_MASK_PNG_Mask,
    };
    struct tinyui_arc *arc = tinyui_arc_create((struct tinyui_widget *)win,
                                               "arc_late_active_pfb");
    ldArc_t *ld_arc;

    memset(target_buffer, 0, sizeof(target_buffer));
    assert(arc != 0);
    ld_arc = (ldArc_t *)arc->widget.ld_widget;
    assert(ld_arc != 0);

    assert(tinyui_arc_set_quarter_source(arc, &quarter_source) == 0);
    assert(tinyui_arc_set_background_angle(arc, 0.0f, 350.0f) == 0);
    assert(tinyui_arc_set_foreground_angle(arc, 30.0f) == 0);
    assert(tinyui_arc_set_parent_color(arc, 0xF0F0F0U) == 0);
    assert(tinyui_arc_set_color(arc, 0xADD8E6U, 0x90EE90U) == 0);
    assert(tinyui_widget_set_pos((struct tinyui_widget *)arc, 0, 0) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)arc, 103, 103) == 0);

    ldArc_show(arc->widget.ld_event_bridge_scene, ld_arc, &target, false);
    arm_2d_op_wait_async(NULL);
}

static void test_arc_init_alias_matches_backend_truth(struct tinyui_window *win)
{
    struct tinyui_arc *arc = tinyui_arc_init((struct tinyui_widget *)win, "arc_alias");

    assert(arc != 0);
    assert(tinyui_arc_get_background_start_angle(arc) == 0.0f);
    assert(tinyui_arc_get_background_angle(arc) == 360.0f);
    assert(tinyui_arc_get_foreground_angle(arc) == 0.0f);
    assert(tinyui_arc_get_rotation_angle(arc) == 0.0f);
}

static void test_arc_create_with_props_failure_rolls_back_attached_child(struct tinyui_window *win)
{
    ldBase_t *win_ld = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    struct tinyui_arc *arc;
    const struct tinyui_widget *disposed_backend;

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_test_capture_destroyed_widget_snapshot(0);
    arc = tinyui_arc_test_create_with_props_fail_before_parent_color(
        (struct tinyui_widget *)win,
        &(struct tinyui_arc_props){
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
    disposed_backend = tinyui_arc_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == TINYUI_BACKEND_WIDGET_ARC);
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

static void test_arc_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_arc_create(0, "id") == 0);
    assert(tinyui_arc_create(win, 0) == 0);
    assert(tinyui_arc_set_background_angle(0, 0, 0) == -1);
}

static void test_arc_destroy_releases_owned_tiles_without_freeing_external_sources(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_arc *arc;
    struct tinyui_widget *backend;
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
    struct tinyui_image_source external_source = {
        .img_tile = &external_img_tile,
        .mask_tile = &external_mask_tile,
    };

    tracked_free_reset();

    assert(app != 0);
    win = tinyui_window_create(app, "arc_destroy_root");
    assert(win != 0);
    arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_destroy");
    assert(arc != 0);
    backend = &arc->widget;
    assert(backend->ld_widget != 0);
    ld_arc = (ldArc_t *)backend->ld_widget;
    assert(ld_arc != 0);

    default_img_tile = ld_arc->ptImgTile;
    default_mask_tile = ld_arc->ptMaskTile;
    tracked_free_watch(default_img_tile);
    tracked_free_watch(default_mask_tile);
    tracked_free_watch(&external_img_tile);
    tracked_free_watch(&external_mask_tile);

    assert(tinyui_arc_set_quarter_source(arc, &external_source) == 0);
    tinyui_app_destroy(app);

    assert(tracked_free_count_for(default_img_tile) == 1);
    assert(tracked_free_count_for(default_mask_tile) == 1);
    assert(tracked_free_count_for(&external_img_tile) == 0);
    assert(tracked_free_count_for(&external_mask_tile) == 0);
}

int main(void)
{
    const char *widget_source =
        tinyui_test_repo_path_from_file(__FILE__, "tinyui/src/widgets/arc.c");
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(widget_source != 0);
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_arc_test_take_last_dispose_snapshot");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_arc_test_create_with_props_fail_before_parent_color");
    assert_source_lacks_function_definition(widget_source, "tinyui_arc_rgb_to_ld_color");
    assert_source_lacks_function_definition(widget_source, "tinyui_arc_ld_color_to_rgb");
    assert_source_lacks_function_definition(widget_source, "tinyui_arc_get_ld");
    assert_source_lacks_function_definition(widget_source, "tinyui_arc_finish_detach_after_backend_failure");
    assert_source_lacks_function_definition(widget_source, "tinyui_arc_dispose_partial_impl");
    assert_source_lacks_function_definition(widget_source, "tinyui_arc_test_take_last_dispose_snapshot");
    assert_source_has_function_definition(widget_source, "static int ", "tinyui_arc_props_are_valid");
    assert_source_has_function_definition(widget_source, "", "tinyui_arc_test_create_with_props_fail_before_parent_color");

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_arc_create_and_backend_mapping(win);
    test_arc_create_and_props(win);
    test_arc_value_and_angle_readback_match_backend_truth(win);
    test_arc_rejects_invalid_inputs(win);
    test_arc_native_quarter_image_mask_and_parent_color_round_trip(win);
    test_arc_parent_color_uses_ld_color_encoding(win);
    test_arc_create_with_props_preserves_default_parent_color_when_unspecified(win);
    test_arc_create_with_props_accepts_black_parent_color(win);
    test_arc_legacy_demo0_parity_parameters_map_to_backend(win);
    test_arc_builtin_quarter_source_matches_legacy_backend_tiles(win);
    test_arc_legacy_demo0_source_preserves_initial_rotation_phase();
    test_arc_show_prepares_when_first_active_pfb_is_not_new_frame(win);
    test_arc_init_alias_matches_backend_truth(win);
    test_arc_create_with_props_failure_rolls_back_attached_child(win);
    test_arc_rejects_null_args(win);

    tinyui_app_destroy(app);
    test_arc_destroy_releases_owned_tiles_without_freeing_external_sources();
    return 0;
}
