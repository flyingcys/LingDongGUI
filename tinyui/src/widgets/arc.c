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
#include "widgets/arc.h"
#include "internal/widget_legacy.h"
#include "../core/runtime_bridge.h"
#include "../../../examples/common/demo/widget/images/uiImages.h"
#include "../../../src/gui/ldArc.h"

#include <string.h>


static struct tinyui_arc *tinyui_arc_as_arc(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_ARC)) {
        return 0;
    }
    return (struct tinyui_arc *)w;
}

static const struct tinyui_arc *tinyui_arc_as_arc_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_ARC)) {
        return 0;
    }
    return (const struct tinyui_arc *)w;
}

static int tinyui_arc_props_are_valid(const tinyui_arc_props_t *props);
static unsigned int tinyui_arc_resolve_parent_color(const struct tinyui_arc *arc,
                                                    const tinyui_arc_props_t *props);

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
        tinyui_runtime_internal_widget_destroy_common(&arc->widget);
    } else {
        ldFree(arc);
    }
}

static void *tinyui_runtime_internal_arc_ld_init(void *ctx,
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
                        103,
                        103,
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
    const tinyui_arc_props_t *props)
{
    struct tinyui_arc *arc;

    if (!tinyui_arc_props_are_valid(props)) {
        return 0;
    }

    arc = tinyui_arc_create(parent);
    if (arc == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && tinyui_runtime_internal_widget_set_style_class(&arc->widget, props->style_class) != 0)
        || tinyui_runtime_internal_widget_set_user_data(&arc->widget, props->user_data) != 0
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

static int tinyui_arc_props_are_valid(const tinyui_arc_props_t *props)
{
    return props != 0
        && props->bg_start_angle >= 0.0f
        && props->bg_end_angle >= props->bg_start_angle
        && props->fg_end_angle >= 0.0f
        && props->rotation_angle >= 0.0f;
}

static unsigned int tinyui_arc_resolve_parent_color(const struct tinyui_arc *arc,
                                                    const tinyui_arc_props_t *props)
{
    if (arc == 0 || props == 0) {
        return 0xF0F0F0U;
    }

    if (props->has_parent_color != 0) {
        return props->parent_color;
    }

    return arc->parent_color;
}

tinyui_obj_t *tinyui_arc_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "arc";
    if (parent_w == 0) { return 0; }

    struct tinyui_arc *arc;
    arm_2d_tile_t *arc_img_tile;
    arm_2d_tile_t *arc_mask_tile;
    struct tinyui_arc_create_ctx ctx;

    if (parent_w == 0 || id == 0 || parent_w->ld_widget == 0) {
        return 0;
    }

    arc_img_tile = ldMalloc(sizeof(*arc_img_tile));
    if (arc_img_tile == 0) {
        return 0;
    }
    *arc_img_tile = *IMAGE_ARC_QUARTER_PNG_Mask;

    arc_mask_tile = ldMalloc(sizeof(*arc_mask_tile));
    if (arc_mask_tile == 0) {
        ldFree(arc_img_tile);
        return 0;
    }
    *arc_mask_tile = *IMAGE_ARC_QUARTER_MASK_PNG_Mask;

    ctx.arc_img_tile = arc_img_tile;
    ctx.arc_mask_tile = arc_mask_tile;
    arc = (struct tinyui_arc *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                         TINYUI_BACKEND_WIDGET_ARC,
                                                         tinyui_runtime_internal_arc_ld_init,
                                                         &ctx,
                                                         sizeof(*arc));
    if (arc == 0) {
        ldFree(arc_mask_tile);
        ldFree(arc_img_tile);
        return 0;
    }

    arc->id = id;
    arc->parent_color      = 0xF0F0F0U;

    if (tinyui_arc_set_background_angle(arc, 0.0f, 360.0f) != 0
        || tinyui_arc_set_foreground_angle(arc, 0.0f) != 0
        || tinyui_arc_set_rotation_angle(arc, 0.0f) != 0
        || tinyui_arc_set_parent_color(arc, arc->parent_color) != 0
        || tinyui_arc_set_color(arc, 0xFFFFFFU, 0xADD8E6U) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }

    return (tinyui_obj_t *)arc;
}

struct tinyui_arc *tinyui_runtime_internal_arc_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_arc_create(parent);
}

