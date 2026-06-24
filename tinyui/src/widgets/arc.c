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
#include "arc.h"
#include "widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldArc.h"

#include <stdlib.h>
#include <string.h>

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;


static int tinyui_arc_props_are_valid(const struct tinyui_arc_props *props);

struct tinyui_arc_create_ctx {
    arm_2d_tile_t *arc_img_tile;
    arm_2d_tile_t *arc_mask_tile;
};


static void tinyui_arc_rollback(struct tinyui_arc *arc)
{
    if (arc == 0) {
        return;
    }
    if (arc->widget.ld_widget != 0) {
        tinyui_widget_destroy_common(&arc->widget);
    } else {
        free(arc);
    }
}

static void *tinyui_arc_ld_init(void *ctx,
                                struct ld_scene_t *scene,
                                uint16_t name_id,
                                uint16_t parent_name_id)
{
    struct tinyui_arc_create_ctx *create_ctx = (struct tinyui_arc_create_ctx *)ctx;
    ldArc_t *ld_arc;

    if (create_ctx == 0 || create_ctx->arc_img_tile == 0 || create_ctx->arc_mask_tile == 0) {
        return 0;
    }

    ld_arc = ldArc_init(scene,
                        0,
                        name_id,
                        parent_name_id,
                        0,
                        0,
                        160,
                        160,
                        create_ctx->arc_img_tile,
                        create_ctx->arc_mask_tile,
                        GLCD_COLOR_WHITE);
    if (ld_arc == 0) {
        return 0;
    }
    ldArcSetQuarterImage(ld_arc,
                         create_ctx->arc_img_tile,
                         create_ctx->arc_mask_tile,
                         true,
                         true);
    return ld_arc;
}
struct tinyui_arc *tinyui_arc_test_create_with_props_fail_before_parent_color(
    struct tinyui_widget *parent,
    const struct tinyui_arc_props *props)
{
    struct tinyui_arc *arc;

    if (!tinyui_arc_props_are_valid(props)) {
        return 0;
    }

    arc = tinyui_arc_create(parent, props->id);
    if (arc == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && tinyui_widget_set_style_class(&arc->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&arc->widget, props->user_data) != 0
        || tinyui_arc_set_background_angle(arc, props->bg_start_angle, props->bg_end_angle) != 0
        || tinyui_arc_set_foreground_angle(arc, props->fg_end_angle) != 0
        || tinyui_arc_set_rotation_angle(arc, props->rotation_angle) != 0
        || (props->quarter_source != 0
            && tinyui_arc_set_quarter_source(arc, props->quarter_source) != 0)) {
        tinyui_arc_rollback(arc);
        return 0;
    }

    tinyui_arc_rollback(arc);
    return 0;
}

static int tinyui_arc_props_are_valid(const struct tinyui_arc_props *props)
{
    return props != 0
        && props->id != 0
        && props->bg_start_angle >= 0.0f
        && props->bg_end_angle >= props->bg_start_angle
        && props->fg_end_angle >= 0.0f
        && props->rotation_angle >= 0.0f;
}

struct tinyui_arc *tinyui_arc_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_arc *arc;
    arm_2d_tile_t *arc_img_tile;
    arm_2d_tile_t *arc_mask_tile;
    struct tinyui_arc_create_ctx ctx;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }

    arc_img_tile = malloc(sizeof(*arc_img_tile));
    if (arc_img_tile == 0) {
        return 0;
    }
    *arc_img_tile = c_tileQuaterArcGRAY8;

    arc_mask_tile = malloc(sizeof(*arc_mask_tile));
    if (arc_mask_tile == 0) {
        free(arc_img_tile);
        return 0;
    }
    *arc_mask_tile = c_tileQuaterArcMask;

    ctx.arc_img_tile = arc_img_tile;
    ctx.arc_mask_tile = arc_mask_tile;
    arc = (struct tinyui_arc *)tinyui_widget_create_leaf(parent,
                                                         TINYUI_BACKEND_WIDGET_ARC,
                                                         tinyui_arc_ld_init,
                                                         &ctx,
                                                         sizeof(*arc));
    if (arc == 0) {
        free(arc_mask_tile);
        free(arc_img_tile);
        return 0;
    }

    arc->id = id;
    arc->parent_color      = (unsigned int)GLCD_COLOR_WHITE;

    if (tinyui_arc_set_background_angle(arc, 0.0f, 360.0f) != 0
        || tinyui_arc_set_foreground_angle(arc, 0.0f) != 0
        || tinyui_arc_set_rotation_angle(arc, 0.0f) != 0
        || tinyui_arc_set_color(arc, 0xFFFFFFU, 0xADD8E6U) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }

    return arc;
}

struct tinyui_arc *tinyui_arc_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_arc_create(parent, id);
}

