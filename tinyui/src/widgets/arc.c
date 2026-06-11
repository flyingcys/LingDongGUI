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
#include "picoui/arc.h"
#include "picoui/widget.h"
#include "../core/runtime_bridge.h"
#include "../backend/ldgui/backend.h"
#include "../../../src/gui/ldArc.h"

#include <stdlib.h>
int picoui_backend_widget_unbind_host(void *backend_widget);
int picoui_backend_widget_detach_from_parent(void *backend_widget);

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;

struct picoui_arc_test_dispose_snapshot {
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

static struct picoui_arc_test_dispose_snapshot picoui_arc_last_dispose_snapshot;
static int picoui_arc_last_dispose_snapshot_valid = 0;

static int picoui_arc_props_are_valid(const struct picoui_arc_props *props);

static ldColor picoui_arc_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static unsigned int picoui_arc_ld_color_to_rgb(ldColor color)
{
    uint32_t red = ((uint32_t)color >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)color >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)color & 0x1FU;

    red = (red << 3) | (red >> 2);
    green = (green << 2) | (green >> 4);
    blue = (blue << 3) | (blue >> 2);
    return (red << 16) | (green << 8) | blue;
}

static struct picoui_backend_widget *picoui_arc_backend(struct picoui_arc *arc)
{
    struct picoui_backend_widget *backend;

    if (arc == 0 || arc->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)arc->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_ARC || backend->ld_widget == 0) {
        return 0;
    }

    return backend;
}

static ldArc_t *picoui_arc_get_ld(struct picoui_arc *arc)
{
    struct picoui_backend_widget *backend = picoui_arc_backend(arc);

    if (backend == 0) {
        return 0;
    }

    return (ldArc_t *)backend->ld_widget;
}

static int picoui_arc_finish_detach_after_backend_failure(
    struct picoui_backend_widget *backend)
{
    struct picoui_backend_widget *parent;
    struct picoui_backend_widget *cursor;

    if (backend == 0 || backend->parent == 0) {
        return 0;
    }

    parent = backend->parent;
    if (parent->first_child == backend) {
        parent->first_child = backend->next_sibling;
    } else {
        cursor = parent->first_child;
        while (cursor != 0 && cursor->next_sibling != backend) {
            cursor = cursor->next_sibling;
        }
        if (cursor == 0) {
            return -1;
        }
        cursor->next_sibling = backend->next_sibling;
    }

    backend->parent = 0;
    backend->next_sibling = 0;
    backend->owner = 0;
    backend->root = 0;
    return 0;
}

