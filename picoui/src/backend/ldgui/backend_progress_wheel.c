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
#include "runtime_bridge.h"
#include "ldProgressWheel.h"

#include <stdlib.h>

static ldColor picoui_backend_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static ldProgressWheel_t *picoui_backend_progress_wheel_get_ld(struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend;

    if (wheel == NULL || wheel->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldProgressWheel_t *)backend->ld_widget;
}

/**
 * @brief Create backend for progress wheel
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_progress_wheel(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldProgressWheel_t *ld_progress_wheel;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_runtime_bridge_backend_state_from_parent(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(widget);
        return 0;
    }
    ld_progress_wheel = ldProgressWheel_init(app_state->ld_scene,
                                             NULL,
                                             name_id,
                                             parent_widget->ld_name_id,
                                             0,
                                             0,
                                             96,
                                             96);
    if (ld_progress_wheel == NULL) {
        free(widget);
        return 0;
    }

    /* Host PicoUI scenes do not initialize the Arm-2D transform dirty-region helper path. */
    ld_progress_wheel->tWheel.tCFG.bUseDirtyRegions = false;
    ldProgressWheelSetWheelColor(ld_progress_wheel, __RGB(32, 87, 196));
    ldProgressWheelSetDotColor(ld_progress_wheel, GLCD_COLOR_WHITE, true);

    if (picoui_backend_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL,
                                         id,
                                         parent_widget->theme) != 0) {
        ldProgressWheel_depose(app_state->ld_scene, ld_progress_wheel);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_progress_wheel;
    widget->ld_name_id = name_id;
    widget->value = 0;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldProgressWheel_depose(app_state->ld_scene, ld_progress_wheel);
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief progress: wheel set percent
 *
 * @param[in] wheel wheel
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent)
{
    ldProgressWheel_t *ld_progress_wheel = picoui_backend_progress_wheel_get_ld(wheel);
    struct picoui_backend_widget *backend;

    if (ld_progress_wheel == NULL || percent < 0 || percent > 100) {
        return -1;
    }

    ldProgressWheelSetProgress(ld_progress_wheel, (int16_t)(percent * 10));
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    backend->value = percent;
    return 0;
}

/**
 * @brief progress: wheel get percent
 *
 * @param[out] wheel wheel
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_wheel_get_percent(struct picoui_progress_wheel *wheel, int *percent)
{
    ldProgressWheel_t *ld_progress_wheel = picoui_backend_progress_wheel_get_ld(wheel);

    if (ld_progress_wheel == NULL || percent == NULL) {
        return -1;
    }

    *percent = ld_progress_wheel->iProgress / 10;
    return 0;
}

/**
 * @brief progress: wheel set wheel color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_wheel_set_wheel_color(void *backend_widget, unsigned int rgb)
{
    ldProgressWheel_t *ld_progress_wheel;

    if (backend_widget == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_progress_wheel = picoui_backend_progress_wheel_get_ld((struct picoui_progress_wheel *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_wheel == NULL) {
        return -1;
    }

    ldProgressWheelSetWheelColor(ld_progress_wheel, picoui_backend_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief progress: wheel set dot color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_wheel_set_dot_color(void *backend_widget, unsigned int rgb)
{
    ldProgressWheel_t *ld_progress_wheel;

    if (backend_widget == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_progress_wheel = picoui_backend_progress_wheel_get_ld((struct picoui_progress_wheel *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_wheel == NULL) {
        return -1;
    }

    ldProgressWheelSetDotColor(ld_progress_wheel,
                               picoui_backend_rgb_to_ld_color(rgb),
                               !ld_progress_wheel->tWheel.tCFG.bIgnoreDot);
    return 0;
}

/**
 * @brief progress: wheel set dot enabled
 *
 * @param[in] backend_widget backend widget
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_wheel_set_dot_enabled(void *backend_widget, int enabled)
{
    ldProgressWheel_t *ld_progress_wheel;

    if (backend_widget == NULL) {
        return -1;
    }

    ld_progress_wheel = picoui_backend_progress_wheel_get_ld((struct picoui_progress_wheel *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_wheel == NULL) {
        return -1;
    }

    ldProgressWheelSetDotColor(ld_progress_wheel,
                               ld_progress_wheel->tWheel.tCFG.tDotColour,
                               enabled != 0);
    return 0;
}

/**
 * @brief progress: wheel get dot enabled
 *
 * @param[in] backend_widget backend widget
 * @return -1 on failure
 */

int picoui_backend_progress_wheel_get_dot_enabled(void *backend_widget)
{
    ldProgressWheel_t *ld_progress_wheel;

    if (backend_widget == NULL) {
        return -1;
    }

    ld_progress_wheel = picoui_backend_progress_wheel_get_ld((struct picoui_progress_wheel *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_wheel == NULL) {
        return -1;
    }

    return ld_progress_wheel->tWheel.tCFG.bIgnoreDot ? 0 : 1;
}