tinyui_obj_t *tinyui_arc_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_arc_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_arc *arc;

    if (props == 0) {
        return tinyui_arc_create(parent);
    }

    obj = tinyui_arc_create(parent);
    if (obj == 0) {
        return 0;
    }
    arc = (struct tinyui_arc *)(void *)obj;

    if ((props->fields & TINYUI_ARC_FIELD_ID) != 0) {
        (void)props->id;
    }
    if ((props->fields & TINYUI_ARC_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&arc->widget, props->user_data) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ARC_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&arc->widget, props->style_class) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ARC_FIELD_BG_START_ANGLE) != 0 ||
        (props->fields & TINYUI_ARC_FIELD_BG_END_ANGLE) != 0) {
        float a0 = ((props->fields & TINYUI_ARC_FIELD_BG_START_ANGLE) != 0) ? props->bg_start_angle : tinyui_arc_get_background_start_angle((const tinyui_obj_t *)arc);
        float a1 = ((props->fields & TINYUI_ARC_FIELD_BG_END_ANGLE) != 0) ? props->bg_end_angle : tinyui_arc_get_background_angle((const tinyui_obj_t *)arc);
            if (tinyui_arc_set_background_angle((tinyui_obj_t *)arc, a0, a1) != 0) {
                tinyui_arc_rollback(arc);
                return 0;
            }
    }
    if ((props->fields & TINYUI_ARC_FIELD_FG_END_ANGLE) != 0) {
    if (tinyui_arc_set_foreground_angle((tinyui_obj_t *)arc, props->fg_end_angle) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ARC_FIELD_ROTATION_ANGLE) != 0) {
    if (tinyui_arc_set_rotation_angle((tinyui_obj_t *)arc, props->rotation_angle) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ARC_FIELD_QUARTER_SOURCE) != 0) {
    if (tinyui_arc_set_quarter_source((tinyui_obj_t *)arc, props->quarter_source) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ARC_FIELD_PARENT_COLOR) != 0) {
    if (tinyui_arc_set_parent_color((tinyui_obj_t *)arc, props->parent_color) != 0) {
        tinyui_arc_rollback(arc);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ARC_FIELD_HAS_PARENT_COLOR) != 0) {
        if (props->has_parent_color) {
            unsigned int pc = ((props->fields & TINYUI_ARC_FIELD_PARENT_COLOR) != 0)
                                  ? props->parent_color
                                  : 0u;
            if (tinyui_arc_set_parent_color((tinyui_obj_t *)arc, pc) != 0) {
                tinyui_arc_rollback(arc);
                return 0;
            }
        }
    }
    if ((props->fields & TINYUI_ARC_FIELD_BG_COLOR) != 0 ||
        (props->fields & TINYUI_ARC_FIELD_FG_COLOR) != 0) {
        unsigned int bg = ((props->fields & TINYUI_ARC_FIELD_BG_COLOR) != 0) ? props->bg_color : tinyui_arc_get_background_color((const tinyui_obj_t *)arc);
        unsigned int fg = ((props->fields & TINYUI_ARC_FIELD_FG_COLOR) != 0) ? props->fg_color : tinyui_arc_get_foreground_color((const tinyui_obj_t *)arc);
            if (tinyui_arc_set_color((tinyui_obj_t *)arc, bg, fg) != 0) {
                tinyui_arc_rollback(arc);
                return 0;
            }
    }

    return obj;
}



