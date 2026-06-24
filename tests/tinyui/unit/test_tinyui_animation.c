#include "animation.h"
#include "app.h"
#include "image.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldAnimation.h"
#include "internal.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);

static void assert_source_lacks_function_definition(const char *path, const char *symbol)
{
    char needle[256];

    snprintf(needle, sizeof(needle), "static int %s(", symbol);
    if (strstr(symbol, "get_ld") != 0) {
        snprintf(needle, sizeof(needle), "static ldAnimation_t *%s(", symbol);
    }
    assert(tinyui_test_source_contains(path, needle) == 0);
}

static void assert_source_has_function_definition(const char *path,
                                                  const char *prefix,
                                                  const char *symbol)
{
    char needle[256];

    snprintf(needle, sizeof(needle), "%s%s(", prefix, symbol);
    assert(tinyui_test_source_contains(path, needle) == 1);
}
static arm_2d_tile_t s_animation_tile = {
    .tRegion = {
        .tSize = {
            .iWidth = 64,
            .iHeight = 16,
        },
    },
};

static arm_2d_tile_t s_animation_grid_tile = {
    .tRegion = {
        .tSize = {
            .iWidth = 32,
            .iHeight = 32,
        },
    },
};

static void test_animation_create_with_props_builds_direct_backend_mapping(struct tinyui_window *win)
{
    struct tinyui_image_source source = {
        .img_tile = &s_animation_tile,
        .mask_tile = 0,
    };
    struct tinyui_animation_props props = {
        .id = "animation_direct_mapping",
        .width = 16,
        .height = 16,
        .period_ms = 120,
        .source = &source,
    };
    struct tinyui_animation *animation =
        tinyui_animation_create_with_props((struct tinyui_widget *)win, &props);
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldAnimation_t *ld_animation;

    assert(animation != 0);
    backend = &animation->widget;
    assert(backend->ld_widget != 0);
    parent_backend = &win->widget;
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_ANIMATION);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_animation = (ldAnimation_t *)backend->ld_widget;
    assert(ld_animation != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
    assert(tinyui_widget_has_ld_binding(&animation->widget) == 1);
    assert(animation->source == &source);
    assert(animation->period_ms == 120);
    assert(animation->width == 16);
    assert(animation->height == 16);
    assert(ld_animation->ptImgTile == &s_animation_tile);
    assert(ld_animation->periodMs == 120);
    assert(ld_animation->showRegion.tLocation.iX == 0);
    assert(ld_animation->showRegion.tLocation.iY == 0);
    assert(ld_animation->showRegion.tSize.iWidth == 16);
    assert(ld_animation->showRegion.tSize.iHeight == 16);
}

static void test_animation_native_image_period_and_frame_round_trip(struct tinyui_window *win)
{
    struct tinyui_image_source source = {
        .img_tile = &s_animation_tile,
        .mask_tile = 0,
    };
    struct tinyui_animation_props props = {
        .id = "animation",
        .width = 16,
        .height = 16,
        .period_ms = 120,
        .source = &source,
    };
    struct tinyui_animation *animation =
        tinyui_animation_create_with_props((struct tinyui_widget *)win, &props);
    struct tinyui_widget *backend;
    ldAnimation_t *ld_animation;

    assert(animation != 0);
    backend = &animation->widget;
    assert(backend->ld_widget != 0);
    ld_animation = (ldAnimation_t *)backend->ld_widget;
    assert(ld_animation != 0);

    assert(ld_animation->ptImgTile == &s_animation_tile);
    assert(ld_animation->periodMs == 120);
    assert(ld_animation->showRegion.tLocation.iX == 0);
    assert(ld_animation->showRegion.tLocation.iY == 0);
    assert(ld_animation->showRegion.tSize.iWidth == 16);
    assert(ld_animation->showRegion.tSize.iHeight == 16);

    assert(tinyui_animation_show_frame(animation, 2) == 0);
    assert(ld_animation->showRegion.tLocation.iX == 32);
    assert(ld_animation->showRegion.tLocation.iY == 0);

    assert(tinyui_animation_show_frame(animation, 4) == -1);
    assert(ld_animation->showRegion.tLocation.iX == 32);
    assert(ld_animation->showRegion.tLocation.iY == 0);
}

