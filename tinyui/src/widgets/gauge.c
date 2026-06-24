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
#include "widgets/gauge.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldGauge.h"

#include <string.h>

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;


static int tinyui_gauge_props_are_valid(const struct tinyui_gauge_props *props);

struct tinyui_gauge_create_ctx {
    arm_2d_tile_t *bg_img_tile;
    arm_2d_tile_t *bg_mask_tile;
    arm_2d_tile_t *pointer_img_tile;
    arm_2d_tile_t *pointer_mask_tile;
};


static void tinyui_gauge_rollback(struct tinyui_gauge *gauge)
{
    if (gauge == 0) {
        return;
    }
    if (gauge->widget.ld_widget != 0) {
        tinyui_widget_destroy_common(&gauge->widget);
    } else {
        ldFree(gauge);
    }
}

static void *tinyui_gauge_ld_init(void *ctx,
                                  struct ld_scene_t *scene,
                                  uint16_t name_id,
                                  uint16_t parent_name_id)
{
    struct tinyui_gauge_create_ctx *create_ctx = (struct tinyui_gauge_create_ctx *)ctx;
    ldGauge_t *ld_gauge;

    if (create_ctx == 0
        || create_ctx->bg_img_tile == 0
        || create_ctx->bg_mask_tile == 0
        || create_ctx->pointer_img_tile == 0
        || create_ctx->pointer_mask_tile == 0) {
        return 0;
    }

    ld_gauge = ldGauge_init(scene,
                            0,
                            name_id,
                            parent_name_id,
                            0,
                            0,
                            160,
                            160,
                            create_ctx->bg_img_tile,
                            create_ctx->bg_mask_tile,
                            0,
                            0);
    if (ld_gauge == 0) {
        return 0;
    }

    ldGaugeSetBackgroundImage(ld_gauge,
                              create_ctx->bg_img_tile,
                              create_ctx->bg_mask_tile,
                              true,
                              true);
    ldGaugeBindPointerImage(ld_gauge,
                            create_ctx->pointer_img_tile,
                            create_ctx->pointer_mask_tile,
                            (int16_t)(create_ctx->pointer_mask_tile->tRegion.tSize.iWidth >> 1),
                            (int16_t)(create_ctx->pointer_mask_tile->tRegion.tSize.iHeight),
                            true,
                            true);
    return ld_gauge;
}
struct tinyui_gauge *tinyui_gauge_test_create_with_props_fail_before_centre_offset(
    struct tinyui_widget *parent,
    const struct tinyui_gauge_props *props)
{
    struct tinyui_gauge *gauge;

    if (!tinyui_gauge_props_are_valid(props)) {
        return 0;
    }

    gauge = tinyui_gauge_create(parent, props->id);
    if (gauge == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && tinyui_widget_set_style_class(&gauge->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&gauge->widget, props->user_data) != 0
        || tinyui_gauge_set_angle(gauge, props->angle) != 0
        || (props->bg_source != 0 && tinyui_gauge_set_bg_source(gauge, props->bg_source) != 0)
        || (props->pointer_source != 0 && tinyui_gauge_set_pointer_source(gauge, props->pointer_source) != 0)) {
        tinyui_gauge_rollback(gauge);
        return 0;
    }

    tinyui_gauge_rollback(gauge);
    return 0;
}

static int tinyui_gauge_props_are_valid(const struct tinyui_gauge_props *props)
{
    return props != 0 && props->id != 0;
}

