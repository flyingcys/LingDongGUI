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
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static int tinyui_progress_wheel_fail_next_set_percent = 0;

struct tinyui_progress_wheel_cfg_bridge {
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

static struct tinyui_backend_widget *tinyui_progress_wheel_backend(struct tinyui_progress_wheel *wheel)
{
    struct tinyui_backend_widget *backend;

    if (wheel == 0 || wheel->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct tinyui_backend_widget *)wheel->widget.backend_widget;
    if (backend->kind != TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL || backend->ld_widget == 0) {
        return 0;
    }

    return backend;
}

static ldProgressWheel_t *tinyui_progress_wheel_get_ld(struct tinyui_progress_wheel *wheel)
{
    struct tinyui_backend_widget *backend = tinyui_progress_wheel_backend(wheel);

    if (backend == 0) {
        return 0;
    }

    return (ldProgressWheel_t *)backend->ld_widget;
}

static int tinyui_progress_wheel_finish_detach_after_backend_failure(
    struct tinyui_backend_widget *backend)
{
    struct tinyui_backend_widget *parent;
    struct tinyui_backend_widget *cursor;

    if (backend == 0 || backend->parent == 0) {
        return 0;
    }

    parent = backend->parent;
    if (parent->first_child == backend) {
        parent->first_child = backend->next_sibling;
    } else {
        cursor = parent->first_child;
        while (cursor != 0 && cursor->next_sibling != backend) {
            cursor = cursor->next_sibling;
        }
        if (cursor == 0) {
            return -1;
        }
        cursor->next_sibling = backend->next_sibling;
    }

    backend->parent = 0;
    backend->next_sibling = 0;
    backend->owner = 0;
    backend->root = 0;
    return 0;
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

void tinyui_progress_wheel_test_capture_dispose_snapshot(
    struct tinyui_backend_widget *backend,
    int detach_result,
    int unbind_result)
{
    ldBase_t *ld_base = (ldBase_t *)backend->ld_widget;

    memset(&tinyui_progress_wheel_last_dispose_snapshot,
           0,
           sizeof(tinyui_progress_wheel_last_dispose_snapshot));
    tinyui_progress_wheel_last_dispose_snapshot.kind = backend->kind;
    tinyui_progress_wheel_last_dispose_snapshot.detach_result = detach_result;
    tinyui_progress_wheel_last_dispose_snapshot.unbind_result = unbind_result;
    tinyui_progress_wheel_last_dispose_snapshot.cleanup_complete =
        (detach_result == 0 && unbind_result == 0);
    tinyui_progress_wheel_last_dispose_snapshot.cleanup_incomplete =
        (detach_result != 0 || unbind_result != 0);
    tinyui_progress_wheel_last_dispose_snapshot.detached =
        (detach_result == 0 && backend->parent == 0);
    tinyui_progress_wheel_last_dispose_snapshot.owner_cleared = (backend->owner == 0);
    tinyui_progress_wheel_last_dispose_snapshot.root_cleared = (backend->root == 0);
    tinyui_progress_wheel_last_dispose_snapshot.parent_cleared = (backend->parent == 0);
    tinyui_progress_wheel_last_dispose_snapshot.next_sibling_cleared = (backend->next_sibling == 0);
    tinyui_progress_wheel_last_dispose_snapshot.host_cleared = (backend->host_widget == 0);
    tinyui_progress_wheel_last_dispose_snapshot.event_bridge_cleared =
        (backend->ld_event_bridge_scene == 0
         && backend->ld_event_bridge_sender == 0
         && backend->ld_event_bridge_next == 0);
    tinyui_progress_wheel_last_dispose_snapshot.ld_pinfo_cleared =
        (ld_base == 0 || ld_base->pInfo == 0);
    tinyui_progress_wheel_last_dispose_snapshot_valid = 1;
}

static int tinyui_progress_wheel_test_finish_detach_after_backend_failure(
    struct tinyui_backend_widget *backend)
{
    return tinyui_progress_wheel_finish_detach_after_backend_failure(backend);
}

void tinyui_progress_wheel_test_fail_next_set_percent(void)
{
    tinyui_progress_wheel_fail_next_set_percent = 1;
}

static void tinyui_progress_wheel_disable_dirty_regions(void *backend_widget)
{
    struct tinyui_backend_widget *backend = backend_widget;
    ldProgressWheel_t *ld_progress_wheel;
    struct tinyui_progress_wheel_cfg_bridge *bridge;

    if (backend == 0 || backend->kind != TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL || backend->ld_widget == 0) {
        return;
    }

    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    bridge = (struct tinyui_progress_wheel_cfg_bridge *)&ld_progress_wheel->tWheel;
    bridge->tCFG.bUseDirtyRegions = false;
}

static void tinyui_progress_wheel_dispose_partial_impl(struct tinyui_progress_wheel *wheel)
{
    struct tinyui_backend_widget *backend;
    struct tinyui_app *app_state;
    ldBase_t *ld_base;
    int detach_result = 0;
    int unbind_result = 0;

    if (wheel == 0) {
        return;
    }

    backend = (struct tinyui_backend_widget *)wheel->widget.backend_widget;
    if (backend != 0) {
        app_state = tinyui_runtime_bridge_backend_state(backend->owner);
        ld_base = (ldBase_t *)backend->ld_widget;
        if (backend->parent != 0) {
            detach_result = tinyui_runtime_bridge_detach_from_parent(backend);
            if (detach_result != 0) {
                detach_result =
                    tinyui_progress_wheel_test_finish_detach_after_backend_failure(backend);
            }
        }
        unbind_result = tinyui_runtime_bridge_unbind_host(backend);
        (void)ld_base;
        tinyui_progress_wheel_test_capture_dispose_snapshot(backend,
                                                            detach_result,
                                                            unbind_result);
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldProgressWheel_depose(app_state->ld_scene, (ldProgressWheel_t *)backend->ld_widget);
        }
        free(backend);
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
    struct tinyui_backend_widget *backend;
    struct tinyui_backend_widget *parent_backend;
    struct tinyui_app *app_state;
    ldProgressWheel_t *ld_progress_wheel;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct tinyui_backend_widget *)parent->backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend == 0
        || parent_backend->ld_widget == 0
        || app_state == 0
        || app_state->ld_scene == 0) {
        return 0;
    }

