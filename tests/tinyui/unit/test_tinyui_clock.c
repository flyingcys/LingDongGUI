#include "app.h"
#include "clock.h"
#include "image.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldClock.h"
#include "internal.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);

static void assert_source_lacks_function_definition(const char *path, const char *symbol)
{
    char needle[256];

    snprintf(needle, sizeof(needle), "static int %s(", symbol);
    if (strstr(symbol, "get_ld") != 0) {
        snprintf(needle, sizeof(needle), "static ldClock_t *%s(", symbol);
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

static struct tracked_free_entry g_tracked_frees[32];
static int g_tracked_free_count = 0;

static void tracked_free_reset(void)
{
    int i;

    for (i = 0; i < 32; ++i) {
        g_tracked_frees[i].ptr = NULL;
        g_tracked_frees[i].count = 0;
    }
    g_tracked_free_count = 0;
}

static void tracked_free_watch(void *ptr)
{
    assert(g_tracked_free_count < 32);
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

static void test_clock_create_builds_direct_backend_mapping(struct tinyui_window *win)
{
    struct tinyui_clock *clock =
        tinyui_clock_create((struct tinyui_widget *)win, "clock_direct_mapping");
    struct tinyui_backend_widget *backend;
    struct tinyui_backend_widget *parent_backend;
    ldClock_t *ld_clock;

    assert(clock != 0);
    backend = (struct tinyui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct tinyui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_CLOCK);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &clock->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);
    assert(((ldBase_t *)ld_clock)->pInfo == backend);
    assert(tinyui_widget_has_ld_binding(&clock->widget) == 1);
}

static void test_clock_create_and_props(struct tinyui_window *win)
{
    int user_cookie = 17;
    struct tinyui_clock_props props = {
        .id = "clock_props",
        .style_class = "clock",
        .user_data = &user_cookie,
        .step_second = 1,
    };
    struct tinyui_clock *clock =
        tinyui_clock_create((struct tinyui_widget *)win, "clock");
    struct tinyui_clock *with_props =
        tinyui_clock_create_with_props((struct tinyui_widget *)win, &props);

    assert(clock != 0);
    assert(with_props != 0);
    assert(tinyui_clock_get_step_second(clock) == 0);
    assert(tinyui_clock_get_step_second(with_props) == props.step_second);
}

static void test_clock_step_second_state(struct tinyui_window *win)
{
    struct tinyui_clock *clock =
        tinyui_clock_create((struct tinyui_widget *)win, "clock_step_second");
    struct tinyui_backend_widget *backend;
    ldClock_t *ld_clock;

    assert(clock != 0);
    backend = (struct tinyui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);
    assert(clock->step_second == 0);
    assert(ld_clock->isStepSecond == false);
    assert(tinyui_clock_set_step_second(clock, 0) == 0);
    assert(clock->step_second == 0);
    assert(tinyui_clock_get_step_second(clock) == 0);
    assert(ld_clock->isStepSecond == false);
    assert(tinyui_clock_set_step_second(clock, 1) == 0);
    assert(clock->step_second == 1);
    assert(tinyui_clock_get_step_second(clock) == 1);
    assert(ld_clock->isStepSecond == true);
    assert(tinyui_clock_set_step_second(clock, -1) == -1);
    assert(clock->step_second == 1);
    assert(tinyui_clock_get_step_second(clock) == 1);
    assert(ld_clock->isStepSecond == true);
    assert(tinyui_clock_set_step_second(clock, 2) == -1);
    assert(clock->step_second == 1);
    assert(tinyui_clock_get_step_second(clock) == 1);
    assert(ld_clock->isStepSecond == true);
}

static void test_clock_rejects_invalid_inputs(struct tinyui_window *win)
{
    struct tinyui_clock *clock =
        tinyui_clock_create((struct tinyui_widget *)win, "clock_invalid");

    assert(clock != 0);
    assert(tinyui_clock_create(0, "clock") == 0);
    assert(tinyui_clock_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_clock_create_with_props(0,
                                          &(struct tinyui_clock_props){
                                              .id = "bad_parent",
                                              .step_second = 0,
                                          }) == 0);
    assert(tinyui_clock_create_with_props((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_clock_create_with_props((struct tinyui_widget *)win,
                                          &(struct tinyui_clock_props){
                                              .step_second = 0,
                                          }) == 0);
    assert(tinyui_clock_create_with_props((struct tinyui_widget *)win,
                                          &(struct tinyui_clock_props){
                                              .id = "bad_step_second_low",
                                              .step_second = -1,
                                          }) == 0);
    assert(tinyui_clock_create_with_props((struct tinyui_widget *)win,
                                          &(struct tinyui_clock_props){
                                              .id = "bad_step_second_high",
                                              .step_second = 2,
                                          }) == 0);
    assert(tinyui_clock_set_step_second(0, 1) == -1);
    assert(tinyui_clock_get_step_second(0) == -1);
    assert(tinyui_clock_set_step_second(clock, -1) == -1);
    assert(tinyui_clock_set_step_second(clock, 2) == -1);
    assert(tinyui_clock_get_step_second(clock) == 0);
}

static void test_clock_release_contract_covers_time_source_and_configuration_boundary(
    struct tinyui_window *win)
{
    struct tinyui_clock *clock =
        tinyui_clock_create_with_props((struct tinyui_widget *)win,
                                       &(struct tinyui_clock_props){
                                           .id = "clock_release_ready",
                                           .style_class = "clock-card",
                                           .step_second = 1,
                                       });
    struct tinyui_backend_widget *backend;
    ldClock_t *ld_clock;

    assert(clock != 0);
    backend = (struct tinyui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_CLOCK);
    assert(backend->style_class == (const char *)"clock-card");
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    assert(tinyui_clock_get_step_second(clock) == 1);
    assert(ld_clock->isStepSecond == true);
    assert(ld_clock->pointerInfo[0].ptImgTile != 0);
    assert(ld_clock->pointerInfo[1].ptImgTile != 0);
    assert(ld_clock->pointerInfo[2].ptImgTile != 0);
    assert(tinyui_clock_get_use_system_time(clock) == 1);
    assert(tinyui_clock_set_step_second(clock, 0) == 0);
    assert(tinyui_clock_get_step_second(clock) == 0);
    assert(ld_clock->isStepSecond == false);
}

static void test_clock_system_time_provider_round_trip(struct tinyui_window *win)
{
    struct tinyui_clock *clock =
        tinyui_clock_create((struct tinyui_widget *)win, "clock_system_provider");
    struct tinyui_backend_widget *backend;
    ldClock_t *ld_clock;
    float frozen_radian;

    assert(clock != 0);
    backend = (struct tinyui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);
    assert(clock->use_system_time == 1);
    assert(ld_clock->isAutoSysTime == true);

    assert(tinyui_clock_set_use_system_time(clock, 0) == 0);
    assert(clock->use_system_time == 0);
    assert(tinyui_clock_get_use_system_time(clock) == 0);
    assert(ld_clock->isAutoSysTime == false);
    ldClock_on_frame_start(backend->owner->ld_scene, ld_clock);
    frozen_radian = ld_clock->pointerInfo[2].radian;
    ldClock_on_frame_start(backend->owner->ld_scene, ld_clock);
    assert(ld_clock->pointerInfo[2].radian == frozen_radian);

    assert(tinyui_clock_set_use_system_time(clock, 1) == 0);
    assert(clock->use_system_time == 1);
    assert(tinyui_clock_get_use_system_time(clock) == 1);
    assert(ld_clock->isAutoSysTime == true);
    assert(tinyui_clock_set_use_system_time(0, 1) == -1);
    assert(tinyui_clock_get_use_system_time(0) == -1);
}

static void test_clock_native_background_pointer_mask_and_anchor_round_trip(struct tinyui_window *win)
{
    struct tinyui_clock *clock =
        tinyui_clock_create((struct tinyui_widget *)win, "clock_native_assets");
    struct tinyui_backend_widget *backend;
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
    struct tinyui_image_source bg_source = {
        .img_tile = &bg_tile,
        .mask_tile = &bg_mask,
    };
    struct tinyui_image_source hour_source = {
        .img_tile = &hour_tile,
        .mask_tile = &hour_mask,
    };
    struct tinyui_image_source minute_source = {
        .img_tile = &minute_tile,
        .mask_tile = &minute_mask,
    };
    struct tinyui_image_source second_source = {
        .img_tile = &second_tile,
        .mask_tile = &second_mask,
    };

    assert(clock != 0);
    backend = (struct tinyui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    assert(tinyui_clock_set_background_source(clock, &bg_source) == 0);
    assert(tinyui_clock_set_hour_pointer_source(clock, &hour_source) == 0);
    assert(tinyui_clock_set_minute_pointer_source(clock, &minute_source) == 0);
    assert(tinyui_clock_set_second_pointer_source(clock, &second_source) == 0);
    assert(tinyui_clock_set_mask_color(clock, 0x123456U) == 0);
    assert(tinyui_clock_set_hour_anchor(clock, 3.5f, 21.0f) == 0);
    assert(tinyui_clock_set_minute_anchor(clock, 4.5f, 31.0f) == 0);
    assert(tinyui_clock_set_second_anchor(clock, 2.0f, 37.0f) == 0);

    assert(clock->background_source == &bg_source);
    assert(clock->hour_pointer_source == &hour_source);
    assert(clock->minute_pointer_source == &minute_source);
    assert(clock->second_pointer_source == &second_source);
    assert(clock->mask_color == 0x123456U);
    assert(clock->hour_anchor_x == 3.5f);
    assert(clock->hour_anchor_y == 21.0f);
    assert(clock->minute_anchor_x == 4.5f);
    assert(clock->minute_anchor_y == 31.0f);
    assert(clock->second_anchor_x == 2.0f);
    assert(clock->second_anchor_y == 37.0f);

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

    assert(tinyui_clock_set_background_source(clock,
                                              &(struct tinyui_image_source){
                                                  .img_tile = NULL,
                                                  .mask_tile = &bg_mask,
                                              }) == -1);
    assert(tinyui_clock_set_hour_pointer_source(clock,
                                                &(struct tinyui_image_source){
                                                    .img_tile = NULL,
                                                    .mask_tile = &hour_mask,
                                                }) == -1);
    assert(tinyui_clock_set_minute_pointer_source(clock,
                                                  &(struct tinyui_image_source){
                                                      .img_tile = NULL,
                                                      .mask_tile = &minute_mask,
                                                  }) == -1);
    assert(tinyui_clock_set_second_pointer_source(clock,
                                                  &(struct tinyui_image_source){
                                                      .img_tile = NULL,
                                                      .mask_tile = &second_mask,
                                                  }) == -1);
    assert(tinyui_clock_set_mask_color(clock, 0x1000000U) == -1);
    assert(clock->background_source == &bg_source);
    assert(clock->hour_pointer_source == &hour_source);
    assert(clock->minute_pointer_source == &minute_source);
    assert(clock->second_pointer_source == &second_source);
    assert(clock->mask_color == 0x123456U);
    assert(ld_clock->ptBgImgTile == &bg_tile);
    assert(ld_clock->ptBgMaskTile == &bg_mask);
    assert(ld_clock->bgMaskColor == (ldColor)0x123456U);
    assert(ld_clock->pointerInfo[0].ptImgTile == &hour_tile);
    assert(ld_clock->pointerInfo[1].ptImgTile == &minute_tile);
    assert(ld_clock->pointerInfo[2].ptImgTile == &second_tile);
    assert(ld_clock->pointerInfo[0].rotationCentre.fX == 3.5f);
    assert(ld_clock->pointerInfo[0].rotationCentre.fY == 21.0f);
    assert(ld_clock->pointerInfo[1].rotationCentre.fX == 4.5f);
    assert(ld_clock->pointerInfo[1].rotationCentre.fY == 31.0f);
    assert(ld_clock->pointerInfo[2].rotationCentre.fX == 2.0f);
    assert(ld_clock->pointerInfo[2].rotationCentre.fY == 37.0f);

    assert(tinyui_clock_set_background_source(0, &bg_source) == -1);
    assert(tinyui_clock_set_hour_pointer_source(0, &hour_source) == -1);
    assert(tinyui_clock_set_minute_pointer_source(0, &minute_source) == -1);
    assert(tinyui_clock_set_second_pointer_source(0, &second_source) == -1);
    assert(tinyui_clock_set_mask_color(0, 0x000000U) == -1);
    assert(tinyui_clock_set_hour_anchor(0, 0.0f, 0.0f) == -1);
    assert(tinyui_clock_set_minute_anchor(0, 0.0f, 0.0f) == -1);
    assert(tinyui_clock_set_second_anchor(0, 0.0f, 0.0f) == -1);
}

static void test_clock_init_and_image_aliases_round_trip(struct tinyui_window *win)
{
    struct tinyui_clock *clock =
        tinyui_clock_init((struct tinyui_widget *)win, "clock_alias");
    struct tinyui_backend_widget *backend;
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
    struct tinyui_image_source bg_source = {
        .img_tile = &bg_tile,
    };
    struct tinyui_image_source hour_source = {
        .img_tile = &hour_tile,
    };
    struct tinyui_image_source minute_source = {
        .img_tile = &minute_tile,
    };
    struct tinyui_image_source second_source = {
        .img_tile = &second_tile,
    };

    assert(clock != 0);
    backend = (struct tinyui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    assert(tinyui_clock_set_background_image(clock, &bg_source) == 0);
    assert(tinyui_clock_set_hour_pointer_image(clock, &hour_source) == 0);
    assert(tinyui_clock_set_minute_pointer_image(clock, &minute_source) == 0);
    assert(tinyui_clock_set_second_pointer_image(clock, &second_source) == 0);
    assert(tinyui_clock_set_step_second(clock, 1) == 0);

    assert(ld_clock->ptBgImgTile == &bg_tile);
    assert(ld_clock->pointerInfo[0].ptImgTile == &hour_tile);
    assert(ld_clock->pointerInfo[1].ptImgTile == &minute_tile);
    assert(ld_clock->pointerInfo[2].ptImgTile == &second_tile);
    assert(ld_clock->isStepSecond == true);
}

static void test_clock_destroy_releases_owned_tiles_without_freeing_external_sources(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_clock *clock;
    struct tinyui_backend_widget *backend;
    ldClock_t *ld_clock;
    arm_2d_tile_t *default_hour_img_tile;
    arm_2d_tile_t *default_hour_mask_tile;
    arm_2d_tile_t *default_minute_img_tile;
    arm_2d_tile_t *default_minute_mask_tile;
    arm_2d_tile_t *default_second_img_tile;
    arm_2d_tile_t *default_second_mask_tile;
    arm_2d_tile_t external_bg_tile = {
        .tRegion = {
            .tSize = { .iWidth = 40, .iHeight = 40 },
        },
    };
    arm_2d_tile_t external_bg_mask = {
        .tRegion = {
            .tSize = { .iWidth = 40, .iHeight = 40 },
        },
    };
    arm_2d_tile_t external_hour_tile = {
        .tRegion = {
            .tSize = { .iWidth = 8, .iHeight = 30 },
        },
    };
    arm_2d_tile_t external_hour_mask = {
        .tRegion = {
            .tSize = { .iWidth = 8, .iHeight = 30 },
        },
    };
    arm_2d_tile_t external_minute_tile = {
        .tRegion = {
            .tSize = { .iWidth = 10, .iHeight = 44 },
        },
    };
    arm_2d_tile_t external_minute_mask = {
        .tRegion = {
            .tSize = { .iWidth = 10, .iHeight = 44 },
        },
    };
    arm_2d_tile_t external_second_tile = {
        .tRegion = {
            .tSize = { .iWidth = 6, .iHeight = 52 },
        },
    };
    arm_2d_tile_t external_second_mask = {
        .tRegion = {
            .tSize = { .iWidth = 6, .iHeight = 52 },
        },
    };
    struct tinyui_image_source bg_source = {
        .img_tile = &external_bg_tile,
        .mask_tile = &external_bg_mask,
    };
    struct tinyui_image_source hour_source = {
        .img_tile = &external_hour_tile,
        .mask_tile = &external_hour_mask,
    };
    struct tinyui_image_source minute_source = {
        .img_tile = &external_minute_tile,
        .mask_tile = &external_minute_mask,
    };
    struct tinyui_image_source second_source = {
        .img_tile = &external_second_tile,
        .mask_tile = &external_second_mask,
    };

    tracked_free_reset();

    assert(app != 0);
    win = tinyui_window_create(app, "clock_destroy_root");
    assert(win != 0);
    clock = tinyui_clock_create((struct tinyui_widget *)win, "clock_destroy");
    assert(clock != 0);
    backend = (struct tinyui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    default_hour_img_tile = ld_clock->pointerInfo[0].ptImgTile;
    default_hour_mask_tile = ld_clock->pointerInfo[0].ptMaskTile;
    default_minute_img_tile = ld_clock->pointerInfo[1].ptImgTile;
    default_minute_mask_tile = ld_clock->pointerInfo[1].ptMaskTile;
    default_second_img_tile = ld_clock->pointerInfo[2].ptImgTile;
    default_second_mask_tile = ld_clock->pointerInfo[2].ptMaskTile;
    tracked_free_watch(default_hour_img_tile);
    tracked_free_watch(default_hour_mask_tile);
    tracked_free_watch(default_minute_img_tile);
    tracked_free_watch(default_minute_mask_tile);
    tracked_free_watch(default_second_img_tile);
    tracked_free_watch(default_second_mask_tile);
    tracked_free_watch(&external_bg_tile);
    tracked_free_watch(&external_bg_mask);
    tracked_free_watch(&external_hour_tile);
    tracked_free_watch(&external_hour_mask);
    tracked_free_watch(&external_minute_tile);
    tracked_free_watch(&external_minute_mask);
    tracked_free_watch(&external_second_tile);
    tracked_free_watch(&external_second_mask);

    assert(tinyui_clock_set_background_source(clock, &bg_source) == 0);
    assert(tinyui_clock_set_hour_pointer_source(clock, &hour_source) == 0);
    assert(tinyui_clock_set_minute_pointer_source(clock, &minute_source) == 0);
    assert(tinyui_clock_set_second_pointer_source(clock, &second_source) == 0);
    tinyui_app_destroy(app);

    assert(tracked_free_count_for(default_hour_img_tile) == 1);
    assert(tracked_free_count_for(default_hour_mask_tile) == 1);
    assert(tracked_free_count_for(default_minute_img_tile) == 1);
    assert(tracked_free_count_for(default_minute_mask_tile) == 1);
    assert(tracked_free_count_for(default_second_img_tile) == 1);
    assert(tracked_free_count_for(default_second_mask_tile) == 1);
    assert(tracked_free_count_for(&external_bg_tile) == 0);
    assert(tracked_free_count_for(&external_bg_mask) == 0);
    assert(tracked_free_count_for(&external_hour_tile) == 0);
    assert(tracked_free_count_for(&external_hour_mask) == 0);
    assert(tracked_free_count_for(&external_minute_tile) == 0);
    assert(tracked_free_count_for(&external_minute_mask) == 0);
    assert(tracked_free_count_for(&external_second_tile) == 0);
    assert(tracked_free_count_for(&external_second_mask) == 0);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    const char *clock_source = "/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/clock.c";

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    assert_source_lacks_function_definition(clock_source, "tinyui_clock_props_are_valid");
    assert_source_lacks_function_definition(clock_source, "tinyui_clock_get_ld");
    assert_source_lacks_function_definition(clock_source, "tinyui_clock_apply_background");
    assert_source_lacks_function_definition(clock_source, "tinyui_clock_apply_pointer");
    assert_source_has_function_definition(clock_source, "static int ", "tinyui_clock_props_are_valid");
    assert_source_has_function_definition(clock_source, "static ldClock_t *", "tinyui_clock_get_ld");
    assert_source_has_function_definition(clock_source, "static int ", "tinyui_clock_apply_background");
    assert_source_has_function_definition(clock_source, "static int ", "tinyui_clock_apply_pointer");

    test_clock_create_builds_direct_backend_mapping(win);
    test_clock_create_and_props(win);
    test_clock_step_second_state(win);
    test_clock_rejects_invalid_inputs(win);
    test_clock_release_contract_covers_time_source_and_configuration_boundary(win);
    test_clock_system_time_provider_round_trip(win);
    test_clock_native_background_pointer_mask_and_anchor_round_trip(win);
    test_clock_init_and_image_aliases_round_trip(win);

    tinyui_app_destroy(app);
    test_clock_destroy_releases_owned_tiles_without_freeing_external_sources();
    return 0;
}
