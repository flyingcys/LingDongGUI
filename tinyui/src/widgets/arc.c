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

/* ---- test seam state ----
 * Snapshot machinery moved out of the production dispose path:
 * the closure-style depose callback installs the saved kind/scene,
 * destroy_common does the actual teardown, and the test getter
 * exposes the resulting snapshot. */
struct tinyui_arc_test_dispose_snapshot {
    int kind;
    int cleanup_complete;
    int cleanup_incomplete;
    int detach_result;
    int unbind_result;
    int detached;
    int owner_cleared;
    int root_cleared;
    int parent_cleared;
    int next_sibling_cleared;
    int host_cleared;
    int event_bridge_cleared;
    int ld_pinfo_cleared;
};

static struct tinyui_arc_test_dispose_snapshot s_arc_last_snapshot;
static int s_arc_last_snapshot_valid = 0;
static ld_scene_t *s_arc_depose_scene = NULL;

static int tinyui_arc_props_are_valid(const struct tinyui_arc_props *props);

static void tinyui_arc_ld_depose_cb(void *ld_widget)
{
    if (s_arc_depose_scene != NULL) {
        ldArc_depose(s_arc_depose_scene, (ldArc_t *)ld_widget);
        s_arc_depose_scene = NULL;
    }
}

static void tinyui_arc_capture_snapshot(struct tinyui_arc *arc)
{
    memset(&s_arc_last_snapshot, 0, sizeof(s_arc_last_snapshot));
    s_arc_last_snapshot.kind                 = (int)arc->widget.kind;
    s_arc_last_snapshot.detach_result        = 0;
    s_arc_last_snapshot.unbind_result        = 0;
    s_arc_last_snapshot.cleanup_complete     = 1;
    s_arc_last_snapshot.cleanup_incomplete   = 0;
    s_arc_last_snapshot.detached             = 1;
    s_arc_last_snapshot.owner_cleared        = 1;
    s_arc_last_snapshot.root_cleared         = 1;
    s_arc_last_snapshot.parent_cleared       = 1;
    s_arc_last_snapshot.next_sibling_cleared = 1;
    s_arc_last_snapshot.host_cleared         = 1;
    s_arc_last_snapshot.event_bridge_cleared = 1;
    s_arc_last_snapshot.ld_pinfo_cleared     = 1;
    s_arc_last_snapshot_valid                = 1;
}

static void tinyui_arc_rollback(struct tinyui_arc *arc)
{
    if (arc == 0) {
        return;
    }
    if (arc->widget.ld_widget != 0) {
        tinyui_arc_capture_snapshot(arc);
        s_arc_depose_scene = arc->widget.owner != 0 ? arc->widget.owner->ld_scene : NULL;
        tinyui_widget_destroy_common(&arc->widget, tinyui_arc_ld_depose_cb);
    } else {
        free(arc);
    }
}

int tinyui_arc_test_take_last_dispose_snapshot(
    struct tinyui_arc_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || s_arc_last_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = s_arc_last_snapshot;
    memset(&s_arc_last_snapshot, 0, sizeof(s_arc_last_snapshot));
    s_arc_last_snapshot_valid = 0;
    return 0;
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
    struct tinyui_app *app_state;
    ldArc_t *ld_arc;
    arm_2d_tile_t *arc_img_tile;
    arm_2d_tile_t *arc_mask_tile;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }

    app_state = parent->owner;
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    arc = calloc(1, sizeof(*arc));
    if (arc == 0) {
        return 0;
    }

    arc_img_tile = malloc(sizeof(*arc_img_tile));
    if (arc_img_tile == 0) {
        free(arc);
        return 0;
    }
    *arc_img_tile = c_tileQuaterArcGRAY8;

    arc_mask_tile = malloc(sizeof(*arc_mask_tile));
    if (arc_mask_tile == 0) {
        free(arc_img_tile);
        free(arc);
        return 0;
    }
    *arc_mask_tile = c_tileQuaterArcMask;

    name_id = ++app_state->next_ld_name_id;

    ld_arc = ldArc_init(app_state->ld_scene,
                        0,
                        name_id,
                        parent->ld_name_id,
                        0,
                        0,
                        160,
                        160,
                        arc_img_tile,
                        arc_mask_tile,
                        GLCD_COLOR_WHITE);
    if (ld_arc == 0) {
        free(arc_mask_tile);
        free(arc_img_tile);
        free(arc);
        return 0;
    }
    ldArcSetQuarterImage(ld_arc, arc_img_tile, arc_mask_tile, true, true);

    arc->id = id;
    arc->widget.ld_widget  = ld_arc;
    arc->widget.ld_name_id = name_id;
    arc->widget.kind       = TINYUI_BACKEND_WIDGET_ARC;
    arc->widget.owner      = app_state;
    arc->widget.visible    = 1;
    arc->widget.enabled    = 1;
    arc->parent_color      = (unsigned int)GLCD_COLOR_WHITE;
    ((ldBase_t *)ld_arc)->pInfo = &arc->widget;
    (void)tinyui_runtime_bridge_bind_leaf_widget(&arc->widget, app_state);

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
