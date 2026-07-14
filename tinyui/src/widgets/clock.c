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
#include "widgets/clock.h"
#include "internal/widget_legacy.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldClock.h"


static struct tinyui_clock *tinyui_clock_as_clock(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_CLOCK)) {
        return 0;
    }
    return (struct tinyui_clock *)w;
}

static const struct tinyui_clock *tinyui_clock_as_clock_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_CLOCK)) {
        return 0;
    }
    return (const struct tinyui_clock *)w;
}


extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;
extern const arm_2d_tile_t c_tileClockface;

/* ---- test seam state ---- */

struct tinyui_clock_create_ctx {
    arm_2d_tile_t *hour_img_tile;
    arm_2d_tile_t *hour_mask_tile;
    arm_2d_tile_t *minute_img_tile;
    arm_2d_tile_t *minute_mask_tile;
    arm_2d_tile_t *second_img_tile;
    arm_2d_tile_t *second_mask_tile;
};


static void tinyui_clock_rollback(struct tinyui_clock *clock)
{
    if (clock == 0) {
        return;
    }
    if (clock->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&clock->widget);
    } else {
        ldFree(clock);
    }
}

static void *tinyui_runtime_internal_clock_ld_init(void *ctx,
                                  struct ld_scene_t *scene,
                                  uint16_t name_id,
                                  uint16_t parent_name_id)
{
    struct tinyui_clock_create_ctx *create_ctx = (struct tinyui_clock_create_ctx *)ctx;
    ldClock_t *ld_clock;

    if (create_ctx == 0
        || create_ctx->hour_img_tile == 0
        || create_ctx->hour_mask_tile == 0
        || create_ctx->minute_img_tile == 0
        || create_ctx->minute_mask_tile == 0
        || create_ctx->second_img_tile == 0
        || create_ctx->second_mask_tile == 0) {
        return 0;
    }

    ld_clock = ldClock_init(scene,
                            NULL,
                            name_id,
                            parent_name_id,
                            0,
                            0,
                            200,
                            200);
    if (ld_clock == 0) {
        return 0;
    }

    ldClockBindHourPointerImage(ld_clock,
                                create_ctx->hour_img_tile,
                                create_ctx->hour_mask_tile,
                                0,
                                (float)(create_ctx->hour_mask_tile->tRegion.tSize.iWidth >> 1),
                                (float)create_ctx->hour_mask_tile->tRegion.tSize.iHeight,
                                true,
                                true);
    ldClockBindMinutePointerImage(ld_clock,
                                  create_ctx->minute_img_tile,
                                  create_ctx->minute_mask_tile,
                                  0,
                                  (float)(create_ctx->minute_mask_tile->tRegion.tSize.iWidth >> 1),
                                  (float)create_ctx->minute_mask_tile->tRegion.tSize.iHeight,
                                  true,
                                  true);
    ldClockBindSecondPointerImage(ld_clock,
                                  create_ctx->second_img_tile,
                                  create_ctx->second_mask_tile,
                                  0,
                                  (float)(create_ctx->second_mask_tile->tRegion.tSize.iWidth >> 1),
                                  100.0f,
                                  true,
                                  true);
    ldClockBindBackgroundImage(ld_clock,
                               (arm_2d_tile_t *)&c_tileClockface,
                               NULL,
                               0,
                               false,
                               false);
    return ld_clock;
}

static int clock_props_valid(const tinyui_clock_props_t *props)
{
    return props != 0 && (props->step_second == 0 || props->step_second == 1);
}

static int tinyui_clock_apply_background(struct tinyui_clock *clock)
{
    ldClock_t *ld_clock;
    arm_2d_tile_t *img_tile;
    arm_2d_tile_t *mask_tile;

    if (clock == 0 || clock->widget.ld_widget == 0
        || clock->widget.kind != TINYUI_BACKEND_WIDGET_CLOCK) {
        return -1;
    }

    ld_clock = (ldClock_t *)clock->widget.ld_widget;
    img_tile = clock->background_source != NULL ? tinyui_image_source_get_image_tile(clock->background_source) : ld_clock->ptBgImgTile;
    mask_tile = clock->background_source != NULL ? tinyui_image_source_get_mask_tile(clock->background_source) : ld_clock->ptBgMaskTile;
    ldClockSetBackgroundImage(ld_clock, img_tile, mask_tile, (ldColor)clock->mask_color);
    return 0;
}