static void picoui_arc_dispose_partial_impl(struct picoui_arc *arc)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldBase_t *ld_base;
    int detach_result = 0;
    int unbind_result = 0;

    if (arc == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)arc->widget.backend_widget;
    if (backend != 0) {
        app_state = picoui_runtime_bridge_backend_state(backend->owner);
        ld_base = (ldBase_t *)backend->ld_widget;
        memset(&picoui_arc_last_dispose_snapshot, 0, sizeof(picoui_arc_last_dispose_snapshot));
        picoui_arc_last_dispose_snapshot.kind = backend->kind;
        if (backend->parent != 0) {
            detach_result = picoui_backend_widget_detach_from_parent(backend);
            if (detach_result != 0) {
                detach_result = picoui_arc_finish_detach_after_backend_failure(backend);
            }
        } else {
            picoui_arc_last_dispose_snapshot.detached = 1;
        }
        unbind_result = picoui_backend_widget_unbind_host(backend);
        picoui_arc_last_dispose_snapshot.detach_result = detach_result;
        picoui_arc_last_dispose_snapshot.unbind_result = unbind_result;
        picoui_arc_last_dispose_snapshot.cleanup_complete =
            (detach_result == 0 && unbind_result == 0);
        picoui_arc_last_dispose_snapshot.cleanup_incomplete =
            (detach_result != 0 || unbind_result != 0);
        picoui_arc_last_dispose_snapshot.detached = (detach_result == 0 && backend->parent == 0);
        picoui_arc_last_dispose_snapshot.owner_cleared = (backend->owner == 0);
        picoui_arc_last_dispose_snapshot.root_cleared = (backend->root == 0);
        picoui_arc_last_dispose_snapshot.parent_cleared = (backend->parent == 0);
        picoui_arc_last_dispose_snapshot.next_sibling_cleared = (backend->next_sibling == 0);
        picoui_arc_last_dispose_snapshot.host_cleared = (backend->host_widget == 0);
        picoui_arc_last_dispose_snapshot.event_bridge_cleared =
            (backend->ld_event_bridge_scene == 0
             && backend->ld_event_bridge_sender == 0
             && backend->ld_event_bridge_next == 0);
        picoui_arc_last_dispose_snapshot.ld_pinfo_cleared =
            (ld_base == 0 || ld_base->pInfo == 0);
        picoui_arc_last_dispose_snapshot_valid = 1;
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldArc_depose(app_state->ld_scene, (ldArc_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(arc);
}

int picoui_backend_arc_test_take_last_dispose_snapshot(
    struct picoui_arc_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || picoui_arc_last_dispose_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = picoui_arc_last_dispose_snapshot;
    memset(&picoui_arc_last_dispose_snapshot, 0, sizeof(picoui_arc_last_dispose_snapshot));
    picoui_arc_last_dispose_snapshot_valid = 0;
    return 0;
}

struct picoui_arc *picoui_backend_arc_test_create_with_props_fail_before_parent_color(
    struct picoui_widget *parent,
    const struct picoui_arc_props *props)
{
    struct picoui_arc *arc;

    if (!picoui_arc_props_are_valid(props)) {
        return 0;
    }

    arc = picoui_arc_create(parent, props->id);
    if (arc == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && picoui_widget_set_style_class(&arc->widget, props->style_class) != 0)
        || picoui_widget_set_user_data(&arc->widget, props->user_data) != 0
        || picoui_arc_set_background_angle(arc, props->bg_start_angle, props->bg_end_angle) != 0
        || picoui_arc_set_foreground_angle(arc, props->fg_end_angle) != 0
        || picoui_arc_set_rotation_angle(arc, props->rotation_angle) != 0
        || (props->quarter_source != 0
            && picoui_arc_set_quarter_source(arc, props->quarter_source) != 0)) {
        picoui_arc_dispose_partial_impl(arc);
        return 0;
    }

    picoui_arc_dispose_partial_impl(arc);
    return 0;
}

static int picoui_arc_props_are_valid(const struct picoui_arc_props *props)
{
    return props != 0
        && props->id != 0
        && props->bg_start_angle >= 0.0f
        && props->bg_end_angle >= props->bg_start_angle
        && props->fg_end_angle >= 0.0f
        && props->rotation_angle >= 0.0f;
}

struct picoui_arc *picoui_arc_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_arc *arc;
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldArc_t *ld_arc;
    arm_2d_tile_t *arc_img_tile;
    arm_2d_tile_t *arc_mask_tile;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->backend_widget;
    app_state = picoui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    arc = calloc(1, sizeof(*arc));
    if (arc == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(arc);
        return 0;
    }

    arc_img_tile = malloc(sizeof(*arc_img_tile));
    if (arc_img_tile == 0) {
        free(backend);
        free(arc);
        return 0;
    }
    *arc_img_tile = c_tileQuaterArcGRAY8;

    arc_mask_tile = malloc(sizeof(*arc_mask_tile));
    if (arc_mask_tile == 0) {
        free(arc_img_tile);
        free(backend);
        free(arc);
        return 0;
    }
    *arc_mask_tile = c_tileQuaterArcMask;

    name_id = picoui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(arc_mask_tile);
        free(arc_img_tile);
        free(backend);
        free(arc);
        return 0;
    }

    ld_arc = ldArc_init(app_state->ld_scene,
                        0,
                        name_id,
                        parent_backend->ld_name_id,
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
        free(backend);
        free(arc);
        return 0;
    }
    ldArcSetQuarterImage(ld_arc, arc_img_tile, arc_mask_tile, true, true);

    if (picoui_backend_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_ARC,
                                         id,
                                         parent_backend->theme) != 0) {
        ldArc_depose(app_state->ld_scene, ld_arc);
        free(backend);
        free(arc);
        return 0;
    }
    backend->ld_widget = ld_arc;
    backend->ld_name_id = name_id;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent_backend, backend) != 0) {
        ldArc_depose(app_state->ld_scene, ld_arc);
        free(backend);
        free(arc);
        return 0;
    }

    arc->id = id;
    arc->widget.backend_widget = backend;
    arc->widget.visible = 1;
    arc->widget.enabled = 1;
    arc->parent_color = (unsigned int)GLCD_COLOR_WHITE;
    if (picoui_backend_widget_bind_host(arc->widget.backend_widget, &arc->widget) != 0
        || picoui_arc_set_background_angle(arc, 0.0f, 360.0f) != 0
        || picoui_arc_set_foreground_angle(arc, 0.0f) != 0
        || picoui_arc_set_rotation_angle(arc, 0.0f) != 0
        || picoui_arc_set_color(arc, 0xFFFFFFU, 0xADD8E6U) != 0) {
        picoui_arc_dispose_partial_impl(arc);
        return 0;
    }

    return arc;
}

