#include "picoui/app.h"
#include "picoui/clock.h"
#include "picoui/image.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldClock.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

static void test_clock_create_and_props(struct picoui_window *win)
{
    int user_cookie = 17;
    struct picoui_clock_props props = {
        .id = "clock_props",
        .style_class = "clock",
        .user_data = &user_cookie,
        .step_second = 1,
    };
    struct picoui_clock *clock =
        picoui_clock_create((struct picoui_widget *)win, "clock");
    struct picoui_clock *with_props =
        picoui_clock_create_with_props((struct picoui_widget *)win, &props);

    assert(clock != 0);
    assert(with_props != 0);
    assert(picoui_clock_get_step_second(clock) == 0);
    assert(picoui_clock_get_step_second(with_props) == props.step_second);
}

static void test_clock_step_second_state(struct picoui_window *win)
{
    struct picoui_clock *clock =
        picoui_clock_create((struct picoui_widget *)win, "clock_step_second");

    assert(clock != 0);
    assert(picoui_clock_set_step_second(clock, 0) == 0);
    assert(picoui_clock_get_step_second(clock) == 0);
    assert(picoui_clock_set_step_second(clock, 1) == 0);
    assert(picoui_clock_get_step_second(clock) == 1);
    assert(picoui_clock_set_step_second(clock, -1) == -1);
    assert(picoui_clock_get_step_second(clock) == 1);
    assert(picoui_clock_set_step_second(clock, 2) == -1);
    assert(picoui_clock_get_step_second(clock) == 1);
}

static void test_clock_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_clock *clock =
        picoui_clock_create((struct picoui_widget *)win, "clock_invalid");

    assert(clock != 0);
    assert(picoui_clock_create(0, "clock") == 0);
    assert(picoui_clock_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_clock_create_with_props(0,
                                          &(struct picoui_clock_props){
                                              .id = "bad_parent",
                                              .step_second = 0,
                                          }) == 0);
    assert(picoui_clock_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_clock_create_with_props((struct picoui_widget *)win,
                                          &(struct picoui_clock_props){
                                              .step_second = 0,
                                          }) == 0);
    assert(picoui_clock_create_with_props((struct picoui_widget *)win,
                                          &(struct picoui_clock_props){
                                              .id = "bad_step_second_low",
                                              .step_second = -1,
                                          }) == 0);
    assert(picoui_clock_create_with_props((struct picoui_widget *)win,
                                          &(struct picoui_clock_props){
                                              .id = "bad_step_second_high",
                                              .step_second = 2,
                                          }) == 0);
    assert(picoui_clock_set_step_second(0, 1) == -1);
    assert(picoui_clock_get_step_second(0) == -1);
    assert(picoui_clock_set_step_second(clock, -1) == -1);
    assert(picoui_clock_set_step_second(clock, 2) == -1);
    assert(picoui_clock_get_step_second(clock) == 0);
}

static void test_clock_release_contract_covers_time_source_and_configuration_boundary(
    struct picoui_window *win)
{
    struct picoui_clock *clock =
        picoui_clock_create_with_props((struct picoui_widget *)win,
                                       &(struct picoui_clock_props){
                                           .id = "clock_release_ready",
                                           .style_class = "clock-card",
                                           .step_second = 1,
                                       });
    struct picoui_backend_widget *backend;
    ldClock_t *ld_clock;

    assert(clock != 0);
    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_CLOCK);
    assert(backend->style_class == (const char *)"clock-card");
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    assert(picoui_clock_get_step_second(clock) == 1);
    assert(ld_clock->isStepSecond == true);
    assert(ld_clock->pointerInfo[0].ptImgTile != 0);
    assert(ld_clock->pointerInfo[1].ptImgTile != 0);
    assert(ld_clock->pointerInfo[2].ptImgTile != 0);
    assert(picoui_clock_get_use_system_time(clock) == 1);
    assert(picoui_clock_set_step_second(clock, 0) == 0);
    assert(picoui_clock_get_step_second(clock) == 0);
    assert(ld_clock->isStepSecond == false);
}

