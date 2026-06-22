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
#include "progress_wheel.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldProgressWheel.h"
#include "../core/runtime_bridge.h"

#include <stdlib.h>
#include <string.h>

int tinyui_runtime_bridge_unbind_host(void *backend_widget);

static int tinyui_progress_wheel_fail_next_set_percent = 0;

struct __attribute__((may_alias)) tinyui_progress_wheel_cfg_bridge {
    struct {
        const arm_2d_tile_t *ptileArcMask;
        const arm_2d_tile_t *ptileDotMask;
        int16_t iWheelDiameter;
        int16_t iRingWidth;
        COLOUR_INT tWheelColour;
        COLOUR_INT tDotColour;
        uint32_t bUseDirtyRegions : 1;
        uint32_t bIgnoreDot : 1;
        uint32_t u2StartPosition : 2;
    } tCFG;
};

static ldColor tinyui_progress_wheel_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

struct tinyui_progress_wheel_test_dispose_snapshot {
    int kind;
    int cleanup_complete;
    int cleanup_incomplete;
    int detach_result;
    int unbind_result;
    int detached;
    int owner_cleared;
    int root_cleared;
    int parent_cleared;
    int next_sibling_cleared;
    int host_cleared;
    int event_bridge_cleared;
    int ld_pinfo_cleared;
};

static struct tinyui_progress_wheel_test_dispose_snapshot
    tinyui_progress_wheel_last_dispose_snapshot;
static int tinyui_progress_wheel_last_dispose_snapshot_valid = 0;

static ldProgressWheel_t *tinyui_progress_wheel_get_ld(struct tinyui_progress_wheel *wheel)
{
    if (wheel == 0 || wheel->widget.ld_widget == 0
        || wheel->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL) {
        return 0;
    }

    return (ldProgressWheel_t *)wheel->widget.ld_widget;
}


static void tinyui_progress_wheel_test_reset_internal_state(void)
{
    memset(&tinyui_progress_wheel_last_dispose_snapshot,
           0,
           sizeof(tinyui_progress_wheel_last_dispose_snapshot));
    tinyui_progress_wheel_last_dispose_snapshot_valid = 0;
}

void tinyui_progress_wheel_test_reset_state(void)
{
    tinyui_progress_wheel_test_reset_internal_state();
}

int tinyui_progress_wheel_test_take_last_dispose_snapshot(
    struct tinyui_progress_wheel_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || tinyui_progress_wheel_last_dispose_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = tinyui_progress_wheel_last_dispose_snapshot;
    memset(&tinyui_progress_wheel_last_dispose_snapshot,
           0,
           sizeof(tinyui_progress_wheel_last_dispose_snapshot));
    tinyui_progress_wheel_last_dispose_snapshot_valid = 0;
    return 0;
}

static void tinyui_progress_wheel_capture_dispose_snapshot(
    struct tinyui_widget *widget,
    int detach_result,
    int unbind_result)
{
    ldBase_t *ld_base = widget != 0 ? (ldBase_t *)widget->ld_widget : 0;

    memset(&tinyui_progress_wheel_last_dispose_snapshot,
           0,
           sizeof(tinyui_progress_wheel_last_dispose_snapshot));
    tinyui_progress_wheel_last_dispose_snapshot.kind =
        widget != 0 ? (int)widget->kind : 0;
    tinyui_progress_wheel_last_dispose_snapshot.detach_result = detach_result;
    tinyui_progress_wheel_last_dispose_snapshot.unbind_result = unbind_result;
    tinyui_progress_wheel_last_dispose_snapshot.cleanup_complete =
        (detach_result == 0 && unbind_result == 0);
    tinyui_progress_wheel_last_dispose_snapshot.cleanup_incomplete =
        (detach_result != 0 || unbind_result != 0);
    tinyui_progress_wheel_last_dispose_snapshot.detached = (detach_result == 0);
    tinyui_progress_wheel_last_dispose_snapshot.owner_cleared =
        (widget == 0 || widget->owner == 0);
    tinyui_progress_wheel_last_dispose_snapshot.root_cleared = 1;
    tinyui_progress_wheel_last_dispose_snapshot.parent_cleared = 1;
    tinyui_progress_wheel_last_dispose_snapshot.next_sibling_cleared = 1;
    tinyui_progress_wheel_last_dispose_snapshot.host_cleared = 1;
    tinyui_progress_wheel_last_dispose_snapshot.event_bridge_cleared =
        (widget == 0
         || (widget->ld_event_bridge_scene == 0
             && widget->ld_event_bridge_sender == 0
             && widget->ld_event_bridge_next == 0));
    tinyui_progress_wheel_last_dispose_snapshot.ld_pinfo_cleared =
        (ld_base == 0 || ld_base->pInfo == 0);
    tinyui_progress_wheel_last_dispose_snapshot_valid = 1;
}