struct picoui_arc *picoui_arc_init(struct picoui_widget *parent, const char *id)
{
    return picoui_arc_create(parent, id);
}

struct picoui_arc *picoui_arc_create_with_props(struct picoui_widget *parent,
                                                const struct picoui_arc_props *props)
{
    struct picoui_arc *arc;

    if (!picoui_arc_props_are_valid(props)) {
        return 0;
    }

    arc = picoui_arc_create(parent, props->id);
    if (arc == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && picoui_widget_set_style_class(&arc->widget, props->style_class) != 0)
        || picoui_widget_set_user_data(&arc->widget, props->user_data) != 0
        || picoui_arc_set_background_angle(arc, props->bg_start_angle, props->bg_end_angle) != 0
        || picoui_arc_set_foreground_angle(arc, props->fg_end_angle) != 0
        || picoui_arc_set_rotation_angle(arc, props->rotation_angle) != 0
        || (props->quarter_source != 0
            && picoui_arc_set_quarter_source(arc, props->quarter_source) != 0)
        || picoui_arc_set_parent_color(arc, props->parent_color) != 0
        || picoui_arc_set_color(arc, props->bg_color, props->fg_color) != 0) {
        picoui_arc_dispose_partial_impl(arc);
        return 0;
    }

    return arc;
}

int picoui_backend_arc_set_background_angle(struct picoui_arc *arc, float bg_start_angle, float bg_end_angle)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0 || bg_end_angle < bg_start_angle) {
        return -1;
    }

    ldArcSetBackgroundAngle(ld_arc, bg_start_angle, bg_end_angle);
    return 0;
}

int picoui_arc_set_background_angle(struct picoui_arc *arc, float bg_start_angle, float bg_end_angle)
{
    if (arc == 0 || bg_start_angle < 0.0f || bg_end_angle < bg_start_angle) {
        return -1;
    }
    if (picoui_backend_arc_set_background_angle(arc, bg_start_angle, bg_end_angle) != 0) {
        return -1;
    }
    arc->bg_start_angle = bg_start_angle;
    arc->bg_end_angle = bg_end_angle;
    return 0;
}

int picoui_backend_arc_set_foreground_angle(struct picoui_arc *arc, float fg_end_angle)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0) {
        return -1;
    }

    ldArcSetForegroundAngle(ld_arc, fg_end_angle);
    return 0;
}

int picoui_arc_set_foreground_angle(struct picoui_arc *arc, float fg_end_angle)
{
    if (arc == 0 || fg_end_angle < 0.0f) {
        return -1;
    }
    if (picoui_backend_arc_set_foreground_angle(arc, fg_end_angle) != 0) {
        return -1;
    }
    arc->fg_end_angle = fg_end_angle;
    return 0;
}

int picoui_backend_arc_set_rotation_angle(struct picoui_arc *arc, float rotation_angle)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0) {
        return -1;
    }

    ldArcSetRotationAngle(ld_arc, rotation_angle);
    return 0;
}

int picoui_arc_set_rotation_angle(struct picoui_arc *arc, float rotation_angle)
{
    if (arc == 0 || rotation_angle < 0.0f) {
        return -1;
    }
    if (picoui_backend_arc_set_rotation_angle(arc, rotation_angle) != 0) {
        return -1;
    }
    arc->rotation_angle = rotation_angle;
    return 0;
}

int picoui_backend_arc_set_color(struct picoui_arc *arc, unsigned int bg_color, unsigned int fg_color)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0) {
        return -1;
    }

    ldArcSetColor(ld_arc,
                  picoui_arc_rgb_to_ld_color(bg_color),
                  picoui_arc_rgb_to_ld_color(fg_color));
    return 0;
}

int picoui_arc_set_color(struct picoui_arc *arc, unsigned int bg_color, unsigned int fg_color)
{
    if (arc == 0) {
        return -1;
    }
    if (picoui_backend_arc_set_color(arc, bg_color, fg_color) != 0) {
        return -1;
    }
    arc->bg_color = bg_color;
    arc->fg_color = fg_color;
    return 0;
}

int picoui_backend_arc_set_quarter_source(struct picoui_arc *arc, struct picoui_image_source *source)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }

    ldArcSetQuarterImage(ld_arc, source->img_tile, source->mask_tile, false, false);
    return 0;
}

