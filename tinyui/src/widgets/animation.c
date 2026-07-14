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


static struct tinyui_animation *tinyui_animation_as_animation(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_ANIMATION)) {
        return 0;
    }
    return (struct tinyui_animation *)w;
}

static const struct tinyui_animation *tinyui_animation_as_animation_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_ANIMATION)) {
        return 0;
    }
    return (const struct tinyui_animation *)w;
}

/* ---- test seam state ---- */
static arm_2d_tile_t s_animation_default_tile = {
    .tRegion = {
        .tSize = {
            .iWidth = 1,
            .iHeight = 1,
        },
    },
};
static struct tinyui_image_source s_animation_default_source;

static void tinyui_animation_prepare_default_source(void)
{
    arm_2d_tile_t *tile = tinyui_image_source_get_image_tile(&s_animation_default_source);

    if (tile->tRegion.tSize.iWidth == 0) {
        *tile = s_animation_default_tile;
        s_animation_default_source.kind = TINYUI_IMAGE_SOURCE_EMPTY;
        s_animation_default_source.width = 1;
        s_animation_default_source.height = 1;
    }
}

struct tinyui_animation_create_ctx {
    int width;
    int height;
    int period_ms;
    struct tinyui_image_source *source;
};


static int tinyui_animation_props_are_valid(const tinyui_animation_props_t *props)
{
    return props != 0
        && props->width > 0
        && props->height > 0
        && props->period_ms > 0
        && props->source != 0
        && tinyui_image_source_get_image_tile(props->source) != 0;
}

static void *tinyui_runtime_internal_animation_ld_init(void *ctx,
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
    if (source == 0 || tinyui_image_source_get_image_tile(source) == 0
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
                            tinyui_image_source_get_image_tile(source),
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

    if (parent == 0 || id == 0 || ((struct tinyui_widget *)(void *)parent)->ld_widget == 0
        || source == 0 || tinyui_image_source_get_image_tile(source) == 0) {
        return 0;
    }

    create_ctx.width = width;
    create_ctx.height = height;
    create_ctx.period_ms = period_ms;
    create_ctx.source = source;
    animation = (struct tinyui_animation *)tinyui_runtime_internal_widget_create_leaf(parent,
                                                                     TINYUI_BACKEND_WIDGET_ANIMATION,
                                                                     tinyui_runtime_internal_animation_ld_init,
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

tinyui_obj_t *tinyui_animation_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "animation";
    if (parent_w == 0) { return 0; }

    tinyui_animation_prepare_default_source();
    return tinyui_animation_alloc(parent_w, id, 1, 1, &s_animation_default_source, 1);
}

/**
 * @brief animation init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_animation *tinyui_runtime_internal_animation_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_animation_create(parent);
}

/**
 * @brief Create animation widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_animation_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_animation_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_animation *animation;

    if (props == 0) {
        return tinyui_animation_create(parent);
    }

    obj = tinyui_animation_create(parent);
    if (obj == 0) {
        return 0;
    }
    animation = (struct tinyui_animation *)(void *)obj;

    if ((props->fields & TINYUI_ANIMATION_FIELD_ID) != 0) {
        (void)props->id;
    }
    if ((props->fields & TINYUI_ANIMATION_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&animation->widget, props->user_data) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)animation);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ANIMATION_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&animation->widget, props->style_class) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)animation);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ANIMATION_FIELD_WIDTH) != 0 || (props->fields & TINYUI_ANIMATION_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&animation->widget);
        int h = tinyui_runtime_internal_widget_get_height(&animation->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_ANIMATION_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_ANIMATION_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&animation->widget, w, h) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)animation);
            return 0;
        }
    }
    if ((props->fields & TINYUI_ANIMATION_FIELD_PERIOD_MS) != 0) {
    if (tinyui_animation_set_period_ms((tinyui_obj_t *)animation, props->period_ms) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)animation);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ANIMATION_FIELD_SOURCE) != 0) {
    if (tinyui_animation_set_source((tinyui_obj_t *)animation, props->source) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)animation);
        return 0;
    }
    }

    return obj;
}



/**
 * @brief Set source of animation widget
 *
 * @param[in] animation Animation widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_animation_set_source(tinyui_obj_t *animation_obj, struct tinyui_image_source *source)
{
    struct tinyui_animation *animation = tinyui_animation_as_animation(animation_obj);
    if (animation == 0) { return -1; }

    ldAnimation_t *ld_animation;

    if (animation == 0 || source == 0 || tinyui_image_source_get_image_tile(source) == 0
        || animation->widget.ld_widget == 0
        || animation->widget.kind != TINYUI_BACKEND_WIDGET_ANIMATION) {
        return -1;
    }

    ld_animation = (ldAnimation_t *)animation->widget.ld_widget;
    ld_animation->ptImgTile = tinyui_image_source_get_image_tile(source);
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

int tinyui_animation_set_period_ms(tinyui_obj_t *animation_obj, int period_ms)
{
    struct tinyui_animation *animation = tinyui_animation_as_animation(animation_obj);
    if (animation == 0) { return -1; }

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

int tinyui_animation_show_frame(tinyui_obj_t *animation_obj, int frame_index)
{
    struct tinyui_animation *animation = tinyui_animation_as_animation(animation_obj);
    if (animation == 0) { return -1; }

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
    if (animation->source == 0 || tinyui_image_source_get_image_tile(animation->source) == 0
        || animation->width <= 0 || animation->height <= 0) {
        return -1;
    }

    img_tile = (arm_2d_tile_t *)tinyui_image_source_get_image_tile(animation->source);
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