void tinyui_progress_wheel_test_fail_next_set_percent(void)
{
    tinyui_progress_wheel_fail_next_set_percent = 1;
}

static void tinyui_progress_wheel_disable_dirty_regions(ldProgressWheel_t *ld_progress_wheel)
{
    struct tinyui_progress_wheel_cfg_bridge *bridge;

    if (ld_progress_wheel == 0) {
        return;
    }

    bridge = (struct tinyui_progress_wheel_cfg_bridge *)&ld_progress_wheel->tWheel;
    bridge->tCFG.bUseDirtyRegions = false;
}

static void tinyui_progress_wheel_dispose_partial_impl(struct tinyui_progress_wheel *wheel)
{
    struct tinyui_app *app_state;
    int detach_result = 0;
    int unbind_result = 0;

    if (wheel == 0) {
        return;
    }

    if (wheel->widget.ld_widget != 0) {
        void *saved_ld_widget = wheel->widget.ld_widget;
        app_state = wheel->widget.owner != 0
            ? tinyui_runtime_bridge_backend_state(wheel->widget.owner)
            : 0;
        detach_result = tinyui_widget_detach_from_parent(&wheel->widget);
        unbind_result = tinyui_runtime_bridge_unbind_host(&wheel->widget);
        tinyui_progress_wheel_capture_dispose_snapshot(&wheel->widget,
                                                       detach_result,
                                                       unbind_result);
        if (app_state != 0 && app_state->ld_scene != 0) {
            ldProgressWheel_depose(app_state->ld_scene,
                                   (ldProgressWheel_t *)saved_ld_widget);
        }
    }

    free(wheel);
}

static int tinyui_progress_wheel_props_are_valid(const struct tinyui_progress_wheel_props *props)
{
    return props != 0 && props->id != 0 && props->percent >= 0 && props->percent <= 100;
}

/**
 * @brief Create progress wheel widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_progress_wheel *tinyui_progress_wheel_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_progress_wheel *wheel;
    struct tinyui_app *app_state;
    ldProgressWheel_t *ld_progress_wheel;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = parent->owner;
    if (parent->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    wheel = calloc(1, sizeof(*wheel));
    if (wheel == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;

    ld_progress_wheel = ldProgressWheel_init(app_state->ld_scene,
                                             NULL,
                                             name_id,
                                             parent->ld_name_id,
                                             0,
                                             0,
                                             96,
                                             96);
    if (ld_progress_wheel == 0) {
        free(wheel);
        return 0;
    }

    ldProgressWheelSetWheelColor(ld_progress_wheel, __RGB(32, 87, 196));
    ldProgressWheelSetDotColor(ld_progress_wheel, GLCD_COLOR_WHITE, true);

    wheel->id = id;
    wheel->percent = 0;
    wheel->wheel_color = 0x2057C4U;
    wheel->dot_color = 0xFFFFFFU;
    wheel->dot_enabled = 1;
    wheel->widget.ld_widget  = ld_progress_wheel;
    wheel->widget.ld_name_id = name_id;
    wheel->widget.kind       = TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL;
    wheel->widget.owner      = app_state;
    wheel->widget.value      = 0;
    wheel->widget.visible    = 1;
    wheel->widget.enabled    = 1;
    ((ldBase_t *)ld_progress_wheel)->pInfo = &wheel->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&wheel->widget, app_state);
    tinyui_progress_wheel_disable_dirty_regions(ld_progress_wheel);

    if (tinyui_progress_wheel_set_percent(wheel, 0) != 0
        || tinyui_progress_wheel_set_dot_enabled(wheel, 1) != 0) {
        tinyui_progress_wheel_dispose_partial_impl(wheel);
        return 0;
    }
    return wheel;
}

/**
 * @brief progress wheel init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_progress_wheel *tinyui_progress_wheel_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_progress_wheel_create(parent, id);
}

/**
 * @brief Create progress wheel widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_progress_wheel *tinyui_progress_wheel_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_progress_wheel_props *props)
{
    struct tinyui_progress_wheel *wheel;

    if (!tinyui_progress_wheel_props_are_valid(props)) {
        return 0;
    }

    wheel = tinyui_progress_wheel_create(parent, props->id);
    if (wheel == 0) {
        return 0;
    }

    if (props->style_class != 0
        && tinyui_widget_set_style_class(&wheel->widget, props->style_class) != 0) {
        tinyui_progress_wheel_dispose_partial_impl(wheel);
        return 0;
    }
    if (tinyui_widget_set_user_data(&wheel->widget, props->user_data) != 0
        || tinyui_progress_wheel_set_percent(wheel, props->percent) != 0) {
        tinyui_progress_wheel_dispose_partial_impl(wheel);
        return 0;
    }
    return wheel;
}

/**
 * @brief Set percent of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_wheel_set_percent(struct tinyui_progress_wheel *wheel, int percent)
{
    ldProgressWheel_t *ld_progress_wheel;

    if (wheel == 0 || percent < 0 || percent > 100) {
        return -1;
    }

    ld_progress_wheel = tinyui_progress_wheel_get_ld(wheel);
    if (ld_progress_wheel == 0) {
        return -1;
    }

    if (tinyui_progress_wheel_fail_next_set_percent != 0) {
        tinyui_progress_wheel_fail_next_set_percent = 0;
        return -1;
    }

    ldProgressWheelSetProgress(ld_progress_wheel, (int16_t)(percent * 10));
    wheel->widget.value = percent;
    wheel->percent = percent;
    return 0;
}

/**
 * @brief Set progress of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_wheel_set_progress(struct tinyui_progress_wheel *wheel, int percent)
{
    return tinyui_progress_wheel_set_percent(wheel, percent);
}

/**
 * @brief Get percent of progress wheel widget
 *
 * @param[in] wheel wheel
 * @return -1 on failure
 */