struct tinyui_arc *tinyui_arc_create_with_props(struct tinyui_widget *parent,
                                                const struct tinyui_arc_props *props)
{
    struct tinyui_arc *arc;

    if (!tinyui_arc_props_are_valid(props)) {
        return 0;
    }

    arc = tinyui_arc_create(parent, props->id);
    if (arc == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && tinyui_widget_set_style_class(&arc->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&arc->widget, props->user_data) != 0
        || tinyui_arc_set_background_angle(arc, props->bg_start_angle, props->bg_end_angle) != 0
        || tinyui_arc_set_foreground_angle(arc, props->fg_end_angle) != 0
        || tinyui_arc_set_rotation_angle(arc, props->rotation_angle) != 0
        || (props->quarter_source != 0
            && tinyui_arc_set_quarter_source(arc, props->quarter_source) != 0)
        || tinyui_arc_set_parent_color(arc, props->parent_color) != 0
        || tinyui_arc_set_color(arc, props->bg_color, props->fg_color) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }

    return arc;
}

int tinyui_arc_set_background_angle(struct tinyui_arc *arc, float bg_start_angle, float bg_end_angle)
{
    if (arc == 0 || bg_start_angle < 0.0f || bg_end_angle < bg_start_angle
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ldArcSetBackgroundAngle((ldArc_t *)arc->widget.ld_widget, bg_start_angle, bg_end_angle);
    arc->bg_start_angle = bg_start_angle;
    arc->bg_end_angle = bg_end_angle;
    return 0;
}

int tinyui_arc_set_foreground_angle(struct tinyui_arc *arc, float fg_end_angle)
{
    if (arc == 0 || fg_end_angle < 0.0f
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ldArcSetForegroundAngle((ldArc_t *)arc->widget.ld_widget, fg_end_angle);
    arc->fg_end_angle = fg_end_angle;
    return 0;
}

int tinyui_arc_set_rotation_angle(struct tinyui_arc *arc, float rotation_angle)
{
    if (arc == 0 || rotation_angle < 0.0f
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ldArcSetRotationAngle((ldArc_t *)arc->widget.ld_widget, rotation_angle);
    arc->rotation_angle = rotation_angle;
    return 0;
}

int tinyui_arc_set_color(struct tinyui_arc *arc, unsigned int bg_color, unsigned int fg_color)
{
    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ldArcSetColor((ldArc_t *)arc->widget.ld_widget,
                  (ldColor)tinyui_rgb_to_ld_color(bg_color),
                  (ldColor)tinyui_rgb_to_ld_color(fg_color));
    arc->bg_color = bg_color;
    arc->fg_color = fg_color;
    return 0;
}

int tinyui_arc_set_quarter_source(struct tinyui_arc *arc, struct tinyui_image_source *source)
{
    if (arc == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ldArcSetQuarterImage((ldArc_t *)arc->widget.ld_widget,
                         source->img_tile, source->mask_tile, false, false);
    arc->quarter_source = source;
    return 0;
}

int tinyui_arc_set_quarter_image(struct tinyui_arc *arc, struct tinyui_image_source *source)
{
    return tinyui_arc_set_quarter_source(arc, source);
}

int tinyui_arc_set_parent_color(struct tinyui_arc *arc, unsigned int parent_color)
{
    if (arc == 0 || parent_color > 0xFFFFFFU
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ((ldArc_t *)arc->widget.ld_widget)->parentColor = (ldColor)parent_color;
    arc->parent_color = parent_color;
    return 0;
}

float tinyui_arc_get_background_start_angle(const struct tinyui_arc *arc)
{
    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0.0f;
    }
    return ldArcGetBackgroundStartAngle((ldArc_t *)arc->widget.ld_widget);
}

float tinyui_arc_get_background_angle(const struct tinyui_arc *arc)
{
    ldArc_t *ld_arc;

    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0.0f;
    }
    ld_arc = (ldArc_t *)arc->widget.ld_widget;
    return ldArcGetBackgroundAngle(ld_arc) - ldArcGetBackgroundStartAngle(ld_arc);
}

float tinyui_arc_get_foreground_angle(const struct tinyui_arc *arc)
{
    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0.0f;
    }
    return ldArcGetForegroundAngle((ldArc_t *)arc->widget.ld_widget);
}

float tinyui_arc_get_rotation_angle(const struct tinyui_arc *arc)
{
    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0.0f;
    }
    return ldArcGetRotationAngle((ldArc_t *)arc->widget.ld_widget);
}

unsigned int tinyui_arc_get_background_color(const struct tinyui_arc *arc)
{
    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0;
    }
    return tinyui_ld_color_to_rgb((unsigned int)ldArcGetBackgroundColor((ldArc_t *)arc->widget.ld_widget));
}

unsigned int tinyui_arc_get_foreground_color(const struct tinyui_arc *arc)
{
    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0;
    }
    return tinyui_ld_color_to_rgb((unsigned int)ldArcGetForegroundColor((ldArc_t *)arc->widget.ld_widget));
}
