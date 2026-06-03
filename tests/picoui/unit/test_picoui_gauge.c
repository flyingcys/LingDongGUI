#include "picoui/app.h"
#include "picoui/gauge.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldGauge.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

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
    assert(picoui_gauge_get_pointer_color(with_props) != 0U);
    assert(picoui_gauge_get_auto_move(with_props) == props.auto_move);
}

static void test_gauge_value_and_pointer_contract_match_backend_truth(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_angle");

    assert(gauge != 0);
    assert(picoui_gauge_set_angle(gauge, 90.0f) == 0);
    assert(picoui_gauge_get_angle(gauge) == 90.0f);
    assert(picoui_gauge_set_pointer_color(gauge, 0xAABBCC) == 0);
    assert(picoui_gauge_get_pointer_color(gauge) != 0U);
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

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_gauge_create_and_props(win);
    test_gauge_value_and_pointer_contract_match_backend_truth(win);
    test_gauge_rejects_invalid_inputs(win);
    test_gauge_native_background_pointer_and_centre_offset_round_trip(win);
    test_gauge_native_trail_and_progress_bar_round_trip(win);
    test_gauge_init_and_shared_base_aliases_round_trip(win);
    test_gauge_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