static int tinyui_clock_apply_pointer(struct tinyui_clock *clock, int index)
{
    ldClock_t *ld_clock;
    struct tinyui_image_source *source;
    arm_2d_tile_t *img_tile;
    arm_2d_tile_t *mask_tile;
    float x;
    float y;

    if (clock == 0 || clock->widget.ld_widget == 0
        || clock->widget.kind != TINYUI_BACKEND_WIDGET_CLOCK) {
        return -1;
    }

    ld_clock = (ldClock_t *)clock->widget.ld_widget;

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

    img_tile = source != NULL ? tinyui_image_source_get_image_tile(source) : ld_clock->pointerInfo[index].ptImgTile;
    mask_tile = source != NULL ? tinyui_image_source_get_mask_tile(source) : ld_clock->pointerInfo[index].ptMaskTile;
    if (source == NULL) {
        ld_clock->pointerInfo[index].maskColor = (ldColor)clock->mask_color;
        ld_clock->pointerInfo[index].rotationCentre = (arm_2d_point_float_t){x, y};
        ld_clock->use_as__ldBase_t.isDirtyRegionUpdate = true;
        ld_clock->use_as__ldBase_t.ptItemRegionList[index].isDRUpdate = true;
        return 0;
    }
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

tinyui_obj_t *tinyui_clock_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "clock";
    if (parent_w == 0) { return 0; }

    struct tinyui_clock *clock;
    arm_2d_tile_t *hour_img_tile;
    arm_2d_tile_t *hour_mask_tile;
    arm_2d_tile_t *minute_img_tile;
    arm_2d_tile_t *minute_mask_tile;
    arm_2d_tile_t *second_img_tile;
    arm_2d_tile_t *second_mask_tile;
    struct tinyui_clock_create_ctx ctx;

    if (parent_w == 0 || id == 0 || parent_w->ld_widget == 0) {
        return 0;
    }

    hour_img_tile = ldMalloc(sizeof(*hour_img_tile));
    if (hour_img_tile == 0) {
        return 0;
    }
    *hour_img_tile = c_tilePointerSecGRAY8;
    hour_img_tile->tRegion.tSize.iHeight = 67;

    hour_mask_tile = ldMalloc(sizeof(*hour_mask_tile));
    if (hour_mask_tile == 0) {
        ldFree(hour_img_tile);
        return 0;
    }
    *hour_mask_tile = c_tilePointerSecMask;
    hour_mask_tile->tRegion.tSize.iHeight = 67;

    minute_img_tile = ldMalloc(sizeof(*minute_img_tile));
    if (minute_img_tile == 0) {
        ldFree(hour_mask_tile);
        ldFree(hour_img_tile);
        return 0;
    }
    *minute_img_tile = c_tilePointerSecGRAY8;

    minute_mask_tile = ldMalloc(sizeof(*minute_mask_tile));
    if (minute_mask_tile == 0) {
        ldFree(minute_img_tile);
        ldFree(hour_mask_tile);
        ldFree(hour_img_tile);
        return 0;
    }
    *minute_mask_tile = c_tilePointerSecMask;

    second_img_tile = ldMalloc(sizeof(*second_img_tile));
    if (second_img_tile == 0) {
        ldFree(minute_mask_tile);
        ldFree(minute_img_tile);
        ldFree(hour_mask_tile);
        ldFree(hour_img_tile);
        return 0;
    }
    *second_img_tile = c_tilePointerSecGRAY8;

    second_mask_tile = ldMalloc(sizeof(*second_mask_tile));
    if (second_mask_tile == 0) {
        ldFree(second_img_tile);
        ldFree(minute_mask_tile);
        ldFree(minute_img_tile);
        ldFree(hour_mask_tile);
        ldFree(hour_img_tile);
        return 0;
    }
    *second_mask_tile = c_tilePointerSecMask;

    ctx.hour_img_tile = hour_img_tile;
    ctx.hour_mask_tile = hour_mask_tile;
    ctx.minute_img_tile = minute_img_tile;
    ctx.minute_mask_tile = minute_mask_tile;
    ctx.second_img_tile = second_img_tile;
    ctx.second_mask_tile = second_mask_tile;
    clock = (struct tinyui_clock *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                             TINYUI_BACKEND_WIDGET_CLOCK,
                                                             tinyui_runtime_internal_clock_ld_init,
                                                             &ctx,
                                                             sizeof(*clock));
    if (clock == 0) {
        ldFree(second_mask_tile);
        ldFree(second_img_tile);
        ldFree(minute_mask_tile);
        ldFree(minute_img_tile);
        ldFree(hour_mask_tile);
        ldFree(hour_img_tile);
        return 0;
    }

    clock->mask_color = 0;
    clock->hour_anchor_x = 0.0f;
    clock->hour_anchor_y = 67.0f;
    clock->minute_anchor_x = 0.0f;
    clock->minute_anchor_y = 100.0f;
    clock->second_anchor_x = 0.0f;
    clock->second_anchor_y = 100.0f;
    clock->use_system_time = 1;
    clock->id = id;
    clock->widget.value = 0;
    if (tinyui_clock_set_step_second(clock, 0) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    return (tinyui_obj_t *)clock;
}

/**
 * @brief clock init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_clock *tinyui_runtime_internal_clock_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_clock_create(parent);
}

/**
 * @brief Set use system time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_use_system_time(tinyui_obj_t *clock_obj, int enabled)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    if (clock == 0 || clock->widget.ld_widget == 0
        || clock->widget.kind != TINYUI_BACKEND_WIDGET_CLOCK) {
        return -1;
    }

    ldClockSetAutoSysTime((ldClock_t *)clock->widget.ld_widget, enabled != 0);
    clock->use_system_time = enabled != 0;
    return 0;
}

int tinyui_clock_set_auto_sys_time(tinyui_obj_t *clock_obj, int enabled)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    return tinyui_clock_set_use_system_time((tinyui_obj_t *)clock, enabled);
}

/**
 * @brief Get use system time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @return -1 on failure
 */

int tinyui_clock_get_use_system_time(const tinyui_obj_t *clock_obj)
{
    const struct tinyui_clock *clock = tinyui_clock_as_clock_const(clock_obj);
    if (clock == 0) { return -1; }

    ldClock_t *ld_clock;

    if (clock == 0 || clock->widget.ld_widget == 0
        || clock->widget.kind != TINYUI_BACKEND_WIDGET_CLOCK) {
        return -1;
    }

    ld_clock = (ldClock_t *)clock->widget.ld_widget;
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

tinyui_obj_t *tinyui_clock_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_clock_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_clock *clock;

    if (props == 0) {
        return tinyui_clock_create(parent);
    }

    obj = tinyui_clock_create(parent);
    if (obj == 0) {
        return 0;
    }
    clock = (struct tinyui_clock *)(void *)obj;

    if ((props->fields & TINYUI_CLOCK_FIELD_ID) != 0) {
        (void)props->id;
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&clock->widget, props->user_data) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&clock->widget, props->style_class) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_BACKGROUND_SOURCE) != 0) {
    if (tinyui_clock_set_background_source((tinyui_obj_t *)clock, props->background_source) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_HOUR_POINTER_SOURCE) != 0) {
    if (tinyui_clock_set_hour_pointer_source((tinyui_obj_t *)clock, props->hour_pointer_source) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_MINUTE_POINTER_SOURCE) != 0) {
    if (tinyui_clock_set_minute_pointer_source((tinyui_obj_t *)clock, props->minute_pointer_source) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_SECOND_POINTER_SOURCE) != 0) {
    if (tinyui_clock_set_second_pointer_source((tinyui_obj_t *)clock, props->second_pointer_source) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_MASK_COLOR) != 0) {
    if (tinyui_clock_set_mask_color((tinyui_obj_t *)clock, props->mask_color) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_HOUR_ANCHOR_X) != 0 ||
        (props->fields & TINYUI_CLOCK_FIELD_HOUR_ANCHOR_Y) != 0) {
        float ax = ((props->fields & TINYUI_CLOCK_FIELD_HOUR_ANCHOR_X) != 0) ? props->hour_anchor_x : clock->hour_anchor_x;
        float ay = ((props->fields & TINYUI_CLOCK_FIELD_HOUR_ANCHOR_Y) != 0) ? props->hour_anchor_y : clock->hour_anchor_y;
            if (tinyui_clock_set_hour_anchor((tinyui_obj_t *)clock, ax, ay) != 0) {
                tinyui_clock_rollback(clock);
                return 0;
            }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_MINUTE_ANCHOR_X) != 0 ||
        (props->fields & TINYUI_CLOCK_FIELD_MINUTE_ANCHOR_Y) != 0) {
        float ax = ((props->fields & TINYUI_CLOCK_FIELD_MINUTE_ANCHOR_X) != 0) ? props->minute_anchor_x : clock->minute_anchor_x;
        float ay = ((props->fields & TINYUI_CLOCK_FIELD_MINUTE_ANCHOR_Y) != 0) ? props->minute_anchor_y : clock->minute_anchor_y;
            if (tinyui_clock_set_minute_anchor((tinyui_obj_t *)clock, ax, ay) != 0) {
                tinyui_clock_rollback(clock);
                return 0;
            }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_SECOND_ANCHOR_X) != 0 ||
        (props->fields & TINYUI_CLOCK_FIELD_SECOND_ANCHOR_Y) != 0) {
        float ax = ((props->fields & TINYUI_CLOCK_FIELD_SECOND_ANCHOR_X) != 0) ? props->second_anchor_x : clock->second_anchor_x;
        float ay = ((props->fields & TINYUI_CLOCK_FIELD_SECOND_ANCHOR_Y) != 0) ? props->second_anchor_y : clock->second_anchor_y;
            if (tinyui_clock_set_second_anchor((tinyui_obj_t *)clock, ax, ay) != 0) {
                tinyui_clock_rollback(clock);
                return 0;
            }
    }
    if ((props->fields & TINYUI_CLOCK_FIELD_STEP_SECOND) != 0) {
    if (tinyui_clock_set_step_second((tinyui_obj_t *)clock, props->step_second) != 0) {
        tinyui_clock_rollback(clock);
        return 0;
    }
    }

    return obj;
}