struct tinyui_gauge *tinyui_gauge_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_gauge *gauge;
    arm_2d_tile_t *bg_img_tile;
    arm_2d_tile_t *bg_mask_tile;
    arm_2d_tile_t *pointer_img_tile;
    arm_2d_tile_t *pointer_mask_tile;
    struct tinyui_gauge_create_ctx ctx;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }

    bg_img_tile = ldMalloc(sizeof(*bg_img_tile));
    if (bg_img_tile == 0) {
        return 0;
    }
    *bg_img_tile = c_tileQuaterArcGRAY8;

    bg_mask_tile = ldMalloc(sizeof(*bg_mask_tile));
    if (bg_mask_tile == 0) {
        ldFree(bg_img_tile);
        return 0;
    }
    *bg_mask_tile = c_tileQuaterArcMask;

    pointer_img_tile = ldMalloc(sizeof(*pointer_img_tile));
    if (pointer_img_tile == 0) {
        ldFree(bg_mask_tile);
        ldFree(bg_img_tile);
        return 0;
    }
    *pointer_img_tile = c_tilePointerSecGRAY8;

    pointer_mask_tile = ldMalloc(sizeof(*pointer_mask_tile));
    if (pointer_mask_tile == 0) {
        ldFree(pointer_img_tile);
        ldFree(bg_mask_tile);
        ldFree(bg_img_tile);
        return 0;
    }
    *pointer_mask_tile = c_tilePointerSecMask;

    ctx.bg_img_tile = bg_img_tile;
    ctx.bg_mask_tile = bg_mask_tile;
    ctx.pointer_img_tile = pointer_img_tile;
    ctx.pointer_mask_tile = pointer_mask_tile;
    gauge = (struct tinyui_gauge *)tinyui_widget_create_leaf(parent,
                                                             TINYUI_BACKEND_WIDGET_GAUGE,
                                                             tinyui_gauge_ld_init,
                                                             &ctx,
                                                             sizeof(*gauge));
    if (gauge == 0) {
        ldFree(pointer_mask_tile);
        ldFree(pointer_img_tile);
        ldFree(bg_mask_tile);
        ldFree(bg_img_tile);
        return 0;
    }

    gauge->id = id;
    gauge->widget.value      = 0;

    if (tinyui_gauge_set_angle(gauge, 0.0f) != 0
        || tinyui_gauge_set_pointer_color(gauge, 0x000000U) != 0
        || tinyui_gauge_set_auto_move(gauge, 0) != 0) {
        tinyui_gauge_rollback(gauge);
        return 0;
    }

    return gauge;
}

struct tinyui_gauge *tinyui_gauge_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_gauge_create(parent, id);
}

struct tinyui_gauge *tinyui_gauge_create_with_props(struct tinyui_widget *parent,
                                                    const struct tinyui_gauge_props *props)
{
    struct tinyui_gauge *gauge;

    if (!tinyui_gauge_props_are_valid(props)) {
        return 0;
    }

    gauge = tinyui_gauge_create(parent, props->id);
    if (gauge == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && tinyui_widget_set_style_class(&gauge->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&gauge->widget, props->user_data) != 0
        || tinyui_gauge_set_angle(gauge, props->angle) != 0
        || (props->bg_source != 0 && tinyui_gauge_set_bg_source(gauge, props->bg_source) != 0)
        || (props->pointer_source != 0 && tinyui_gauge_set_pointer_source(gauge, props->pointer_source) != 0)
        || tinyui_gauge_set_centre_offset(gauge, props->centre_offset_x, props->centre_offset_y) != 0
        || tinyui_gauge_set_pointer_color(gauge, props->pointer_color) != 0
        || tinyui_gauge_set_auto_move(gauge, props->auto_move) != 0) {
        tinyui_gauge_rollback(gauge);
        return 0;
    }

    return gauge;
}

