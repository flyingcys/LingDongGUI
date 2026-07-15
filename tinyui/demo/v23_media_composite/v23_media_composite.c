/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * M3 Task 10 家族场景：image / canvas / qrcode / date_time / clock /
 * animation / message_box。固定坐标；message_box confirm 走 L5-E。
 */

#include "v23_media_composite/v23_media_composite.h"
#include "tinyui.h"

#include <stdio.h>

static tinyui_image_source_t g_img_src;
static tinyui_image_source_t g_anim_src;

static void log_event_trace(const char *line)
{
    if (line == NULL || line[0] == '\0') {
        return;
    }
    printf("TINYUI_EVENT_TRACE_LINE=%s\n", line);
    fflush(stdout);
}

static void on_message_confirm(tinyui_obj_t *box, void *user_data)
{
    (void)box;
    (void)user_data;
    log_event_trace("message_box:CONFIRM");
}

static int make_ui(tinyui_obj_t *screen)
{
    tinyui_obj_t *image;
    tinyui_obj_t *canvas;
    tinyui_obj_t *qrcode;
    tinyui_obj_t *date_time;
    tinyui_obj_t *clock;
    tinyui_obj_t *animation;
    tinyui_obj_t *message_box;
    static const char *buttons[] = {"OK", NULL};

    if (screen == NULL) {
        return -1;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    if (tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_NOTE, &g_img_src) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, &g_anim_src)
            != TINYUI_OK) {
        return -1;
    }

    image = tinyui_image_create(screen);
    canvas = tinyui_canvas_create(screen);
    qrcode = tinyui_qrcode_create(screen);
    date_time = tinyui_date_time_create(screen);
    clock = tinyui_clock_create(screen);
    animation = tinyui_animation_create(screen);
    message_box = tinyui_message_box_create(screen);
    if (image == NULL || canvas == NULL || qrcode == NULL || date_time == NULL
        || clock == NULL || animation == NULL || message_box == NULL) {
        return -1;
    }

    if (tinyui_obj_set_pos(image, 16, 16) != TINYUI_OK
        || tinyui_obj_set_size(image, 80, 80) != TINYUI_OK
        || tinyui_image_set_source(image, &g_img_src) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(canvas, 112, 16) != TINYUI_OK
        || tinyui_obj_set_size(canvas, 100, 80) != TINYUI_OK
        || tinyui_canvas_clear(canvas) != 0
        || tinyui_canvas_fill_rect(canvas, 8, 8, 84, 64, 0x1F6FEBU, 255) != 0
        || tinyui_canvas_draw_line(canvas, 8, 8, 92, 72, 2, 0xFFFFFFU, 255, 255) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(qrcode, 232, 16) != TINYUI_OK
        || tinyui_obj_set_size(qrcode, 96, 96) != TINYUI_OK
        || tinyui_qrcode_set_text(qrcode, "tinyui-v23") != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(date_time, 16, 120) != TINYUI_OK
        || tinyui_obj_set_size(date_time, 180, 28) != TINYUI_OK
        || tinyui_date_time_set_date(date_time, 2026, 7, 15) != 0
        || tinyui_date_time_set_time(date_time, 8, 47, 0) != 0
        || tinyui_date_time_set_use_system_time(date_time, 0) != 0
        || tinyui_date_time_set_text_color(date_time, 0x102030U) != 0
        || tinyui_date_time_set_bg_color(date_time, 0xE8EEF5U) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(clock, 220, 120) != TINYUI_OK
        || tinyui_obj_set_size(clock, 100, 100) != TINYUI_OK
        || tinyui_clock_set_use_system_time(clock, 0) != 0) {
        return -1;
    }

    /* Place animation away from full-screen keyboard-like overlaps; use paper tile. */
    if (tinyui_obj_set_pos(animation, 340, 40) != TINYUI_OK
        || tinyui_obj_set_size(animation, 64, 64) != TINYUI_OK
        || tinyui_animation_set_source(animation, &g_anim_src) != 0
        || tinyui_animation_set_period_ms(animation, 120) != 0
        || tinyui_animation_show_frame(animation, 0) != 0) {
        return -1;
    }

    /* Keep native-ish layout so the confirm button hit-test remains valid. */
    if (tinyui_obj_set_pos(message_box, 16, 160) != TINYUI_OK
        || tinyui_message_box_set_layout(message_box, 260, 140) != 0
        || tinyui_message_box_set_title(message_box, "Notice") != 0
        || tinyui_message_box_set_message(message_box, "Media OK") != 0
        || tinyui_message_box_set_buttons(message_box, buttons, 1) != 0
        || tinyui_message_box_set_bg_color(message_box, 0xE8EEF5U) != 0) {
        return -1;
    }
    tinyui_message_box_set_on_confirm(message_box, on_message_confirm, NULL);

    printf("TINYUI_SCENARIO=v23_media_composite\n");
    fflush(stdout);
    return 0;
}

void tinyui_demo_v23_media_composite(void)
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

void tinyui_demo_v23_media_composite_frame(unsigned int elapsed_ms)
{
    (void)elapsed_ms;
}
