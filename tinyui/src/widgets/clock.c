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

#include "internal.h"
#include "clock.h"
#include "widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldClock.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;
extern const arm_2d_tile_t c_tileClockface;

static int tinyui_clock_props_are_valid(const struct tinyui_clock_props *props)
{
    return props != 0 && props->id != 0 && (props->step_second == 0 || props->step_second == 1);
}

static ldClock_t *tinyui_clock_get_ld(struct tinyui_clock *clock)
{
    if (clock == NULL || clock->widget.ld_widget == NULL
        || clock->widget.kind != TINYUI_BACKEND_WIDGET_CLOCK) {
        return NULL;
    }

    return (ldClock_t *)clock->widget.ld_widget;
}

static int tinyui_clock_apply_background(struct tinyui_clock *clock)
{
    ldClock_t *ld_clock = tinyui_clock_get_ld(clock);
    arm_2d_tile_t *img_tile;
    arm_2d_tile_t *mask_tile;

    if (ld_clock == NULL) {
        return -1;
    }

    img_tile = clock->background_source != NULL ? clock->background_source->img_tile : ld_clock->ptBgImgTile;
    mask_tile = clock->background_source != NULL ? clock->background_source->mask_tile : ld_clock->ptBgMaskTile;
    ldClockSetBackgroundImage(ld_clock, img_tile, mask_tile, (ldColor)clock->mask_color);
    return 0;
}

static int tinyui_clock_apply_pointer(struct tinyui_clock *clock, int index)
{
    ldClock_t *ld_clock = tinyui_clock_get_ld(clock);
    struct tinyui_image_source *source;
    arm_2d_tile_t *img_tile;
    arm_2d_tile_t *mask_tile;
    float x;
    float y;

    if (ld_clock == NULL) {
        return -1;
    }

    switch (index) {
    case 0:
        source = clock->hour_pointer_source;
        x = clock->hour_anchor_x;
        y = clock->hour_anchor_y;
        break;
    case 1:
        source = clock->minute_pointer_source;
        x = clock->minute_anchor_x;
        y = clock->minute_anchor_y;
        break;
    case 2:
        source = clock->second_pointer_source;
        x = clock->second_anchor_x;
        y = clock->second_anchor_y;
        break;
    default:
        return -1;
    }

    img_tile = source != NULL ? source->img_tile : ld_clock->pointerInfo[index].ptImgTile;
    mask_tile = source != NULL ? source->mask_tile : ld_clock->pointerInfo[index].ptMaskTile;
    switch (index) {
    case 0:
        ldClockSetHourPointerImage(ld_clock, img_tile, mask_tile, (ldColor)clock->mask_color, x, y);
        return 0;
    case 1:
        ldClockSetMinutePointerImage(ld_clock, img_tile, mask_tile, (ldColor)clock->mask_color, x, y);
        return 0;
    case 2:
        ldClockSetSecondPointerImage(ld_clock, img_tile, mask_tile, (ldColor)clock->mask_color, x, y);
        return 0;
    default:
        return -1;
    }
}

