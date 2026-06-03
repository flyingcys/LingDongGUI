#include "picoui/animation.h"
#include "picoui/app.h"
#include "picoui/image.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldAnimation.h"
#include "internal.h"

#include <assert.h>

static arm_2d_tile_t s_animation_tile = {
    .tRegion = {
        .tSize = {
            .iWidth = 64,
            .iHeight = 16,
        },
    },
};

static void test_animation_native_image_period_and_frame_round_trip(struct picoui_window *win)
{
    struct picoui_image_source source = {
        .img_tile = &s_animation_tile,
        .mask_tile = 0,
    };
    struct picoui_animation_props props = {
        .id = "animation",
        .width = 16,
        .height = 16,
        .period_ms = 120,
        .source = &source,
    };
    struct picoui_animation *animation =
        picoui_animation_create_with_props((struct picoui_widget *)win, &props);
    struct picoui_backend_widget *backend;
    ldAnimation_t *ld_animation;

    assert(animation != 0);
    backend = (struct picoui_backend_widget *)animation->widget.backend_widget;
    assert(backend != 0);
    ld_animation = (ldAnimation_t *)backend->ld_widget;
    assert(ld_animation != 0);

    assert(ld_animation->ptImgTile == &s_animation_tile);
    assert(ld_animation->periodMs == 120);
    assert(ld_animation->showRegion.tLocation.iX == 0);
    assert(ld_animation->showRegion.tLocation.iY == 0);
    assert(ld_animation->showRegion.tSize.iWidth == 16);
    assert(ld_animation->showRegion.tSize.iHeight == 16);

    assert(picoui_animation_show_frame(animation, 2) == 0);
    assert(ld_animation->showRegion.tLocation.iX == 32);
    assert(ld_animation->showRegion.tLocation.iY == 0);

    assert(picoui_animation_show_frame(animation, 4) == -1);
    assert(ld_animation->showRegion.tLocation.iX == 32);
    assert(ld_animation->showRegion.tLocation.iY == 0);
}

static void test_animation_init_and_shared_base_aliases_round_trip(struct picoui_window *win)
{
    struct picoui_image_source source = {
        .img_tile = &s_animation_tile,
        .mask_tile = 0,
    };
    struct picoui_animation_props props = {
        .id = "animation_alias_props",
        .width = 16,
        .height = 16,
        .period_ms = 100,
        .source = &source,
    };
    struct picoui_animation *animation =
        picoui_animation_create_with_props((struct picoui_widget *)win, &props);
    struct picoui_animation *alias =
        picoui_animation_init((struct picoui_widget *)win, "animation_alias");
    struct picoui_backend_widget *backend;
    ldAnimation_t *ld_animation;

    assert(animation != 0);
    backend = (struct picoui_backend_widget *)animation->widget.backend_widget;
    assert(backend != 0);
    ld_animation = (ldAnimation_t *)backend->ld_widget;
    assert(ld_animation != 0);

    assert(alias != 0);
    assert(picoui_widget_set_pos(&animation->widget, 6, 10) == 0);
    assert(((ldBase_t *)ld_animation)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 6);
    assert(((ldBase_t *)ld_animation)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 10);
    assert(picoui_widget_set_visible(&animation->widget, 0) == 0);
    assert(((ldBase_t *)ld_animation)->isHidden == true);
    assert(picoui_widget_set_opacity(&animation->widget, 58) == 0);
    assert(((ldBase_t *)ld_animation)->opacity == 58);
    assert(picoui_widget_set_selectable(&animation->widget, 1) == 0);
    assert(((ldBase_t *)ld_animation)->isSelectable == true);
    assert(picoui_widget_set_selected(&animation->widget, 1) == 0);
    assert(((ldBase_t *)ld_animation)->isSelected == true);
    assert(picoui_widget_set_selectable(&animation->widget, 0) == 0);
    assert(((ldBase_t *)ld_animation)->isSelectable == false);
    assert(picoui_widget_set_corner(&animation->widget, 3) == 0);
    assert(((ldBase_t *)ld_animation)->isCorner == true);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_animation_native_image_period_and_frame_round_trip(win);
    test_animation_init_and_shared_base_aliases_round_trip(win);

    picoui_app_destroy(app);
    return 0;
}