static void test_clock_system_time_provider_round_trip(struct picoui_window *win)
{
    struct picoui_clock *clock =
        picoui_clock_create((struct picoui_widget *)win, "clock_system_provider");
    struct picoui_backend_widget *backend;
    ldClock_t *ld_clock;
    float frozen_radian;

    assert(clock != 0);
    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    assert(picoui_clock_set_use_system_time(clock, 0) == 0);
    assert(picoui_clock_get_use_system_time(clock) == 0);
    ldClock_on_frame_start(((struct picoui_backend_app_state *)backend->owner->backend_app)->ld_scene, ld_clock);
    frozen_radian = ld_clock->pointerInfo[2].radian;
    ldClock_on_frame_start(((struct picoui_backend_app_state *)backend->owner->backend_app)->ld_scene, ld_clock);
    assert(ld_clock->pointerInfo[2].radian == frozen_radian);

    assert(picoui_clock_set_use_system_time(clock, 1) == 0);
    assert(picoui_clock_get_use_system_time(clock) == 1);
    assert(picoui_clock_set_use_system_time(0, 1) == -1);
    assert(picoui_clock_get_use_system_time(0) == -1);
}

static void test_clock_native_background_pointer_mask_and_anchor_round_trip(struct picoui_window *win)
{
    struct picoui_clock *clock =
        picoui_clock_create((struct picoui_widget *)win, "clock_native_assets");
    struct picoui_backend_widget *backend;
    ldClock_t *ld_clock;
    arm_2d_tile_t bg_tile = {
        .tRegion = {
            .tSize = { .iWidth = 40, .iHeight = 40 },
        },
    };
    arm_2d_tile_t bg_mask = {
        .tRegion = {
            .tSize = { .iWidth = 40, .iHeight = 40 },
        },
    };
    arm_2d_tile_t hour_tile = {
        .tRegion = {
            .tSize = { .iWidth = 8, .iHeight = 30 },
        },
    };
    arm_2d_tile_t hour_mask = {
        .tRegion = {
            .tSize = { .iWidth = 8, .iHeight = 30 },
        },
    };
    arm_2d_tile_t minute_tile = {
        .tRegion = {
            .tSize = { .iWidth = 10, .iHeight = 44 },
        },
    };
    arm_2d_tile_t minute_mask = {
        .tRegion = {
            .tSize = { .iWidth = 10, .iHeight = 44 },
        },
    };
    arm_2d_tile_t second_tile = {
        .tRegion = {
            .tSize = { .iWidth = 6, .iHeight = 52 },
        },
    };
    arm_2d_tile_t second_mask = {
        .tRegion = {
            .tSize = { .iWidth = 6, .iHeight = 52 },
        },
    };
    struct picoui_image_source bg_source = {
        .img_tile = &bg_tile,
        .mask_tile = &bg_mask,
    };
    struct picoui_image_source hour_source = {
        .img_tile = &hour_tile,
        .mask_tile = &hour_mask,
    };
    struct picoui_image_source minute_source = {
        .img_tile = &minute_tile,
        .mask_tile = &minute_mask,
    };
    struct picoui_image_source second_source = {
        .img_tile = &second_tile,
        .mask_tile = &second_mask,
    };

    assert(clock != 0);
    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    assert(picoui_clock_set_background_source(clock, &bg_source) == 0);
    assert(picoui_clock_set_hour_pointer_source(clock, &hour_source) == 0);
    assert(picoui_clock_set_minute_pointer_source(clock, &minute_source) == 0);
    assert(picoui_clock_set_second_pointer_source(clock, &second_source) == 0);
    assert(picoui_clock_set_mask_color(clock, 0x123456U) == 0);
    assert(picoui_clock_set_hour_anchor(clock, 3.5f, 21.0f) == 0);
    assert(picoui_clock_set_minute_anchor(clock, 4.5f, 31.0f) == 0);
    assert(picoui_clock_set_second_anchor(clock, 2.0f, 37.0f) == 0);

    assert(ld_clock->ptBgImgTile == &bg_tile);
    assert(ld_clock->ptBgMaskTile == &bg_mask);
    assert(ld_clock->bgMaskColor == (ldColor)0x123456U);
    assert(ld_clock->pointerInfo[0].ptImgTile == &hour_tile);
    assert(ld_clock->pointerInfo[0].ptMaskTile == &hour_mask);
    assert(ld_clock->pointerInfo[1].ptImgTile == &minute_tile);
    assert(ld_clock->pointerInfo[1].ptMaskTile == &minute_mask);
    assert(ld_clock->pointerInfo[2].ptImgTile == &second_tile);
    assert(ld_clock->pointerInfo[2].ptMaskTile == &second_mask);
    assert(ld_clock->pointerInfo[0].maskColor == (ldColor)0x123456U);
    assert(ld_clock->pointerInfo[1].maskColor == (ldColor)0x123456U);
    assert(ld_clock->pointerInfo[2].maskColor == (ldColor)0x123456U);
    assert(ld_clock->pointerInfo[0].rotationCentre.fX == 3.5f);
    assert(ld_clock->pointerInfo[0].rotationCentre.fY == 21.0f);
    assert(ld_clock->pointerInfo[1].rotationCentre.fX == 4.5f);
    assert(ld_clock->pointerInfo[1].rotationCentre.fY == 31.0f);
    assert(ld_clock->pointerInfo[2].rotationCentre.fX == 2.0f);
    assert(ld_clock->pointerInfo[2].rotationCentre.fY == 37.0f);

    assert(picoui_clock_set_background_source(0, &bg_source) == -1);
    assert(picoui_clock_set_hour_pointer_source(0, &hour_source) == -1);
    assert(picoui_clock_set_minute_pointer_source(0, &minute_source) == -1);
    assert(picoui_clock_set_second_pointer_source(0, &second_source) == -1);
    assert(picoui_clock_set_mask_color(0, 0x000000U) == -1);
    assert(picoui_clock_set_hour_anchor(0, 0.0f, 0.0f) == -1);
    assert(picoui_clock_set_minute_anchor(0, 0.0f, 0.0f) == -1);
    assert(picoui_clock_set_second_anchor(0, 0.0f, 0.0f) == -1);
}

