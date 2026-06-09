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
#include "ldGauge.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

static ldGauge_t *picoui_backend_gauge_get_ld(struct picoui_gauge *gauge)
{
    struct picoui_backend_widget *backend;

    if (gauge == NULL || gauge->widget.backend_widget == NULL) {
        return NULL;
    }
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_GAUGE || backend->ld_widget == NULL) {
        return NULL;
    }
    return (ldGauge_t *)backend->ld_widget;
}

/**
 * @brief Create backend for gauge
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_gauge(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldGauge_t *ld_gauge;
    arm_2d_tile_t *bg_img_tile;
    arm_2d_tile_t *bg_mask_tile;
    arm_2d_tile_t *pointer_img_tile;
    arm_2d_tile_t *pointer_mask_tile;
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

    bg_img_tile = malloc(sizeof(*bg_img_tile));
    if (bg_img_tile == NULL) {
        free(widget);
        return 0;
    }
    *bg_img_tile = c_tileQuaterArcGRAY8;

    bg_mask_tile = malloc(sizeof(*bg_mask_tile));
    if (bg_mask_tile == NULL) {
        free(bg_img_tile);
        free(widget);
        return 0;
    }
    *bg_mask_tile = c_tileQuaterArcMask;

    pointer_img_tile = malloc(sizeof(*pointer_img_tile));
    if (pointer_img_tile == NULL) {
        free(bg_mask_tile);
        free(bg_img_tile);
        free(widget);
        return 0;
    }
    *pointer_img_tile = c_tilePointerSecGRAY8;

    pointer_mask_tile = malloc(sizeof(*pointer_mask_tile));
    if (pointer_mask_tile == NULL) {
        free(pointer_img_tile);
        free(bg_mask_tile);
        free(bg_img_tile);
        free(widget);
        return 0;
    }
    *pointer_mask_tile = c_tilePointerSecMask;

    name_id = picoui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(pointer_mask_tile);
        free(pointer_img_tile);
        free(bg_mask_tile);
        free(bg_img_tile);
        free(widget);
        return 0;
    }
    ld_gauge = ldGauge_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_widget->ld_name_id,
                            0,
                            0,
                            160,
                            160,
                            bg_img_tile,
                            bg_mask_tile,
                            0,
                            0);
    if (ld_gauge == NULL) {
        free(pointer_mask_tile);
        free(pointer_img_tile);
        free(bg_img_tile);
        free(bg_mask_tile);
        free(widget);
        return 0;
    }
    ldGaugeSetBackgroundImage(ld_gauge, bg_img_tile, bg_mask_tile, true, true);

    ldGaugeBindPointerImage(ld_gauge,
                            pointer_img_tile,
                            pointer_mask_tile,
                            (int16_t)(pointer_mask_tile->tRegion.tSize.iWidth >> 1),
                            (int16_t)(pointer_mask_tile->tRegion.tSize.iHeight),
                            true,
                            true);

    if (picoui_backend_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_GAUGE,
                                         id,
                                         parent_widget->theme) != 0) {
        ldGauge_depose(app_state->ld_scene, ld_gauge);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_gauge;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldGauge_depose(app_state->ld_scene, ld_gauge);
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set angle of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] angle Angle in degrees
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_angle(struct picoui_gauge *gauge, float angle)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);
    struct picoui_backend_widget *backend;

    if (ld_gauge == NULL) {
        return -1;
    }
    ldGaugeSetAngle(ld_gauge, angle);
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    backend->value = (int)angle;
    return 0;
}

/**
 * @brief Set bg source of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_bg_source(struct picoui_gauge *gauge, struct picoui_image_source *source)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL || source == NULL || source->img_tile == NULL || source->mask_tile == NULL) {
        return -1;
    }

    ldGaugeSetBackgroundImage(ld_gauge, source->img_tile, source->mask_tile, false, false);
    return 0;
}

/**
 * @brief Set pointer source of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_pointer_source(struct picoui_gauge *gauge, struct picoui_image_source *source)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);
    arm_2d_tile_t *mask_tile;

    if (ld_gauge == NULL || source == NULL || source->img_tile == NULL || source->mask_tile == NULL) {
        return -1;
    }

    mask_tile = (arm_2d_tile_t *)source->mask_tile;

    ldGaugeBindPointerImage(ld_gauge,
                            source->img_tile,
                            source->mask_tile,
                            (int16_t)(mask_tile->tRegion.tSize.iWidth >> 1),
                            (int16_t)(mask_tile->tRegion.tSize.iHeight),
                            false,
                            false);
    return 0;
}

/**
 * @brief Set centre offset of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] centre_offset_x centre offset x
 * @param[in] centre_offset_y centre offset y
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_centre_offset(struct picoui_gauge *gauge,
                                           int centre_offset_x,
                                           int centre_offset_y)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL) {
        return -1;
    }

    ld_gauge->centreOffsetX = (int16_t)centre_offset_x;
    ld_gauge->centreOffsetY = (int16_t)centre_offset_y;
    return 0;
}

/**
 * @brief Set trail of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] bg_trail_source bg trail source
 * @param[in] pointer_trail_source pointer trail source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_trail(struct picoui_gauge *gauge,
                                   struct picoui_image_source *bg_trail_source,
                                   struct picoui_image_source *pointer_trail_source)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL
        || bg_trail_source == NULL
        || pointer_trail_source == NULL
        || bg_trail_source->mask_tile == NULL
        || pointer_trail_source->mask_tile == NULL) {
        return -1;
    }

    ldGaugeSetTrail(ld_gauge,
                    bg_trail_source->mask_tile,
                    pointer_trail_source->mask_tile);
    return 0;
}

/**
 * @brief Set progress bar of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] bg_progress_source bg progress source
 * @param[in] pointer_progress_source pointer progress source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_progress_bar(struct picoui_gauge *gauge,
                                          struct picoui_image_source *bg_progress_source,
                                          struct picoui_image_source *pointer_progress_source)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL
        || bg_progress_source == NULL
        || pointer_progress_source == NULL
        || bg_progress_source->mask_tile == NULL
        || pointer_progress_source->mask_tile == NULL) {
        return -1;
    }

    ldGaugeSetProgressBar(ld_gauge,
                          bg_progress_source->mask_tile,
                          pointer_progress_source->mask_tile);
    return 0;
}

/**
 * @brief Get angle from gauge backend
 *
 * @param[out] gauge Gauge widget instance
 * @param[in] angle Angle in degrees
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_get_angle(struct picoui_gauge *gauge, float *angle)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL || angle == NULL) {
        return -1;
    }
    *angle = (float)ld_gauge->_nowAngle_x10 / 10.0f;
    return 0;
}

/**
 * @brief Set pointer color of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] pointer_color pointer color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL) {
        return -1;
    }
    ldGaugeSetPointerColor(ld_gauge, (ldColor)pointer_color);
    return 0;
}

/**
 * @brief Get pointer color from gauge backend
 *
 * @param[out] gauge Gauge widget instance
 * @param[in] pointer_color pointer color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_get_pointer_color(struct picoui_gauge *gauge, unsigned int *pointer_color)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL || pointer_color == NULL) {
        return -1;
    }
    *pointer_color = (unsigned int)ld_gauge->maskColor;
    return 0;
}

/**
 * @brief Set auto move of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] auto_move auto move
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL) {
        return -1;
    }
    ldGaugeSetAutoMove(ld_gauge, auto_move != 0);
    return 0;
}

/**
 * @brief Get auto move from gauge backend
 *
 * @param[out] gauge Gauge widget instance
 * @param[in] auto_move auto move
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_get_auto_move(struct picoui_gauge *gauge, int *auto_move)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL || auto_move == NULL) {
        return -1;
    }
    *auto_move = ld_gauge->isAutoMove ? 1 : 0;
    return 0;
}