/**
 * @brief Set step second of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] step_second step second
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_step_second(tinyui_obj_t *clock_obj, int step_second)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    if (clock == 0 || (step_second != 0 && step_second != 1)
        || clock->widget.ld_widget == 0
        || clock->widget.kind != TINYUI_BACKEND_WIDGET_CLOCK) {
        return -1;
    }

    ldClockSetStepSecond((ldClock_t *)clock->widget.ld_widget, step_second != 0);
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

int tinyui_clock_get_step_second(const tinyui_obj_t *clock_obj)
{
    const struct tinyui_clock *clock = tinyui_clock_as_clock_const(clock_obj);
    if (clock == 0) { return -1; }

    ldClock_t *ld_clock;

    if (clock == 0 || clock->widget.ld_widget == 0
        || clock->widget.kind != TINYUI_BACKEND_WIDGET_CLOCK) {
        return -1;
    }

    ld_clock = (ldClock_t *)clock->widget.ld_widget;
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

int tinyui_clock_set_background_source(tinyui_obj_t *clock_obj, struct tinyui_image_source *source)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    if (clock == 0 || source == 0 || tinyui_image_source_get_image_tile(source) == 0) {
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

int tinyui_clock_set_background_image(tinyui_obj_t *clock_obj, struct tinyui_image_source *source)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    return tinyui_clock_set_background_source((tinyui_obj_t *)clock, source);
}

/**
 * @brief Set hour pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_hour_pointer_source(tinyui_obj_t *clock_obj, struct tinyui_image_source *source)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    if (clock == 0 || source == 0 || tinyui_image_source_get_image_tile(source) == 0) {
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

int tinyui_clock_set_hour_pointer_image(tinyui_obj_t *clock_obj, struct tinyui_image_source *source)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    return tinyui_clock_set_hour_pointer_source((tinyui_obj_t *)clock, source);
}

/**
 * @brief Set minute pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_minute_pointer_source(tinyui_obj_t *clock_obj, struct tinyui_image_source *source)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    if (clock == 0 || source == 0 || tinyui_image_source_get_image_tile(source) == 0) {
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

int tinyui_clock_set_minute_pointer_image(tinyui_obj_t *clock_obj, struct tinyui_image_source *source)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    return tinyui_clock_set_minute_pointer_source((tinyui_obj_t *)clock, source);
}

/**
 * @brief Set second pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_second_pointer_source(tinyui_obj_t *clock_obj, struct tinyui_image_source *source)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    if (clock == 0 || source == 0 || tinyui_image_source_get_image_tile(source) == 0) {
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

int tinyui_clock_set_second_pointer_image(tinyui_obj_t *clock_obj, struct tinyui_image_source *source)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    return tinyui_clock_set_second_pointer_source((tinyui_obj_t *)clock, source);
}

/**
 * @brief Set mask color of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] mask_color mask color
 * @return 0 on success, -1 on failure
 */

int tinyui_clock_set_mask_color(tinyui_obj_t *clock_obj, unsigned int mask_color)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

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

int tinyui_clock_set_hour_anchor(tinyui_obj_t *clock_obj, float x, float y)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

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

int tinyui_clock_set_minute_anchor(tinyui_obj_t *clock_obj, float x, float y)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

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

int tinyui_clock_set_second_anchor(tinyui_obj_t *clock_obj, float x, float y)
{
    struct tinyui_clock *clock = tinyui_clock_as_clock(clock_obj);
    if (clock == 0) { return -1; }

    if (clock == 0) {
        return -1;
    }

    clock->second_anchor_x = x;
    clock->second_anchor_y = y;
    return tinyui_clock_apply_pointer(clock, 2);
}