int picoui_arc_set_quarter_source(struct picoui_arc *arc, struct picoui_image_source *source)
{
    if (arc == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }

    if (picoui_backend_arc_set_quarter_source(arc, source) != 0) {
        return -1;
    }

    arc->quarter_source = source;
    return 0;
}

int picoui_backend_arc_set_parent_color(struct picoui_arc *arc, unsigned int parent_color)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0 || parent_color > 0xFFFFFFU) {
        return -1;
    }

    ld_arc->parentColor = (ldColor)parent_color;
    return 0;
}

int picoui_arc_set_parent_color(struct picoui_arc *arc, unsigned int parent_color)
{
    if (arc == 0 || parent_color > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_arc_set_parent_color(arc, parent_color) != 0) {
        return -1;
    }

    arc->parent_color = parent_color;
    return 0;
}

int picoui_backend_arc_get_background_angle(struct picoui_arc *arc, float *bg_start_angle, float *bg_angle)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0 || bg_start_angle == 0 || bg_angle == 0) {
        return -1;
    }

    *bg_start_angle = ldArcGetBackgroundStartAngle(ld_arc);
    *bg_angle = ldArcGetBackgroundAngle(ld_arc);
    return 0;
}

float picoui_arc_get_background_start_angle(const struct picoui_arc *arc)
{
    float bg_start_angle = 0.0f;
    float bg_angle = 0.0f;

    if (arc == 0) {
        return 0.0f;
    }
    if (picoui_backend_arc_get_background_angle((struct picoui_arc *)arc, &bg_start_angle, &bg_angle) != 0) {
        return 0.0f;
    }
    return bg_start_angle;
}

float picoui_arc_get_background_angle(const struct picoui_arc *arc)
{
    float bg_start_angle = 0.0f;
    float bg_end_angle = 0.0f;

    if (arc == 0) {
        return 0.0f;
    }
    if (picoui_backend_arc_get_background_angle((struct picoui_arc *)arc, &bg_start_angle, &bg_end_angle) != 0) {
        return 0.0f;
    }
    return bg_end_angle - bg_start_angle;
}

int picoui_backend_arc_get_foreground_angle(struct picoui_arc *arc, float *fg_end_angle)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0 || fg_end_angle == 0) {
        return -1;
    }

    *fg_end_angle = ldArcGetForegroundAngle(ld_arc);
    return 0;
}

float picoui_arc_get_foreground_angle(const struct picoui_arc *arc)
{
    float fg_end_angle = 0.0f;

    if (arc == 0) {
        return 0.0f;
    }
    if (picoui_backend_arc_get_foreground_angle((struct picoui_arc *)arc, &fg_end_angle) != 0) {
        return 0.0f;
    }
    return fg_end_angle;
}

int picoui_backend_arc_get_rotation_angle(struct picoui_arc *arc, float *rotation_angle)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0 || rotation_angle == 0) {
        return -1;
    }

    *rotation_angle = ldArcGetRotationAngle(ld_arc);
    return 0;
}

float picoui_arc_get_rotation_angle(const struct picoui_arc *arc)
{
    float rotation_angle = 0.0f;

    if (arc == 0) {
        return 0.0f;
    }
    if (picoui_backend_arc_get_rotation_angle((struct picoui_arc *)arc, &rotation_angle) != 0) {
        return 0.0f;
    }
    return rotation_angle;
}

int picoui_backend_arc_get_color(struct picoui_arc *arc, unsigned int *bg_color, unsigned int *fg_color)
{
    ldArc_t *ld_arc = picoui_arc_get_ld(arc);

    if (ld_arc == 0 || bg_color == 0 || fg_color == 0) {
        return -1;
    }

    *bg_color = picoui_arc_ld_color_to_rgb(ldArcGetBackgroundColor(ld_arc));
    *fg_color = picoui_arc_ld_color_to_rgb(ldArcGetForegroundColor(ld_arc));
    return 0;
}

unsigned int picoui_arc_get_background_color(const struct picoui_arc *arc)
{
    unsigned int bg_color = 0;
    unsigned int fg_color = 0;

    if (arc == 0) {
        return 0;
    }
    if (picoui_backend_arc_get_color((struct picoui_arc *)arc, &bg_color, &fg_color) != 0) {
        return 0;
    }
    return bg_color;
}

unsigned int picoui_arc_get_foreground_color(const struct picoui_arc *arc)
{
    unsigned int bg_color = 0;
    unsigned int fg_color = 0;

    if (arc == 0) {
        return 0;
    }
    if (picoui_backend_arc_get_color((struct picoui_arc *)arc, &bg_color, &fg_color) != 0) {
        return 0;
    }
    return fg_color;
}
