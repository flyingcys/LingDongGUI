/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * M3 Task 10 家族场景：switch / progress_bar / progress_wheel / arc / gauge。
 * 使用固定坐标保证 L5-V region 确定性；可操作控件仅 switch 走 L5-E。
 */

#include "v23_value_instruments/v23_value_instruments.h"
#include "tinyui.h"

#include <stdio.h>

static void log_event_trace(const char *line)
{
    if (line == NULL || line[0] == '\0') {
        return;
    }
    printf("TINYUI_EVENT_TRACE_LINE=%s\n", line);
    fflush(stdout);
}

static void on_switch_event(const tinyui_event_t *event)
{
    char line[64];

    if (event == NULL || event->code != TINYUI_EVENT_VALUE_CHANGED) {
        return;
    }
    snprintf(line, sizeof(line), "switch:VALUE_CHANGED:%d", (int)event->data.value);
    log_event_trace(line);
}

static int make_ui(tinyui_obj_t *screen)
{
    tinyui_obj_t *sw;
    tinyui_obj_t *bar;
    tinyui_obj_t *wheel;
    tinyui_obj_t *arc;
    tinyui_obj_t *gauge;

    if (screen == NULL) {
        return -1;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    sw = tinyui_switch_create(screen);
    bar = tinyui_progress_bar_create(screen);
    wheel = tinyui_progress_wheel_create(screen);
    arc = tinyui_arc_create(screen);
    gauge = tinyui_gauge_create(screen);
    if (sw == NULL || bar == NULL || wheel == NULL || arc == NULL || gauge == NULL) {
        return -1;
    }

    if (tinyui_obj_set_pos(sw, 24, 24) != TINYUI_OK
        || tinyui_obj_set_size(sw, 96, 36) != TINYUI_OK
        || tinyui_switch_set_checked(sw, 0) != 0
        || tinyui_switch_set_horizontal(sw, 1) != 0
        || tinyui_switch_set_on_toggled(sw, on_switch_event, NULL) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(bar, 24, 80) != TINYUI_OK
        || tinyui_obj_set_size(bar, 280, 28) != TINYUI_OK
        || tinyui_progress_bar_set_percent(bar, 62) != 0
        || tinyui_progress_bar_set_horizontal(bar, 1) != 0
        || tinyui_progress_bar_set_color(bar, 0xD0D7DEU, 0x1F6FEBU) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(wheel, 24, 132) != TINYUI_OK
        || tinyui_obj_set_size(wheel, 96, 96) != TINYUI_OK
        || tinyui_progress_wheel_set_percent(wheel, 45) != 0
        || tinyui_progress_wheel_set_dot_enabled(wheel, 1) != 0
        || tinyui_progress_wheel_set_dot_color(wheel, 0x1F6FEBU) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(arc, 148, 132) != TINYUI_OK
        || tinyui_obj_set_size(arc, 96, 96) != TINYUI_OK
        || tinyui_arc_set_background_angle(arc, 0.0f, 270.0f) != 0
        || tinyui_arc_set_foreground_angle(arc, 180.0f) != 0
        || tinyui_arc_set_color(arc, 0xD0D7DEU, 0x1F6FEBU) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(gauge, 272, 132) != TINYUI_OK
        || tinyui_obj_set_size(gauge, 96, 96) != TINYUI_OK
        || tinyui_gauge_set_angle(gauge, 120.0f) != 0
        || tinyui_gauge_set_pointer_color(gauge, 0xCF222EU) != 0) {
        return -1;
    }

    printf("TINYUI_SCENARIO=v23_value_instruments\n");
    fflush(stdout);
    return 0;
}

void tinyui_demo_v23_value_instruments(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();

    if (screen == NULL) {
        return;
    }
    if (make_ui(screen) != 0) {
        return;
    }
    (void)tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0);
}

void tinyui_demo_v23_value_instruments_frame(unsigned int elapsed_ms)
{
    (void)elapsed_ms;
}
