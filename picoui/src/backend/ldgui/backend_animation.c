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
#include "ldAnimation.h"

#include <stdlib.h>

static ldAnimation_t *picoui_backend_animation_get_ld(struct picoui_animation *animation)
{
    struct picoui_backend_widget *backend;

    if (animation == NULL || animation->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)animation->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_ANIMATION || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldAnimation_t *)backend->ld_widget;
}

/**
 * @brief Create backend for animation
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] source Image source
 * @param[in] period_ms Period in milliseconds
 */

void *picoui_backend_create_animation(void *parent,
                                      const char *id,
                                      int width,
                                      int height,
                                      struct picoui_image_source *source,
                                      int period_ms)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldAnimation_t *ld_animation;
    uint16_t name_id;

    if (parent == 0 || id == 0 || width <= 0 || height <= 0 || period_ms <= 0 ||
        source == NULL || source->img_tile == NULL) {
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
    ld_animation = ldAnimation_init(app_state->ld_scene,
                                    NULL,
                                    name_id,
                                    parent_widget->ld_name_id,
                                    0,
                                    0,
                                    (int16_t)width,
                                    (int16_t)height,
                                    source->img_tile,
                                    (uint16_t)period_ms);
    if (ld_animation == NULL) {
        free(widget);
        return 0;
    }

    if (picoui_backend_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_ANIMATION,
                                         id,
                                         parent_widget->theme) != 0) {
        ldAnimation_depose(app_state->ld_scene, ld_animation);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_animation;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldAnimation_depose(app_state->ld_scene, ld_animation);
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set source of animation backend
 *
 * @param[in] animation Animation widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_animation_set_source(struct picoui_animation *animation,
                                        struct picoui_image_source *source)
{
    ldAnimation_t *ld_animation = picoui_backend_animation_get_ld(animation);

    if (ld_animation == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    ld_animation->ptImgTile = source->img_tile;
    return 0;
}

/**
 * @brief Set period ms of animation backend
 *
 * @param[in] animation Animation widget instance
 * @param[in] period_ms Period in milliseconds
 * @return 0 on success, -1 on failure
 */

int picoui_backend_animation_set_period_ms(struct picoui_animation *animation, int period_ms)
{
    ldAnimation_t *ld_animation = picoui_backend_animation_get_ld(animation);

    if (ld_animation == NULL || period_ms <= 0 || period_ms > 0xFFFF) {
        return -1;
    }

    ld_animation->periodMs = (uint16_t)period_ms;
    return 0;
}

/**
 * @brief animation: show frame
 *
 * @param[in] animation Animation widget instance
 * @param[in] frame_index Frame index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_animation_show_frame(struct picoui_animation *animation, int frame_index)
{
    ldAnimation_t *ld_animation = picoui_backend_animation_get_ld(animation);
    arm_2d_tile_t *img_tile;
    int frames_per_row;

    if (ld_animation == NULL || animation->source == NULL || animation->source->img_tile == NULL ||
        animation->width <= 0 || animation->height <= 0 || frame_index < 0) {
        return -1;
    }

    img_tile = (arm_2d_tile_t *)animation->source->img_tile;
    frames_per_row = img_tile->tRegion.tSize.iWidth / animation->width;
    if (frames_per_row <= 0) {
        return -1;
    }

    if (frame_index >= frames_per_row) {
        return -1;
    }

    ld_animation->showRegion.tLocation.iX = (int16_t)(frame_index * animation->width);
    ld_animation->showRegion.tLocation.iY = 0;
    ld_animation->showRegion.tSize.iWidth = (int16_t)animation->width;
    ld_animation->showRegion.tSize.iHeight = (int16_t)animation->height;
    return 0;
}