int tinyui_gauge_set_angle(struct tinyui_gauge *gauge, float angle)
{
    if (gauge == 0 || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    ldGaugeSetAngle((ldGauge_t *)gauge->widget.ld_widget, angle);
    gauge->widget.value = (int)angle;
    gauge->angle = angle;
    return 0;
}

float tinyui_gauge_get_angle(const struct tinyui_gauge *gauge)
{
    if (gauge == 0 || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return 0.0f;
    }

    return (float)((ldGauge_t *)gauge->widget.ld_widget)->_nowAngle_x10 / 10.0f;
}

int tinyui_gauge_set_bg_source(struct tinyui_gauge *gauge, struct tinyui_image_source *source)
{
    ldGauge_t *ld_gauge;

    if (gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0
        || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    ld_gauge = (ldGauge_t *)gauge->widget.ld_widget;
    ldGaugeSetBackgroundImage(ld_gauge, source->img_tile, source->mask_tile, false, false);
    gauge->bg_source = source;
    return 0;
}

int tinyui_gauge_set_background_image(struct tinyui_gauge *gauge, struct tinyui_image_source *source)
{
    return tinyui_gauge_set_bg_source(gauge, source);
}

int tinyui_gauge_set_pointer_source(struct tinyui_gauge *gauge, struct tinyui_image_source *source)
{
    ldGauge_t *ld_gauge;
    arm_2d_tile_t *mask_tile;

    if (gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0
        || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    ld_gauge = (ldGauge_t *)gauge->widget.ld_widget;
    mask_tile = (arm_2d_tile_t *)source->mask_tile;
    ldGaugeBindPointerImage(ld_gauge,
                            source->img_tile,
                            source->mask_tile,
                            (int16_t)(mask_tile->tRegion.tSize.iWidth >> 1),
                            (int16_t)(mask_tile->tRegion.tSize.iHeight),
                            false,
                            false);
    gauge->pointer_source = source;
    return 0;
}

int tinyui_gauge_set_centre_offset(struct tinyui_gauge *gauge, int centre_offset_x, int centre_offset_y)
{
    ldGauge_t *ld_gauge;

    if (gauge == 0 || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    ld_gauge = (ldGauge_t *)gauge->widget.ld_widget;
    ld_gauge->centreOffsetX = (int16_t)centre_offset_x;
    ld_gauge->centreOffsetY = (int16_t)centre_offset_y;
    gauge->centre_offset_x = centre_offset_x;
    gauge->centre_offset_y = centre_offset_y;
    return 0;
}

int tinyui_gauge_set_trail(struct tinyui_gauge *gauge,
                           struct tinyui_image_source *bg_trail_source,
                           struct tinyui_image_source *pointer_trail_source)
{
    ldGauge_t *ld_gauge;

    if (gauge == 0
        || bg_trail_source == 0
        || pointer_trail_source == 0
        || bg_trail_source->mask_tile == 0
        || pointer_trail_source->mask_tile == 0
        || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    ld_gauge = (ldGauge_t *)gauge->widget.ld_widget;
    ldGaugeSetTrail(ld_gauge,
                    bg_trail_source->mask_tile,
                    pointer_trail_source->mask_tile);
    return 0;
}

int tinyui_gauge_set_progress_bar(struct tinyui_gauge *gauge,
                                  struct tinyui_image_source *bg_progress_source,
                                  struct tinyui_image_source *pointer_progress_source)
{
    ldGauge_t *ld_gauge;

    if (gauge == 0
        || bg_progress_source == 0
        || pointer_progress_source == 0
        || bg_progress_source->mask_tile == 0
        || pointer_progress_source->mask_tile == 0
        || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    ld_gauge = (ldGauge_t *)gauge->widget.ld_widget;
    ldGaugeSetProgressBar(ld_gauge,
                          bg_progress_source->mask_tile,
                          pointer_progress_source->mask_tile);
    return 0;
}

int tinyui_gauge_set_pointer_color(struct tinyui_gauge *gauge, unsigned int pointer_color)
{
    if (gauge == 0 || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    ldGaugeSetPointerColor((ldGauge_t *)gauge->widget.ld_widget,
                           (ldColor)tinyui_rgb_to_ld_color(pointer_color));
    gauge->pointer_color = pointer_color;
    return 0;
}

unsigned int tinyui_gauge_get_pointer_color(const struct tinyui_gauge *gauge)
{
    if (gauge == 0 || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return 0;
    }

    return tinyui_ld_color_to_rgb((unsigned int)((ldGauge_t *)gauge->widget.ld_widget)->maskColor);
}

int tinyui_gauge_set_auto_move(struct tinyui_gauge *gauge, int auto_move)
{
    if (gauge == 0 || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    ldGaugeSetAutoMove((ldGauge_t *)gauge->widget.ld_widget, auto_move != 0);
    gauge->auto_move = auto_move != 0 ? 1 : 0;
    return 0;
}

int tinyui_gauge_get_auto_move(const struct tinyui_gauge *gauge)
{
    if (gauge == 0 || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return -1;
    }

    return ((ldGauge_t *)gauge->widget.ld_widget)->isAutoMove ? 1 : 0;
}
