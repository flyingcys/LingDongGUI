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

#include "backend.h"
#include "internal.h"
#include "ldClock.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

static struct picoui_backend_app_state *picoui_backend_clock_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldClock_t *picoui_backend_clock_get_ld(struct picoui_clock *clock)
{
    struct picoui_backend_widget *backend;

    if (clock == NULL || clock->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_CLOCK || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldClock_t *)backend->ld_widget;
}

static int picoui_backend_clock_apply_background(struct picoui_clock *clock)
{
    ldClock_t *ld_clock = picoui_backend_clock_get_ld(clock);
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

static int picoui_backend_clock_apply_pointer(struct picoui_clock *clock, int index)
{
    ldClock_t *ld_clock = picoui_backend_clock_get_ld(clock);
    struct picoui_image_source *source;
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
 * @brief Create backend for clock
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_clock(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldClock_t *ld_clock;
    arm_2d_tile_t *hour_img_tile;
    arm_2d_tile_t *hour_mask_tile;
    arm_2d_tile_t *minute_img_tile;
    arm_2d_tile_t *minute_mask_tile;
    arm_2d_tile_t *second_img_tile;
    arm_2d_tile_t *second_mask_tile;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_clock_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    hour_img_tile = malloc(sizeof(*hour_img_tile));
    if (hour_img_tile == NULL) {
        free(widget);
        return 0;
    }
    *hour_img_tile = c_tilePointerSecGRAY8;
    hour_img_tile->tRegion.tSize.iHeight = 67;

    hour_mask_tile = malloc(sizeof(*hour_mask_tile));
    if (hour_mask_tile == NULL) {
        free(hour_img_tile);
        free(widget);
        return 0;
    }
    *hour_mask_tile = c_tilePointerSecMask;
    hour_mask_tile->tRegion.tSize.iHeight = 67;

    minute_img_tile = malloc(sizeof(*minute_img_tile));
    if (minute_img_tile == NULL) {
        free(hour_mask_tile);
        free(hour_img_tile);
        free(widget);
        return 0;
    }
    *minute_img_tile = c_tilePointerSecGRAY8;

    minute_mask_tile = malloc(sizeof(*minute_mask_tile));
    if (minute_mask_tile == NULL) {
        free(minute_img_tile);
        free(hour_mask_tile);
        free(hour_img_tile);
        free(widget);
        return 0;
    }
    *minute_mask_tile = c_tilePointerSecMask;

    second_img_tile = malloc(sizeof(*second_img_tile));
    if (second_img_tile == NULL) {
        free(minute_mask_tile);
        free(minute_img_tile);
        free(hour_mask_tile);
        free(hour_img_tile);
        free(widget);
        return 0;
    }
    *second_img_tile = c_tilePointerSecGRAY8;

    second_mask_tile = malloc(sizeof(*second_mask_tile));
    if (second_mask_tile == NULL) {
        free(second_img_tile);
        free(minute_mask_tile);
        free(minute_img_tile);
        free(hour_mask_tile);
        free(hour_img_tile);
        free(widget);
        return 0;
    }
    *second_mask_tile = c_tilePointerSecMask;

    name_id = ++app_state->next_ld_name_id;
    ld_clock = ldClock_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_widget->ld_name_id,
                            0,
                            0,
                            200,
                            200);
    if (ld_clock == NULL) {
        free(second_mask_tile);
        free(second_img_tile);
        free(minute_mask_tile);
        free(minute_img_tile);
        free(hour_mask_tile);
        free(hour_img_tile);
        free(widget);
        return 0;
    }

    ldClockSetHourPointerImage(ld_clock,
                               hour_img_tile,
                               hour_mask_tile,
                               0,
                               (float)(hour_mask_tile->tRegion.tSize.iWidth >> 1),
                               (float)hour_mask_tile->tRegion.tSize.iHeight);
    ldClockSetMinutePointerImage(ld_clock,
                                 minute_img_tile,
                                 minute_mask_tile,
                                 0,
                                 (float)(minute_mask_tile->tRegion.tSize.iWidth >> 1),
                                 (float)minute_mask_tile->tRegion.tSize.iHeight);
    ldClockSetSecondPointerImage(ld_clock,
                                 second_img_tile,
                                 second_mask_tile,
                                 0,
                                 (float)(second_mask_tile->tRegion.tSize.iWidth >> 1),
                                 100.0f);

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_CLOCK;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_clock;
    widget->ld_name_id = name_id;
    widget->value = 0;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set use system time of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_backend_clock_set_use_system_time(struct picoui_clock *clock, int enabled)
{
    ldClock_t *ld_clock = picoui_backend_clock_get_ld(clock);

    if (ld_clock == NULL) {
        return -1;
    }

    ldClockSetAutoSysTime(ld_clock, enabled != 0);
    return 0;
}

/**
 * @brief Get use system time from clock backend
 *
 * @param[out] clock Clock widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_backend_clock_get_use_system_time(struct picoui_clock *clock, int *enabled)
{
    ldClock_t *ld_clock = picoui_backend_clock_get_ld(clock);

    if (ld_clock == NULL || enabled == NULL) {
        return -1;
    }

    *enabled = ld_clock->isAutoSysTime ? 1 : 0;
    return 0;
}

/**
 * @brief Set step second of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] step_second step second
 * @return 0 on success, -1 on failure
 */

int picoui_backend_clock_set_step_second(struct picoui_clock *clock, int step_second)
{
    ldClock_t *ld_clock = picoui_backend_clock_get_ld(clock);
    struct picoui_backend_widget *backend;

    if (ld_clock == NULL || (step_second != 0 && step_second != 1)) {
        return -1;
    }

    ldClockSetStepSecond(ld_clock, step_second != 0);
    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    backend->value = step_second;
    return 0;
}

/**
 * @brief Get step second from clock backend
 *
 * @param[out] clock Clock widget instance
 * @param[in] step_second step second
 * @return 0 on success, -1 on failure
 */

int picoui_backend_clock_get_step_second(struct picoui_clock *clock, int *step_second)
{
    ldClock_t *ld_clock = picoui_backend_clock_get_ld(clock);

    if (ld_clock == NULL || step_second == NULL) {
        return -1;
    }

    *step_second = ld_clock->isStepSecond ? 1 : 0;
    return 0;
}

/**
 * @brief Set background source of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int picoui_backend_clock_set_background_source(struct picoui_clock *clock, struct picoui_image_source *source)
{
    if (clock == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    clock->background_source = source;
    return picoui_backend_clock_apply_background(clock);
}

/**
 * @brief Set hour pointer source of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int picoui_backend_clock_set_hour_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source)
{
    if (clock == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    clock->hour_pointer_source = source;
    return picoui_backend_clock_apply_pointer(clock, 0);
}

/**
 * @brief Set minute pointer source of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int picoui_backend_clock_set_minute_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source)
{
    if (clock == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    clock->minute_pointer_source = source;
    return picoui_backend_clock_apply_pointer(clock, 1);
}

/**
 * @brief Set second pointer source of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int picoui_backend_clock_set_second_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source)
{
    if (clock == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    clock->second_pointer_source = source;
    return picoui_backend_clock_apply_pointer(clock, 2);
}

/**
 * @brief Set mask color of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] mask_color mask color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_clock_set_mask_color(struct picoui_clock *clock, unsigned int mask_color)
{
    if (clock == NULL || mask_color > 0xFFFFFFU) {
        return -1;
    }

    clock->mask_color = mask_color;
    if (picoui_backend_clock_apply_background(clock) != 0
        || picoui_backend_clock_apply_pointer(clock, 0) != 0
        || picoui_backend_clock_apply_pointer(clock, 1) != 0
        || picoui_backend_clock_apply_pointer(clock, 2) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Set hour anchor of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return -1 on failure
 */

int picoui_backend_clock_set_hour_anchor(struct picoui_clock *clock, float x, float y)
{
    if (clock == NULL) {
        return -1;
    }

    clock->hour_anchor_x = x;
    clock->hour_anchor_y = y;
    return picoui_backend_clock_apply_pointer(clock, 0);
}

/**
 * @brief Set minute anchor of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return -1 on failure
 */

int picoui_backend_clock_set_minute_anchor(struct picoui_clock *clock, float x, float y)
{
    if (clock == NULL) {
        return -1;
    }

    clock->minute_anchor_x = x;
    clock->minute_anchor_y = y;
    return picoui_backend_clock_apply_pointer(clock, 1);
}

/**
 * @brief Set second anchor of clock backend
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return -1 on failure
 */

int picoui_backend_clock_set_second_anchor(struct picoui_clock *clock, float x, float y)
{
    if (clock == NULL) {
        return -1;
    }

    clock->second_anchor_x = x;
    clock->second_anchor_y = y;
    return picoui_backend_clock_apply_pointer(clock, 2);
}