/**
 * @brief Create clock widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_clock *tinyui_clock_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_clock *clock;
    struct tinyui_app *app_state;
    ldClock_t *ld_clock;
    arm_2d_tile_t *hour_img_tile;
    arm_2d_tile_t *hour_mask_tile;
    arm_2d_tile_t *minute_img_tile;
    arm_2d_tile_t *minute_mask_tile;
    arm_2d_tile_t *second_img_tile;
    arm_2d_tile_t *second_mask_tile;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }

    app_state = parent->owner;
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    clock = calloc(1, sizeof(*clock));
    if (clock == 0) {
        return 0;
    }

    hour_img_tile = malloc(sizeof(*hour_img_tile));
    if (hour_img_tile == 0) {
        free(clock);
        return 0;
    }
    *hour_img_tile = c_tilePointerSecGRAY8;
    hour_img_tile->tRegion.tSize.iHeight = 67;

    hour_mask_tile = malloc(sizeof(*hour_mask_tile));
    if (hour_mask_tile == 0) {
        free(hour_img_tile);
        free(clock);
        return 0;
    }
    *hour_mask_tile = c_tilePointerSecMask;
    hour_mask_tile->tRegion.tSize.iHeight = 67;

    minute_img_tile = malloc(sizeof(*minute_img_tile));
    if (minute_img_tile == 0) {
        free(hour_mask_tile);
        free(hour_img_tile);
        free(clock);
        return 0;
    }
    *minute_img_tile = c_tilePointerSecGRAY8;

    minute_mask_tile = malloc(sizeof(*minute_mask_tile));
    if (minute_mask_tile == 0) {
        free(minute_img_tile);
        free(hour_mask_tile);
        free(hour_img_tile);
        free(clock);
        return 0;
    }
    *minute_mask_tile = c_tilePointerSecMask;

    second_img_tile = malloc(sizeof(*second_img_tile));
    if (second_img_tile == 0) {
        free(minute_mask_tile);
        free(minute_img_tile);
        free(hour_mask_tile);
        free(hour_img_tile);
        free(clock);
        return 0;
    }
    *second_img_tile = c_tilePointerSecGRAY8;

    second_mask_tile = malloc(sizeof(*second_mask_tile));
    if (second_mask_tile == 0) {
        free(second_img_tile);
        free(minute_mask_tile);
        free(minute_img_tile);
        free(hour_mask_tile);
        free(hour_img_tile);
        free(clock);
        return 0;
    }
    *second_mask_tile = c_tilePointerSecMask;

    name_id = ++app_state->next_ld_name_id;

    ld_clock = ldClock_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent->ld_name_id,
                            0,
                            0,
                            200,
                            200);
    if (ld_clock == 0) {
        free(second_mask_tile);
        free(second_img_tile);
        free(minute_mask_tile);
        free(minute_img_tile);
        free(hour_mask_tile);
        free(hour_img_tile);
        free(clock);
        return 0;
    }

    ldClockBindHourPointerImage(ld_clock,
                                hour_img_tile,
                                hour_mask_tile,
                                0,
                                (float)(hour_mask_tile->tRegion.tSize.iWidth >> 1),
                                (float)hour_mask_tile->tRegion.tSize.iHeight,
                                true,
                                true);
    ldClockBindMinutePointerImage(ld_clock,
                                  minute_img_tile,
                                  minute_mask_tile,
                                  0,
                                  (float)(minute_mask_tile->tRegion.tSize.iWidth >> 1),
                                  (float)minute_mask_tile->tRegion.tSize.iHeight,
                                  true,
                                  true);
    ldClockBindSecondPointerImage(ld_clock,
                                  second_img_tile,
                                  second_mask_tile,
                                  0,
                                  (float)(second_mask_tile->tRegion.tSize.iWidth >> 1),
                                  100.0f,
                                  true,
                                  true);
    ldClockBindBackgroundImage(ld_clock,
                               (arm_2d_tile_t *)&c_tileClockface,
                               NULL,
                               0,
                               false,
                               false);

    clock->id = id;
    clock->widget.ld_widget  = ld_clock;
    clock->widget.ld_name_id = name_id;
    clock->widget.kind       = TINYUI_BACKEND_WIDGET_CLOCK;
    clock->widget.owner      = app_state;
    clock->widget.visible    = 1;
    clock->widget.enabled    = 1;
    clock->widget.value      = 0;
    ((ldBase_t *)ld_clock)->pInfo = &clock->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&clock->widget, app_state);

    clock->mask_color = 0;
    clock->hour_anchor_x = 0.0f;
    clock->hour_anchor_y = 67.0f;
    clock->minute_anchor_x = 0.0f;
    clock->minute_anchor_y = 100.0f;
    clock->second_anchor_x = 0.0f;
    clock->second_anchor_y = 100.0f;
    clock->use_system_time = 1;
    if (tinyui_clock_set_step_second(clock, 0) != 0) {
        free(clock);
        return 0;
    }
    return clock;
}

/**
 * @brief clock init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_clock *tinyui_clock_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_clock_create(parent, id);
}

/**
 * @brief Set use system time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_use_system_time(struct tinyui_clock *clock, int enabled)
{
    ldClock_t *ld_clock;

    if (clock == 0) {
        return -1;
    }

    ld_clock = tinyui_clock_get_ld(clock);
    if (ld_clock == NULL) {
        return -1;
    }

    ldClockSetAutoSysTime(ld_clock, enabled != 0);
    clock->use_system_time = enabled != 0;
    return 0;
}

int tinyui_clock_set_auto_sys_time(struct tinyui_clock *clock, int enabled)
{
    return tinyui_clock_set_use_system_time(clock, enabled);
}

/**
 * @brief Get use system time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @return -1 on failure
 */

int tinyui_clock_get_use_system_time(const struct tinyui_clock *clock)
{
    ldClock_t *ld_clock;

    if (clock == 0) {
        return -1;
    }

    ld_clock = tinyui_clock_get_ld((struct tinyui_clock *)clock);
    if (ld_clock == NULL) {
        return -1;
    }

    ((struct tinyui_clock *)clock)->use_system_time = ld_clock->isAutoSysTime ? 1 : 0;
    return ((struct tinyui_clock *)clock)->use_system_time;
}

