/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "animation_basic/animation_basic.h"
#include "tinyui.h"

#include <stdio.h>

/* Descriptor must outlive builder return — image object borrows it. */
static tinyui_image_source_t s_anim_src;
static tinyui_obj_t *s_anim;
static int s_frame;

static void on_anim_timer(tinyui_timer_t *timer, void *user_data)
{
    tinyui_obj_t *anim = (tinyui_obj_t *)user_data;
    int next;

    (void)timer;
    if (anim == NULL) {
        return;
    }
    next = s_frame + 1;
    if (tinyui_animation_show_frame(anim, next) != 0) {
        next = 0;
        (void)tinyui_animation_show_frame(anim, next);
    }
    s_frame = next;
}

tinyui_result_t tinyui_demo_animation_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *anim;
    tinyui_timer_t *timer;
    tinyui_animation_props_t props;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    if (tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_LETTER_PAPER,
                                         &s_anim_src) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    title = tinyui_label_create(screen);
    if (title == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_obj_set_pos(title, 32, 24) != TINYUI_OK
        || tinyui_obj_set_size(title, 200, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Animation") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    props.fields = TINYUI_ANIMATION_FIELD_WIDTH
                 | TINYUI_ANIMATION_FIELD_HEIGHT
                 | TINYUI_ANIMATION_FIELD_PERIOD_MS
                 | TINYUI_ANIMATION_FIELD_SOURCE;
    props.width = 64;
    props.height = 64;
    props.period_ms = 120;
    props.source = &s_anim_src;

    anim = tinyui_animation_create_with_props(screen, &props);
    if (anim == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_obj_set_pos(anim, 160, 100) != TINYUI_OK
        || tinyui_animation_show_frame(anim, 0) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    s_anim = anim;
    s_frame = 0;
    timer = tinyui_timer_create(120U, true, on_anim_timer, anim);
    if (timer == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_timer_start(timer) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    printf("TINYUI_SCENARIO=animation_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