int tinyui_arc_set_background_angle(tinyui_obj_t *arc_obj, float bg_start_angle, float bg_end_angle)
{
    struct tinyui_arc *arc = tinyui_arc_as_arc(arc_obj);
    if (arc == 0) { return -1; }

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

int tinyui_arc_set_foreground_angle(tinyui_obj_t *arc_obj, float fg_end_angle)
{
    struct tinyui_arc *arc = tinyui_arc_as_arc(arc_obj);
    if (arc == 0) { return -1; }

    if (arc == 0 || fg_end_angle < 0.0f
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ldArcSetForegroundAngle((ldArc_t *)arc->widget.ld_widget, fg_end_angle);
    arc->fg_end_angle = fg_end_angle;
    return 0;
}

int tinyui_arc_set_rotation_angle(tinyui_obj_t *arc_obj, float rotation_angle)
{
    struct tinyui_arc *arc = tinyui_arc_as_arc(arc_obj);
    if (arc == 0) { return -1; }

    if (arc == 0 || rotation_angle < 0.0f
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ldArcSetRotationAngle((ldArc_t *)arc->widget.ld_widget, rotation_angle);
    arc->rotation_angle = rotation_angle;
    return 0;
}

int tinyui_arc_set_color(tinyui_obj_t *arc_obj, unsigned int bg_color, unsigned int fg_color)
{
    struct tinyui_arc *arc = tinyui_arc_as_arc(arc_obj);
    if (arc == 0) { return -1; }

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

int tinyui_arc_set_quarter_source(tinyui_obj_t *arc_obj, struct tinyui_image_source *source)
{
    struct tinyui_arc *arc = tinyui_arc_as_arc(arc_obj);
    if (arc == 0) { return -1; }

    if (arc == 0 || source == 0 || source->kind == TINYUI_IMAGE_SOURCE_EMPTY
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ldArcSetQuarterImage((ldArc_t *)arc->widget.ld_widget,
                         tinyui_image_source_get_image_tile(source), tinyui_image_source_get_mask_tile(source), false, false);
    arc->quarter_source = source;
    return 0;
}

int tinyui_arc_set_quarter_image(tinyui_obj_t *arc_obj, struct tinyui_image_source *source)
{
    struct tinyui_arc *arc = tinyui_arc_as_arc(arc_obj);
    if (arc == 0) { return -1; }

    return tinyui_arc_set_quarter_source((tinyui_obj_t *)arc, source);
}

int tinyui_arc_set_parent_color(tinyui_obj_t *arc_obj, unsigned int parent_color)
{
    struct tinyui_arc *arc = tinyui_arc_as_arc(arc_obj);
    if (arc == 0) { return -1; }

    if (arc == 0 || parent_color > 0xFFFFFFU
        || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    ((ldArc_t *)arc->widget.ld_widget)->parentColor =
        (ldColor)tinyui_rgb_to_ld_color(parent_color);
    arc->parent_color = parent_color;
    return 0;
}

float tinyui_arc_get_background_start_angle(const tinyui_obj_t *arc_obj)
{
    const struct tinyui_arc *arc = tinyui_arc_as_arc_const(arc_obj);

    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0.0f;
    }
    return ldArcGetBackgroundStartAngle((ldArc_t *)arc->widget.ld_widget);
}

float tinyui_arc_get_background_angle(const tinyui_obj_t *arc_obj)
{
    const struct tinyui_arc *arc = tinyui_arc_as_arc_const(arc_obj);

    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0.0f;
    }
    /* Native ldArcGetBackgroundAngle returns the background end angle. */
    return ldArcGetBackgroundAngle((ldArc_t *)arc->widget.ld_widget);
}

float tinyui_arc_get_foreground_angle(const tinyui_obj_t *arc_obj)
{
    const struct tinyui_arc *arc = tinyui_arc_as_arc_const(arc_obj);

    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0.0f;
    }
    return ldArcGetForegroundAngle((ldArc_t *)arc->widget.ld_widget);
}

float tinyui_arc_get_rotation_angle(const tinyui_obj_t *arc_obj)
{
    const struct tinyui_arc *arc = tinyui_arc_as_arc_const(arc_obj);

    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0.0f;
    }
    return ldArcGetRotationAngle((ldArc_t *)arc->widget.ld_widget);
}

unsigned int tinyui_arc_get_background_color(const tinyui_obj_t *arc_obj)
{
    const struct tinyui_arc *arc = tinyui_arc_as_arc_const(arc_obj);

    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0;
    }
    return tinyui_ld_color_to_rgb((unsigned int)ldArcGetBackgroundColor((ldArc_t *)arc->widget.ld_widget));
}

unsigned int tinyui_arc_get_foreground_color(const tinyui_obj_t *arc_obj)
{
    const struct tinyui_arc *arc = tinyui_arc_as_arc_const(arc_obj);

    if (arc == 0 || arc->widget.ld_widget == 0
        || arc->widget.kind != TINYUI_BACKEND_WIDGET_ARC) {
        return 0;
    }
    return tinyui_ld_color_to_rgb((unsigned int)ldArcGetForegroundColor((ldArc_t *)arc->widget.ld_widget));
}