static void test_clock_init_and_image_aliases_round_trip(struct picoui_window *win)
{
    struct picoui_clock *clock =
        picoui_clock_init((struct picoui_widget *)win, "clock_alias");
    struct picoui_backend_widget *backend;
    ldClock_t *ld_clock;
    arm_2d_tile_t bg_tile = {
        .tRegion = {
            .tSize = { .iWidth = 48, .iHeight = 48 },
        },
    };
    arm_2d_tile_t hour_tile = {
        .tRegion = {
            .tSize = { .iWidth = 8, .iHeight = 28 },
        },
    };
    arm_2d_tile_t minute_tile = {
        .tRegion = {
            .tSize = { .iWidth = 8, .iHeight = 36 },
        },
    };
    arm_2d_tile_t second_tile = {
        .tRegion = {
            .tSize = { .iWidth = 6, .iHeight = 42 },
        },
    };
    struct picoui_image_source bg_source = {
        .img_tile = &bg_tile,
    };
    struct picoui_image_source hour_source = {
        .img_tile = &hour_tile,
    };
    struct picoui_image_source minute_source = {
        .img_tile = &minute_tile,
    };
    struct picoui_image_source second_source = {
        .img_tile = &second_tile,
    };

    assert(clock != 0);
    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    assert(picoui_clock_set_background_image(clock, &bg_source) == 0);
    assert(picoui_clock_set_hour_pointer_image(clock, &hour_source) == 0);
    assert(picoui_clock_set_minute_pointer_image(clock, &minute_source) == 0);
    assert(picoui_clock_set_second_pointer_image(clock, &second_source) == 0);
    assert(picoui_clock_set_step_second(clock, 1) == 0);

    assert(ld_clock->ptBgImgTile == &bg_tile);
    assert(ld_clock->pointerInfo[0].ptImgTile == &hour_tile);
    assert(ld_clock->pointerInfo[1].ptImgTile == &minute_tile);
    assert(ld_clock->pointerInfo[2].ptImgTile == &second_tile);
    assert(ld_clock->isStepSecond == true);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_clock_create_and_props(win);
    test_clock_step_second_state(win);
    test_clock_rejects_invalid_inputs(win);
    test_clock_release_contract_covers_time_source_and_configuration_boundary(win);
    test_clock_system_time_provider_round_trip(win);
    test_clock_native_background_pointer_mask_and_anchor_round_trip(win);
    test_clock_init_and_image_aliases_round_trip(win);

    picoui_app_destroy(app);
    return 0;
}