static void test_animation_show_frame_advances_across_rows(struct tinyui_window *win)
{
    struct tinyui_image_source source = {
        .img_tile = &s_animation_grid_tile,
        .mask_tile = 0,
    };
    struct tinyui_animation_props props = {
        .id = "animation_grid",
        .width = 16,
        .height = 16,
        .period_ms = 90,
        .source = &source,
    };
    struct tinyui_animation *animation =
        tinyui_animation_create_with_props((struct tinyui_widget *)win, &props);
    struct tinyui_widget *backend;
    ldAnimation_t *ld_animation;

    assert(animation != 0);
    backend = &animation->widget;
    assert(backend->ld_widget != 0);
    ld_animation = (ldAnimation_t *)backend->ld_widget;
    assert(ld_animation != 0);

    assert(tinyui_animation_show_frame(animation, 3) == 0);
    assert(ld_animation->showRegion.tLocation.iX == 16);
    assert(ld_animation->showRegion.tLocation.iY == 16);
    assert(ld_animation->showRegion.tSize.iWidth == 16);
    assert(ld_animation->showRegion.tSize.iHeight == 16);

    assert(tinyui_animation_show_frame(animation, 4) == -1);
    assert(ld_animation->showRegion.tLocation.iX == 16);
    assert(ld_animation->showRegion.tLocation.iY == 16);
}

static void test_animation_init_and_shared_base_aliases_round_trip(struct tinyui_window *win)
{
    struct tinyui_image_source source = {
        .img_tile = &s_animation_tile,
        .mask_tile = 0,
    };
    struct tinyui_animation_props props = {
        .id = "animation_alias_props",
        .width = 16,
        .height = 16,
        .period_ms = 100,
        .source = &source,
    };
    struct tinyui_animation *animation =
        tinyui_animation_create_with_props((struct tinyui_widget *)win, &props);
    struct tinyui_animation *alias =
        tinyui_animation_init((struct tinyui_widget *)win, "animation_alias");
    struct tinyui_widget *backend;
    ldAnimation_t *ld_animation;

    assert(animation != 0);
    backend = &animation->widget;
    assert(backend->ld_widget != 0);
    ld_animation = (ldAnimation_t *)backend->ld_widget;
    assert(ld_animation != 0);

    assert(alias != 0);
    assert(tinyui_widget_set_pos(&animation->widget, 6, 10) == 0);
    assert(((ldBase_t *)ld_animation)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 6);
    assert(((ldBase_t *)ld_animation)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 10);
    assert(tinyui_widget_set_visible(&animation->widget, 0) == 0);
    assert(((ldBase_t *)ld_animation)->isHidden == true);
    assert(tinyui_widget_set_opacity(&animation->widget, 58) == 0);
    assert(((ldBase_t *)ld_animation)->opacity == 58);
    assert(tinyui_widget_set_selectable(&animation->widget, 1) == 0);
    assert(((ldBase_t *)ld_animation)->isSelectable == true);
    assert(tinyui_widget_set_selected(&animation->widget, 1) == 0);
    assert(((ldBase_t *)ld_animation)->isSelected == true);
    assert(tinyui_widget_set_selectable(&animation->widget, 0) == 0);
    assert(((ldBase_t *)ld_animation)->isSelectable == false);
    assert(tinyui_widget_set_corner(&animation->widget, 3) == 0);
    assert(((ldBase_t *)ld_animation)->isCorner == true);
}

static void test_animation_internal_seams_renamed_in_source(void)
{
    const char *animation_source = "tinyui/src/widgets/animation.c";

    /* Phase C2: get_ld helper collapsed — setters cast widget.ld_widget directly. */
    assert_source_lacks_function_definition(animation_source,
                                            "tinyui_animation_get_ld");
    assert_source_has_function_definition(animation_source,
                                          "static int ",
                                          "tinyui_animation_props_are_valid");
    assert_source_has_function_definition(animation_source,
                                          "static void *",
                                          "tinyui_animation_ld_init");
    assert(tinyui_test_source_contains(animation_source, "tinyui_widget_create_leaf(") == 1);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_animation_create_with_props_builds_direct_backend_mapping(win);
    test_animation_native_image_period_and_frame_round_trip(win);
    test_animation_show_frame_advances_across_rows(win);
    test_animation_init_and_shared_base_aliases_round_trip(win);
    test_animation_internal_seams_renamed_in_source();

    tinyui_app_destroy(app);
    return 0;
}