/**
 * @brief Create clock widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_clock *tinyui_clock_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_clock_props *props)
{
    struct tinyui_clock *clock;

    if (!tinyui_clock_props_are_valid(props)) {
        return 0;
    }

    clock = tinyui_clock_create(parent, props->id);
    if (clock == 0) {
        return 0;
    }

    if (props->style_class != 0
        && tinyui_widget_set_style_class(&clock->widget, props->style_class) != 0) {
        free(clock);
        return 0;
    }
    if (tinyui_widget_set_user_data(&clock->widget, props->user_data) != 0
        || tinyui_clock_set_step_second(clock, props->step_second) != 0
        || (props->background_source != 0
            && tinyui_clock_set_background_source(clock, props->background_source) != 0)
        || (props->hour_pointer_source != 0
            && tinyui_clock_set_hour_pointer_source(clock, props->hour_pointer_source) != 0)
        || (props->minute_pointer_source != 0
            && tinyui_clock_set_minute_pointer_source(clock, props->minute_pointer_source) != 0)
        || (props->second_pointer_source != 0
            && tinyui_clock_set_second_pointer_source(clock, props->second_pointer_source) != 0)
        || tinyui_clock_set_mask_color(clock, props->mask_color) != 0
        || tinyui_clock_set_hour_anchor(clock, props->hour_anchor_x, props->hour_anchor_y) != 0
        || tinyui_clock_set_minute_anchor(clock, props->minute_anchor_x, props->minute_anchor_y) != 0
        || tinyui_clock_set_second_anchor(clock, props->second_anchor_x, props->second_anchor_y) != 0) {
        free(clock);
        return 0;
    }

    return clock;
}

/**
 * @brief Set step second of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] step_second step second
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_step_second(struct tinyui_clock *clock, int step_second)
{
    ldClock_t *ld_clock;

    if (clock == 0 || (step_second != 0 && step_second != 1)) {
        return -1;
    }

    ld_clock = tinyui_clock_get_ld(clock);
    if (ld_clock == NULL) {
        return -1;
    }

    ldClockSetStepSecond(ld_clock, step_second != 0);
    clock->widget.value = step_second;
    clock->step_second = step_second;
    return 0;
}

/**
 * @brief Get step second of clock widget
 *
 * @param[in] clock Clock widget instance
 * @return -1 on failure
 */

int tinyui_clock_get_step_second(const struct tinyui_clock *clock)
{
    ldClock_t *ld_clock;

    if (clock == 0) {
        return -1;
    }

    ld_clock = tinyui_clock_get_ld((struct tinyui_clock *)clock);
    if (ld_clock == NULL) {
        return -1;
    }

    ((struct tinyui_clock *)clock)->step_second = ld_clock->isStepSecond ? 1 : 0;
    return ((struct tinyui_clock *)clock)->step_second;
}

/**
 * @brief Set background source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_background_source(struct tinyui_clock *clock, struct tinyui_image_source *source)
{
    if (clock == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    clock->background_source = source;
    return tinyui_clock_apply_background(clock);
}

/**
 * @brief Set background image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_background_image(struct tinyui_clock *clock, struct tinyui_image_source *source)
{
    return tinyui_clock_set_background_source(clock, source);
}

/**
 * @brief Set hour pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_hour_pointer_source(struct tinyui_clock *clock, struct tinyui_image_source *source)
{
    if (clock == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    clock->hour_pointer_source = source;
    return tinyui_clock_apply_pointer(clock, 0);
}

/**
 * @brief Set hour pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_hour_pointer_image(struct tinyui_clock *clock, struct tinyui_image_source *source)
{
    return tinyui_clock_set_hour_pointer_source(clock, source);
}

/**
 * @brief Set minute pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_minute_pointer_source(struct tinyui_clock *clock, struct tinyui_image_source *source)
{
    if (clock == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    clock->minute_pointer_source = source;
    return tinyui_clock_apply_pointer(clock, 1);
}

/**
 * @brief Set minute pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_minute_pointer_image(struct tinyui_clock *clock, struct tinyui_image_source *source)
{
    return tinyui_clock_set_minute_pointer_source(clock, source);
}

/**
 * @brief Set second pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_second_pointer_source(struct tinyui_clock *clock, struct tinyui_image_source *source)
{
    if (clock == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    clock->second_pointer_source = source;
    return tinyui_clock_apply_pointer(clock, 2);
}

/**
 * @brief Set second pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_second_pointer_image(struct tinyui_clock *clock, struct tinyui_image_source *source)
{
    return tinyui_clock_set_second_pointer_source(clock, source);
}

/**
 * @brief Set mask color of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] mask_color mask color
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_mask_color(struct tinyui_clock *clock, unsigned int mask_color)
{
    if (clock == 0 || mask_color > 0xFFFFFFU) {
        return -1;
    }

    clock->mask_color = mask_color;
    if (tinyui_clock_apply_background(clock) != 0 || tinyui_clock_apply_pointer(clock, 0) != 0
        || tinyui_clock_apply_pointer(clock, 1) != 0
        || tinyui_clock_apply_pointer(clock, 2) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Set hour anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_hour_anchor(struct tinyui_clock *clock, float x, float y)
{
    if (clock == 0) {
        return -1;
    }

    clock->hour_anchor_x = x;
    clock->hour_anchor_y = y;
    return tinyui_clock_apply_pointer(clock, 0);
}

/**
 * @brief Set minute anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_minute_anchor(struct tinyui_clock *clock, float x, float y)
{
    if (clock == 0) {
        return -1;
    }

    clock->minute_anchor_x = x;
    clock->minute_anchor_y = y;
    return tinyui_clock_apply_pointer(clock, 1);
}

/**
 * @brief Set second anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_second_anchor(struct tinyui_clock *clock, float x, float y)
{
    if (clock == 0) {
        return -1;
    }

    clock->second_anchor_x = x;
    clock->second_anchor_y = y;
    return tinyui_clock_apply_pointer(clock, 2);
}