int tinyui_progress_wheel_get_percent(const struct tinyui_progress_wheel *wheel)
{
    if (wheel == 0) {
        return -1;
    }

    return wheel->percent;
}

/**
 * @brief Set wheel color of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_wheel_set_wheel_color(struct tinyui_progress_wheel *wheel, unsigned int rgb)
{
    ldProgressWheel_t *ld_progress_wheel;

    if (wheel == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_progress_wheel = tinyui_progress_wheel_get_ld(wheel);
    if (ld_progress_wheel == 0) {
        return -1;
    }

    ldProgressWheelSetWheelColor(ld_progress_wheel, tinyui_progress_wheel_rgb_to_ld_color(rgb));
    wheel->wheel_color = rgb;
    return 0;
}

/**
 * @brief Set dot color of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_wheel_set_dot_color(struct tinyui_progress_wheel *wheel, unsigned int rgb)
{
    ldProgressWheel_t *ld_progress_wheel;

    if (wheel == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_progress_wheel = tinyui_progress_wheel_get_ld(wheel);
    if (ld_progress_wheel == 0) {
        return -1;
    }

    ldProgressWheelSetDotColor(ld_progress_wheel,
                               tinyui_progress_wheel_rgb_to_ld_color(rgb),
                               wheel->dot_enabled != 0);
    wheel->dot_color = rgb;
    return 0;
}

/**
 * @brief Set dot enabled of progress wheel widget
 *
 * @param[in] wheel wheel
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_wheel_set_dot_enabled(struct tinyui_progress_wheel *wheel, int enabled)
{
    ldProgressWheel_t *ld_progress_wheel;

    if (wheel == 0) {
        return -1;
    }

    ld_progress_wheel = tinyui_progress_wheel_get_ld(wheel);
    if (ld_progress_wheel == 0) {
        return -1;
    }

    ldProgressWheelSetDotColor(ld_progress_wheel,
                               tinyui_progress_wheel_rgb_to_ld_color(wheel->dot_color),
                               enabled != 0);
    wheel->dot_enabled = enabled != 0 ? 1 : 0;
    return 0;
}

/**
 * @brief Get dot enabled of progress wheel widget
 *
 * @param[in] wheel wheel
 * @return -1 on failure
 */

int tinyui_progress_wheel_get_dot_enabled(const struct tinyui_progress_wheel *wheel)
{
    if (wheel == 0) {
        return -1;
    }

    return wheel->dot_enabled;
}
