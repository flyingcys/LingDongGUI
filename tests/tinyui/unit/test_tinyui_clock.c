/*
 * TinyUI clock unit tests — M3 Task 5 L3/L4 harness.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldClock.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/clock.h"

#include <assert.h>
#include <string.h>

static void bind_source(tinyui_image_source_t *source, arm_2d_tile_t *tile)
{
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    tile->tRegion.tSize.iWidth = 8;
    tile->tRegion.tSize.iHeight = 8;
    memcpy(source->_image_private, tile, sizeof(*tile));
}

static void test_clock_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *clock = tinyui_clock_create(root);
    struct tinyui_widget *backend;
    ldClock_t *ld_clock;

    assert(clock != 0);
    backend = (struct tinyui_widget *)(void *)clock;
    assert(backend->kind == TINYUI_BACKEND_WIDGET_CLOCK);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);
    assert(((ldBase_t *)ld_clock)->widgetType == widgetTypeClock);
}

static void test_clock_system_time_step_and_images(tinyui_obj_t *root)
{
    tinyui_obj_t *clock = tinyui_clock_create(root);
    ldClock_t *ld_clock;
    arm_2d_tile_t bg = {0}, hour = {0}, minute = {0}, second = {0};
    tinyui_image_source_t bg_src, hour_src, minute_src, second_src;

    assert(clock != 0);
    ld_clock = (ldClock_t *)((struct tinyui_widget *)(void *)clock)->ld_widget;
    assert(ld_clock != 0);

    assert(tinyui_clock_set_auto_sys_time(clock, 0) == 0);
    assert(ld_clock->isAutoSysTime == 0);
    assert(tinyui_clock_get_use_system_time(clock) == 0);
    assert(tinyui_clock_set_auto_sys_time(clock, 1) == 0);
    assert(ld_clock->isAutoSysTime == 1);

    assert(tinyui_clock_set_step_second(clock, 1) == 0);
    assert(ld_clock->isStepSecond == 1);
    assert(tinyui_clock_get_step_second(clock) == 1);
    assert(tinyui_clock_set_step_second(clock, 0) == 0);
    assert(ld_clock->isStepSecond == 0);

    bind_source(&bg_src, &bg);
    bind_source(&hour_src, &hour);
    bind_source(&minute_src, &minute);
    bind_source(&second_src, &second);

    assert(tinyui_clock_set_background_image(clock, &bg_src) == 0);
    assert(ld_clock->ptBgImgTile == tinyui_image_source_get_image_tile(&bg_src));
    assert(tinyui_clock_set_hour_pointer_image(clock, &hour_src) == 0);
    assert(ld_clock->pointerInfo[0].ptImgTile == tinyui_image_source_get_image_tile(&hour_src));
    assert(tinyui_clock_set_minute_pointer_image(clock, &minute_src) == 0);
    assert(ld_clock->pointerInfo[1].ptImgTile == tinyui_image_source_get_image_tile(&minute_src));
    assert(tinyui_clock_set_second_pointer_image(clock, &second_src) == 0);
    assert(ld_clock->pointerInfo[2].ptImgTile == tinyui_image_source_get_image_tile(&second_src));
}

static void test_clock_rejects_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *clock = tinyui_clock_create(root);
    assert(clock != 0);
    assert(tinyui_clock_create(0) == 0);
    assert(tinyui_clock_set_step_second(clock, 2) == -1);
    assert(tinyui_clock_set_background_image(0, 0) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_clock_create_and_ld_mapping(root);
    test_clock_system_time_step_and_images(root);
    test_clock_rejects_invalid(root);

    tinyui_deinit();
    return 0;
}
