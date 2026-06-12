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
#include "picoui/animation.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldAnimation.h"

#include <stdlib.h>

static ldAnimation_t *tinyui_animation_get_ld(struct picoui_animation *animation)
{
    struct picoui_backend_widget *backend;

    if (animation == 0 || animation->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)animation->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_ANIMATION || backend->ld_widget == 0) {
        return 0;
    }

    return (ldAnimation_t *)backend->ld_widget;
}

static int tinyui_animation_props_are_valid(const struct picoui_animation_props *props)
{
    return props != 0
        && props->id != 0
        && props->width > 0
        && props->height > 0
        && props->period_ms > 0
        && props->source != 0
        && props->source->img_tile != 0;
}

static int tinyui_animation_attach_native(struct picoui_animation *animation,
                                          struct picoui_widget *parent,
                                          int width,
                                          int height,
                                          struct picoui_image_source *source,
                                          int period_ms)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldAnimation_t *ld_animation;
    uint16_t name_id;

    if (animation == 0 || parent == 0 || parent->backend_widget == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    parent_backend = (struct picoui_backend_widget *)parent->backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return -1;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        return -1;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        return -1;
    }

    ld_animation = ldAnimation_init(app_state->ld_scene,
                                    NULL,
                                    name_id,
                                    parent_backend->ld_name_id,
                                    0,
                                    0,
                                    (int16_t)width,
                                    (int16_t)height,
                                    source->img_tile,
                                    (uint16_t)period_ms);
    if (ld_animation == 0) {
        free(backend);
        return -1;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_ANIMATION,
                                         animation->id,
                                         parent_backend->theme) != 0) {
        ldAnimation_depose(app_state->ld_scene, ld_animation);
        free(backend);
        return -1;
    }
    backend->ld_widget = ld_animation;
    backend->ld_name_id = name_id;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldAnimation_depose(app_state->ld_scene, ld_animation);
        free(backend);
        return -1;
    }

    animation->widget.backend_widget = backend;
    if (tinyui_runtime_bridge_bind_host(animation->widget.backend_widget, &animation->widget) != 0) {
        (void)tinyui_runtime_bridge_detach_from_parent(animation->widget.backend_widget);
        ldAnimation_depose(app_state->ld_scene, ld_animation);
        free(backend);
        animation->widget.backend_widget = 0;
        return -1;
    }

    return 0;
}

/**
 * @brief Create animation widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_animation *picoui_animation_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_animation *animation;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    animation = calloc(1, sizeof(*animation));
    if (animation == 0) {
        return 0;
    }

    animation->id = id;
    animation->widget.visible = 1;
    animation->widget.enabled = 1;
    return animation;
}

/**
 * @brief animation init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_animation *picoui_animation_init(struct picoui_widget *parent, const char *id)
{
    return picoui_animation_create(parent, id);
}

/**
 * @brief Create animation widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_animation *picoui_animation_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_animation_props *props)
{
    struct picoui_animation *animation;

    if (!tinyui_animation_props_are_valid(props) || parent == 0 || parent->backend_widget == 0) {
        return 0;
    }

    animation = picoui_animation_create(parent, props->id);
    if (animation == 0) {
        return 0;
    }

    if (tinyui_animation_attach_native(animation,
                                       parent,
                                       props->width,
                                       props->height,
                                       props->source,
                                       props->period_ms) != 0
        || picoui_animation_set_source(animation, props->source) != 0
        || picoui_animation_set_period_ms(animation, props->period_ms) != 0) {
        free(animation);
        return 0;
    }

    animation->width = props->width;
    animation->height = props->height;
    animation->period_ms = props->period_ms;
    animation->source = props->source;
    animation->widget.width = props->width;
    animation->widget.height = props->height;
    if (picoui_animation_show_frame(animation, 0) != 0) {
        free(animation);
        return 0;
    }
    if (picoui_widget_set_user_data(&animation->widget, props->user_data) != 0) {
        free(animation);
        return 0;
    }
    if (props->style_class != 0 &&
        picoui_widget_set_style_class(&animation->widget, props->style_class) != 0) {
        free(animation);
        return 0;
    }

    return animation;
}

/**
 * @brief Set source of animation widget
 *
 * @param[in] animation Animation widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_animation_set_source(struct picoui_animation *animation, struct picoui_image_source *source)
{
    ldAnimation_t *ld_animation;

    if (animation == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_animation = tinyui_animation_get_ld(animation);
    if (ld_animation == 0) {
        return -1;
    }

    ld_animation->ptImgTile = source->img_tile;
    animation->source = source;
    return 0;
}

/**
 * @brief Set period ms of animation widget
 *
 * @param[in] animation Animation widget instance
 * @param[in] period_ms Period in milliseconds
 * @return 0 on success, -1 on failure
 */

int picoui_animation_set_period_ms(struct picoui_animation *animation, int period_ms)
{
    ldAnimation_t *ld_animation;

    if (animation == 0 || period_ms <= 0) {
        return -1;
    }

    ld_animation = tinyui_animation_get_ld(animation);
    if (ld_animation == 0 || period_ms > 0xFFFF) {
        return -1;
    }

    ld_animation->periodMs = (uint16_t)period_ms;
    animation->period_ms = period_ms;
    return 0;
}

/**
 * @brief animation show frame
 *
 * @param[in] animation Animation widget instance
 * @param[in] frame_index Frame index
 * @return -1 on failure
 */

int picoui_animation_show_frame(struct picoui_animation *animation, int frame_index)
{
    ldAnimation_t *ld_animation;
    arm_2d_tile_t *img_tile;
    int frames_per_row;
    int frame_count;
    int frame_x;
    int frame_y;

    if (animation == 0 || frame_index < 0) {
        return -1;
    }

    ld_animation = tinyui_animation_get_ld(animation);
    if (ld_animation == 0 || animation->source == 0 || animation->source->img_tile == 0 ||
        animation->width <= 0 || animation->height <= 0) {
        return -1;
    }

    img_tile = (arm_2d_tile_t *)animation->source->img_tile;
    frames_per_row = img_tile->tRegion.tSize.iWidth / animation->width;
    if (frames_per_row <= 0) {
        return -1;
    }

    frame_count = (img_tile->tRegion.tSize.iHeight / animation->height) * frames_per_row;
    if (frame_count <= 0 || frame_index >= frame_count) {
        return -1;
    }

    frame_x = (frame_index % frames_per_row) * animation->width;
    frame_y = (frame_index / frames_per_row) * animation->height;
    ld_animation->showRegion.tLocation.iX = (int16_t)frame_x;
    ld_animation->showRegion.tLocation.iY = (int16_t)frame_y;
    ld_animation->showRegion.tSize.iWidth = (int16_t)animation->width;
    ld_animation->showRegion.tSize.iHeight = (int16_t)animation->height;
    return 0;
}
