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
#include "ldArc.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;

static ldArc_t *picoui_backend_arc_get_ld(struct picoui_arc *arc)
{
    struct picoui_backend_widget *backend;

    if (arc == NULL || arc->widget.backend_widget == NULL) {
        return NULL;
    }
    backend = (struct picoui_backend_widget *)arc->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_ARC || backend->ld_widget == NULL) {
        return NULL;
    }
    return (ldArc_t *)backend->ld_widget;
}

/**
 * @brief Create backend for arc
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_arc(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldArc_t *ld_arc;
    arm_2d_tile_t *arc_img_tile;
    arm_2d_tile_t *arc_mask_tile;
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

    arc_img_tile = malloc(sizeof(*arc_img_tile));
    if (arc_img_tile == NULL) {
        free(widget);
        return 0;
    }
    *arc_img_tile = c_tileQuaterArcGRAY8;

    arc_mask_tile = malloc(sizeof(*arc_mask_tile));
    if (arc_mask_tile == NULL) {
        free(arc_img_tile);
        free(widget);
        return 0;
    }
    *arc_mask_tile = c_tileQuaterArcMask;

    name_id = picoui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(arc_mask_tile);
        free(arc_img_tile);
        free(widget);
        return 0;
    }
    ld_arc = ldArc_init(app_state->ld_scene,
                        NULL,
                        name_id,
                        parent_widget->ld_name_id,
                        0,
                        0,
                        160,
                        160,
                        arc_img_tile,
                        arc_mask_tile,
                        GLCD_COLOR_WHITE);
    if (ld_arc == NULL) {
        free(arc_img_tile);
        free(arc_mask_tile);
        free(widget);
        return 0;
    }
    ldArcSetQuarterImage(ld_arc, arc_img_tile, arc_mask_tile, true, true);

    if (picoui_backend_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_ARC,
                                         id,
                                         parent_widget->theme) != 0) {
        ldArc_depose(app_state->ld_scene, ld_arc);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_arc;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldArc_depose(app_state->ld_scene, ld_arc);
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set background angle of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] bg_start_angle Background arc start angle
 * @param[in] bg_end_angle Background arc end angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_background_angle(struct picoui_arc *arc, float bg_start_angle, float bg_end_angle)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL || bg_end_angle < bg_start_angle) {
        return -1;
    }
    ldArcSetBackgroundAngle(ld_arc, bg_start_angle, bg_end_angle);
    return 0;
}

/**
 * @brief Set foreground angle of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] fg_end_angle Foreground arc end angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_foreground_angle(struct picoui_arc *arc, float fg_end_angle)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL) {
        return -1;
    }
    ldArcSetForegroundAngle(ld_arc, fg_end_angle);
    return 0;
}

/**
 * @brief Set rotation angle of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] rotation_angle Arc rotation angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_rotation_angle(struct picoui_arc *arc, float rotation_angle)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL) {
        return -1;
    }
    ldArcSetRotationAngle(ld_arc, rotation_angle);
    return 0;
}

/**
 * @brief Set color of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] bg_color Background color
 * @param[in] fg_color Foreground color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_color(struct picoui_arc *arc, unsigned int bg_color, unsigned int fg_color)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL) {
        return -1;
    }
    ldArcSetColor(ld_arc, (ldColor)bg_color, (ldColor)fg_color);
    return 0;
}

/**
 * @brief Set quarter source of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_quarter_source(struct picoui_arc *arc, struct picoui_image_source *source)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL || source == NULL || source->img_tile == NULL || source->mask_tile == NULL) {
        return -1;
    }

    ldArcSetQuarterImage(ld_arc, source->img_tile, source->mask_tile, false, false);
    return 0;
}

/**
 * @brief Set parent color of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] parent_color parent color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_parent_color(struct picoui_arc *arc, unsigned int parent_color)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL || parent_color > 0xFFFFFFU) {
        return -1;
    }

    ld_arc->parentColor = (ldColor)parent_color;
    return 0;
}

/**
 * @brief Get background angle from arc backend
 *
 * @param[out] arc Arc widget instance
 * @param[in] bg_start_angle Background arc start angle
 * @param[in] bg_angle bg angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_get_background_angle(struct picoui_arc *arc, float *bg_start_angle, float *bg_angle)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL || bg_start_angle == NULL || bg_angle == NULL) {
        return -1;
    }
    *bg_start_angle = ldArcGetBackgroundStartAngle(ld_arc);
    *bg_angle = ldArcGetBackgroundAngle(ld_arc);
    return 0;
}

/**
 * @brief Get foreground angle from arc backend
 *
 * @param[out] arc Arc widget instance
 * @param[in] fg_end_angle Foreground arc end angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_get_foreground_angle(struct picoui_arc *arc, float *fg_end_angle)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL || fg_end_angle == NULL) {
        return -1;
    }
    *fg_end_angle = ldArcGetForegroundAngle(ld_arc);
    return 0;
}

/**
 * @brief Get rotation angle from arc backend
 *
 * @param[out] arc Arc widget instance
 * @param[in] rotation_angle Arc rotation angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_get_rotation_angle(struct picoui_arc *arc, float *rotation_angle)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL || rotation_angle == NULL) {
        return -1;
    }
    *rotation_angle = ldArcGetRotationAngle(ld_arc);
    return 0;
}

/**
 * @brief Get color from arc backend
 *
 * @param[out] arc Arc widget instance
 * @param[in] bg_color Background color
 * @param[in] fg_color Foreground color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_get_color(struct picoui_arc *arc, unsigned int *bg_color, unsigned int *fg_color)
{
    ldArc_t *ld_arc = picoui_backend_arc_get_ld(arc);

    if (ld_arc == NULL || bg_color == NULL || fg_color == NULL) {
        return -1;
    }
    *bg_color = (unsigned int)ldArcGetBackgroundColor(ld_arc);
    *fg_color = (unsigned int)ldArcGetForegroundColor(ld_arc);
    return 0;
}
