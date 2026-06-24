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
#include "widgets/animation.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldAnimation.h"

#include <stdlib.h>

/* ---- test seam state ---- */
static arm_2d_tile_t s_animation_default_tile = {
    .tRegion = {
        .tSize = {
            .iWidth = 1,
            .iHeight = 1,
        },
    },
};
static struct tinyui_image_source s_animation_default_source = {
    .img_tile = &s_animation_default_tile,
    .mask_tile = 0,
};

struct tinyui_animation_create_ctx {
    int width;
    int height;
    int period_ms;
    struct tinyui_image_source *source;
};


static int tinyui_animation_props_are_valid(const struct tinyui_animation_props *props)
{
    return props != 0
        && props->id != 0
        && props->width > 0
        && props->height > 0
        && props->period_ms > 0
        && props->source != 0
        && props->source->img_tile != 0;
}

static void *tinyui_animation_ld_init(void *ctx,
                                      struct ld_scene_t *scene,
                                      uint16_t name_id,
                                      uint16_t parent_name_id)
{
    struct tinyui_animation_create_ctx *create_ctx = (struct tinyui_animation_create_ctx *)ctx;
    struct tinyui_image_source *source;
    int width;
    int height;
    int period_ms;

    if (create_ctx == 0 || scene == 0) {
        return 0;
    }

    source = create_ctx->source;
    width = create_ctx->width;
    height = create_ctx->height;
    period_ms = create_ctx->period_ms;
    if (source == 0 || source->img_tile == 0
        || width <= 0 || width > INT16_MAX
        || height <= 0 || height > INT16_MAX
        || period_ms <= 0 || period_ms > 0xFFFF) {
        return 0;
    }

    return ldAnimation_init(scene,
                            NULL,
                            name_id,
                            parent_name_id,
                            0,
                            0,
                            (int16_t)width,
                            (int16_t)height,
                            source->img_tile,
                            (uint16_t)period_ms);
}

static struct tinyui_animation *tinyui_animation_alloc(struct tinyui_widget *parent,
                                                       const char *id,
                                                       int width,
                                                       int height,
                                                       struct tinyui_image_source *source,
                                                       int period_ms)
{
    struct tinyui_animation_create_ctx create_ctx;
    struct tinyui_animation *animation;

    if (parent == 0 || id == 0 || parent->ld_widget == 0
        || source == 0 || source->img_tile == 0) {
        return 0;
    }

    create_ctx.width = width;
    create_ctx.height = height;
    create_ctx.period_ms = period_ms;
    create_ctx.source = source;
    animation = (struct tinyui_animation *)tinyui_widget_create_leaf(parent,
                                                                     TINYUI_BACKEND_WIDGET_ANIMATION,
                                                                     tinyui_animation_ld_init,
                                                                     &create_ctx,
                                                                     sizeof(*animation));
    if (animation == 0) {
        return 0;
    }

    animation->id = id;
    animation->widget.visible = 1;
    animation->widget.enabled = 1;
    return animation;
}

/**
 * @brief Create animation widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_animation *tinyui_animation_create(struct tinyui_widget *parent, const char *id)
{
    return tinyui_animation_alloc(parent, id, 1, 1, &s_animation_default_source, 1);
}

/**
 * @brief animation init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_animation *tinyui_animation_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_animation_create(parent, id);
}

/**
 * @brief Create animation widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_animation *tinyui_animation_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_animation_props *props)
{
    struct tinyui_animation *animation;

    if (!tinyui_animation_props_are_valid(props) || parent == 0 || parent->ld_widget == 0) {
        return 0;
    }

    animation = tinyui_animation_alloc(parent,
                                       props->id,
                                       props->width,
                                       props->height,
                                       props->source,
                                       props->period_ms);
    if (animation == 0) {
        return 0;
    }

    if (tinyui_animation_set_source(animation, props->source) != 0
        || tinyui_animation_set_period_ms(animation, props->period_ms) != 0
        || tinyui_widget_set_size(&animation->widget, props->width, props->height) != 0) {
        tinyui_widget_destroy_common(&animation->widget);
        return 0;
    }

    animation->width = props->width;
    animation->height = props->height;
    animation->period_ms = props->period_ms;
    animation->source = props->source;
    animation->widget.width = props->width;
    animation->widget.height = props->height;
    if (tinyui_animation_show_frame(animation, 0) != 0
        || tinyui_widget_set_user_data(&animation->widget, props->user_data) != 0
        || (props->style_class != 0
            && tinyui_widget_set_style_class(&animation->widget, props->style_class) != 0)) {
        tinyui_widget_destroy_common(&animation->widget);
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

int tinyui_animation_set_source(struct tinyui_animation *animation, struct tinyui_image_source *source)
{
    ldAnimation_t *ld_animation;

    if (animation == 0 || source == 0 || source->img_tile == 0
        || animation->widget.ld_widget == 0
        || animation->widget.kind != TINYUI_BACKEND_WIDGET_ANIMATION) {
        return -1;
    }

    ld_animation = (ldAnimation_t *)animation->widget.ld_widget;
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

int tinyui_animation_set_period_ms(struct tinyui_animation *animation, int period_ms)
{
    ldAnimation_t *ld_animation;

    if (animation == 0 || period_ms <= 0 || period_ms > 0xFFFF
        || animation->widget.ld_widget == 0
        || animation->widget.kind != TINYUI_BACKEND_WIDGET_ANIMATION) {
        return -1;
    }

    ld_animation = (ldAnimation_t *)animation->widget.ld_widget;
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

int tinyui_animation_show_frame(struct tinyui_animation *animation, int frame_index)
{
    ldAnimation_t *ld_animation;
    arm_2d_tile_t *img_tile;
    int frames_per_row;
    int frame_count;
    int frame_x;
    int frame_y;

    if (animation == 0 || frame_index < 0
        || animation->widget.ld_widget == 0
        || animation->widget.kind != TINYUI_BACKEND_WIDGET_ANIMATION) {
        return -1;
    }

    ld_animation = (ldAnimation_t *)animation->widget.ld_widget;
    if (animation->source == 0 || animation->source->img_tile == 0
        || animation->width <= 0 || animation->height <= 0) {
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