    wheel = calloc(1, sizeof(*wheel));
    if (wheel == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(wheel);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(wheel);
        return 0;
    }

    ld_progress_wheel = ldProgressWheel_init(app_state->ld_scene,
                                             NULL,
                                             name_id,
                                             parent_backend->ld_name_id,
                                             0,
                                             0,
                                             96,
                                             96);
    if (ld_progress_wheel == 0) {
        free(backend);
        free(wheel);
        return 0;
    }

    ldProgressWheelSetWheelColor(ld_progress_wheel, __RGB(32, 87, 196));
    ldProgressWheelSetDotColor(ld_progress_wheel, GLCD_COLOR_WHITE, true);

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL,
                                         id,
                                         parent_backend->theme) != 0) {
        ldProgressWheel_depose(app_state->ld_scene, ld_progress_wheel);
        free(backend);
        free(wheel);
        return 0;
    }
    backend->ld_widget = ld_progress_wheel;
    backend->ld_name_id = name_id;
    backend->value = 0;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldProgressWheel_depose(app_state->ld_scene, ld_progress_wheel);
        free(backend);
        free(wheel);
        return 0;
    }

    wheel->id = id;
    wheel->percent = 0;
    wheel->wheel_color = 0x2057C4U;
    wheel->dot_color = 0xFFFFFFU;
    wheel->dot_enabled = 1;
    wheel->widget.backend_widget = backend;
    wheel->widget.visible = 1;
    wheel->widget.enabled = 1;
    tinyui_progress_wheel_disable_dirty_regions(backend);
    if (tinyui_runtime_bridge_bind_host(wheel->widget.backend_widget, &wheel->widget) != 0) {
        tinyui_progress_wheel_dispose_partial_impl(wheel);
        return 0;
    }
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
    struct tinyui_backend_widget *backend;

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
    backend = (struct tinyui_backend_widget *)wheel->widget.backend_widget;
    backend->value = percent;
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
