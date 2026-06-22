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
#include "gauge.h"
#include "widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldGauge.h"

#include <stdlib.h>
int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

struct tinyui_gauge_test_dispose_snapshot {
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

static struct tinyui_gauge_test_dispose_snapshot tinyui_gauge_last_dispose_snapshot;
static int tinyui_gauge_last_dispose_snapshot_valid = 0;

static int tinyui_gauge_props_are_valid(const struct tinyui_gauge_props *props);

static ldColor tinyui_gauge_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static unsigned int tinyui_gauge_ld_color_to_rgb(ldColor color)
{
    uint32_t red = ((uint32_t)color >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)color >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)color & 0x1FU;

    red = (red << 3) | (red >> 2);
    green = (green << 2) | (green >> 4);
    blue = (blue << 3) | (blue >> 2);
    return (red << 16) | (green << 8) | blue;
}

static ldGauge_t *tinyui_gauge_get_ld(struct tinyui_gauge *gauge)
{
    if (gauge == 0 || gauge->widget.ld_widget == 0
        || gauge->widget.kind != TINYUI_BACKEND_WIDGET_GAUGE) {
        return 0;
    }
    return (ldGauge_t *)gauge->widget.ld_widget;
}

static void tinyui_gauge_dispose_partial_impl(struct tinyui_gauge *gauge)
{
    struct tinyui_app *app_state;
    ldBase_t *ld_base;
    int detach_result = 0;
    int unbind_result = 0;

    if (gauge == 0) {
        return;
    }

    if (gauge->widget.ld_widget != 0) {
        void *saved_ld_widget = gauge->widget.ld_widget;
        app_state = gauge->widget.owner != 0
            ? tinyui_runtime_bridge_backend_state(gauge->widget.owner)
            : 0;
        ld_base = (ldBase_t *)saved_ld_widget;
        memset(&tinyui_gauge_last_dispose_snapshot, 0, sizeof(tinyui_gauge_last_dispose_snapshot));
        tinyui_gauge_last_dispose_snapshot.kind = (int)gauge->widget.kind;
        detach_result = tinyui_widget_detach_from_parent(&gauge->widget);
        unbind_result = tinyui_runtime_bridge_unbind_host(&gauge->widget);
        tinyui_gauge_last_dispose_snapshot.detach_result = detach_result;
        tinyui_gauge_last_dispose_snapshot.unbind_result = unbind_result;
        tinyui_gauge_last_dispose_snapshot.cleanup_complete =
            (detach_result == 0 && unbind_result == 0);
        tinyui_gauge_last_dispose_snapshot.cleanup_incomplete =
            (detach_result != 0 || unbind_result != 0);
        tinyui_gauge_last_dispose_snapshot.detached = 1;
        tinyui_gauge_last_dispose_snapshot.owner_cleared = (gauge->widget.owner == 0);
        tinyui_gauge_last_dispose_snapshot.root_cleared = 1;
        tinyui_gauge_last_dispose_snapshot.parent_cleared = 1;
        tinyui_gauge_last_dispose_snapshot.next_sibling_cleared = 1;
        tinyui_gauge_last_dispose_snapshot.host_cleared = 1;
        tinyui_gauge_last_dispose_snapshot.event_bridge_cleared =
            (gauge->widget.ld_event_bridge_scene == 0
             && gauge->widget.ld_event_bridge_sender == 0
             && gauge->widget.ld_event_bridge_next == 0);
        tinyui_gauge_last_dispose_snapshot.ld_pinfo_cleared =
            (ld_base == 0 || ld_base->pInfo == 0);
        tinyui_gauge_last_dispose_snapshot_valid = 1;
        if (app_state != 0 && app_state->ld_scene != 0) {
            ldGauge_depose(app_state->ld_scene, (ldGauge_t *)saved_ld_widget);
        }
    }

    free(gauge);
}

int tinyui_gauge_test_take_last_dispose_snapshot(
    struct tinyui_gauge_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || tinyui_gauge_last_dispose_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = tinyui_gauge_last_dispose_snapshot;
    memset(&tinyui_gauge_last_dispose_snapshot, 0, sizeof(tinyui_gauge_last_dispose_snapshot));
    tinyui_gauge_last_dispose_snapshot_valid = 0;
    return 0;
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
        tinyui_gauge_dispose_partial_impl(gauge);
        return 0;
    }

    tinyui_gauge_dispose_partial_impl(gauge);
    return 0;
}

static int tinyui_gauge_props_are_valid(const struct tinyui_gauge_props *props)
{
    return props != 0 && props->id != 0;
}

struct tinyui_gauge *tinyui_gauge_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_gauge *gauge;
    struct tinyui_app *app_state;
    ldGauge_t *ld_gauge;
    arm_2d_tile_t *bg_img_tile;
    arm_2d_tile_t *bg_mask_tile;
    arm_2d_tile_t *pointer_img_tile;
    arm_2d_tile_t *pointer_mask_tile;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }

    app_state = parent->owner;
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    gauge = calloc(1, sizeof(*gauge));
    if (gauge == 0) {
        return 0;
    }

    bg_img_tile = malloc(sizeof(*bg_img_tile));
    if (bg_img_tile == 0) {
        free(gauge);
        return 0;
    }
    *bg_img_tile = c_tileQuaterArcGRAY8;

    bg_mask_tile = malloc(sizeof(*bg_mask_tile));
    if (bg_mask_tile == 0) {
        free(bg_img_tile);
        free(gauge);
        return 0;
    }
    *bg_mask_tile = c_tileQuaterArcMask;

    pointer_img_tile = malloc(sizeof(*pointer_img_tile));
    if (pointer_img_tile == 0) {
        free(bg_mask_tile);
        free(bg_img_tile);
        free(gauge);
        return 0;
    }
    *pointer_img_tile = c_tilePointerSecGRAY8;

    pointer_mask_tile = malloc(sizeof(*pointer_mask_tile));
    if (pointer_mask_tile == 0) {
        free(pointer_img_tile);
        free(bg_mask_tile);
        free(bg_img_tile);
        free(gauge);
        return 0;
    }
    *pointer_mask_tile = c_tilePointerSecMask;

    name_id = ++app_state->next_ld_name_id;

    ld_gauge = ldGauge_init(app_state->ld_scene,
                            0,
                            name_id,
                            parent->ld_name_id,
                            0,
                            0,
                            160,
                            160,
                            bg_img_tile,
                            bg_mask_tile,
                            0,
                            0);
    if (ld_gauge == 0) {
        free(pointer_mask_tile);
        free(pointer_img_tile);
        free(bg_mask_tile);
        free(bg_img_tile);
        free(gauge);
        return 0;
    }

    ldGaugeSetBackgroundImage(ld_gauge, bg_img_tile, bg_mask_tile, true, true);
    ldGaugeBindPointerImage(ld_gauge,
                            pointer_img_tile,
                            pointer_mask_tile,
                            (int16_t)(pointer_mask_tile->tRegion.tSize.iWidth >> 1),
                            (int16_t)(pointer_mask_tile->tRegion.tSize.iHeight),
                            true,
                            true);

    gauge->id = id;
    gauge->widget.ld_widget  = ld_gauge;
    gauge->widget.ld_name_id = name_id;
    gauge->widget.kind       = TINYUI_BACKEND_WIDGET_GAUGE;
    gauge->widget.owner      = app_state;
    gauge->widget.value      = 0;
    gauge->widget.visible    = 1;
    gauge->widget.enabled    = 1;
    ((ldBase_t *)ld_gauge)->pInfo = &gauge->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&gauge->widget, app_state);

    if (tinyui_gauge_set_angle(gauge, 0.0f) != 0
        || tinyui_gauge_set_pointer_color(gauge, 0x000000U) != 0
        || tinyui_gauge_set_auto_move(gauge, 0) != 0) {
        tinyui_gauge_dispose_partial_impl(gauge);
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
        tinyui_gauge_dispose_partial_impl(gauge);
        return 0;
    }

    return gauge;
}

int tinyui_backend_gauge_set_angle(struct tinyui_gauge *gauge, float angle)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0) {
        return -1;
    }

    ldGaugeSetAngle(ld_gauge, angle);
    gauge->widget.value = (int)angle;
    return 0;
}

int tinyui_gauge_set_angle(struct tinyui_gauge *gauge, float angle)
{
    if (gauge == 0) {
        return -1;
    }
    if (tinyui_backend_gauge_set_angle(gauge, angle) != 0) {
        return -1;
    }
    gauge->angle = angle;
    return 0;
}

int tinyui_backend_gauge_set_bg_source(struct tinyui_gauge *gauge, struct tinyui_image_source *source)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }

    ldGaugeSetBackgroundImage(ld_gauge, source->img_tile, source->mask_tile, false, false);
    return 0;
}

int tinyui_gauge_set_bg_source(struct tinyui_gauge *gauge, struct tinyui_image_source *source)
{
    if (gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }
    if (tinyui_backend_gauge_set_bg_source(gauge, source) != 0) {
        return -1;
    }
    gauge->bg_source = source;
    return 0;
}

int tinyui_gauge_set_background_image(struct tinyui_gauge *gauge, struct tinyui_image_source *source)
{
    return tinyui_gauge_set_bg_source(gauge, source);
}

int tinyui_backend_gauge_set_pointer_source(struct tinyui_gauge *gauge, struct tinyui_image_source *source)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);
    arm_2d_tile_t *mask_tile;

    if (ld_gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }

    mask_tile = (arm_2d_tile_t *)source->mask_tile;
    ldGaugeBindPointerImage(ld_gauge,
                            source->img_tile,
                            source->mask_tile,
                            (int16_t)(mask_tile->tRegion.tSize.iWidth >> 1),
                            (int16_t)(mask_tile->tRegion.tSize.iHeight),
                            false,
                            false);
    return 0;
}

int tinyui_gauge_set_pointer_source(struct tinyui_gauge *gauge, struct tinyui_image_source *source)
{
    if (gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }
    if (tinyui_backend_gauge_set_pointer_source(gauge, source) != 0) {
        return -1;
    }
    gauge->pointer_source = source;
    return 0;
}

int tinyui_backend_gauge_set_centre_offset(struct tinyui_gauge *gauge,
                                           int centre_offset_x,
                                           int centre_offset_y)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0) {
        return -1;
    }

    ld_gauge->centreOffsetX = (int16_t)centre_offset_x;
    ld_gauge->centreOffsetY = (int16_t)centre_offset_y;
    return 0;
}

int tinyui_gauge_set_centre_offset(struct tinyui_gauge *gauge, int centre_offset_x, int centre_offset_y)
{
    if (gauge == 0) {
        return -1;
    }
    if (tinyui_backend_gauge_set_centre_offset(gauge, centre_offset_x, centre_offset_y) != 0) {
        return -1;
    }
    gauge->centre_offset_x = centre_offset_x;
    gauge->centre_offset_y = centre_offset_y;
    return 0;
}

int tinyui_backend_gauge_set_trail(struct tinyui_gauge *gauge,
                                   struct tinyui_image_source *bg_trail_source,
                                   struct tinyui_image_source *pointer_trail_source)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0
        || bg_trail_source == 0
        || pointer_trail_source == 0
        || bg_trail_source->mask_tile == 0
        || pointer_trail_source->mask_tile == 0) {
        return -1;
    }

    ldGaugeSetTrail(ld_gauge,
                    bg_trail_source->mask_tile,
                    pointer_trail_source->mask_tile);
    return 0;
}

int tinyui_gauge_set_trail(struct tinyui_gauge *gauge,
                           struct tinyui_image_source *bg_trail_source,
                           struct tinyui_image_source *pointer_trail_source)
{
    if (gauge == 0
        || bg_trail_source == 0
        || pointer_trail_source == 0
        || bg_trail_source->mask_tile == 0
        || pointer_trail_source->mask_tile == 0) {
        return -1;
    }

    if (tinyui_backend_gauge_set_trail(gauge, bg_trail_source, pointer_trail_source) != 0) {
        return -1;
    }

    return 0;
}

int tinyui_backend_gauge_set_progress_bar(struct tinyui_gauge *gauge,
                                          struct tinyui_image_source *bg_progress_source,
                                          struct tinyui_image_source *pointer_progress_source)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0
        || bg_progress_source == 0
        || pointer_progress_source == 0
        || bg_progress_source->mask_tile == 0
        || pointer_progress_source->mask_tile == 0) {
        return -1;
    }

    ldGaugeSetProgressBar(ld_gauge,
                          bg_progress_source->mask_tile,
                          pointer_progress_source->mask_tile);
    return 0;
}

int tinyui_gauge_set_progress_bar(struct tinyui_gauge *gauge,
                                  struct tinyui_image_source *bg_progress_source,
                                  struct tinyui_image_source *pointer_progress_source)
{
    if (gauge == 0
        || bg_progress_source == 0
        || pointer_progress_source == 0
        || bg_progress_source->mask_tile == 0
        || pointer_progress_source->mask_tile == 0) {
        return -1;
    }

    if (tinyui_backend_gauge_set_progress_bar(gauge, bg_progress_source, pointer_progress_source) != 0) {
        return -1;
    }

    return 0;
}

int tinyui_backend_gauge_get_angle(struct tinyui_gauge *gauge, float *angle)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0 || angle == 0) {
        return -1;
    }

    *angle = (float)ld_gauge->_nowAngle_x10 / 10.0f;
    return 0;
}

float tinyui_gauge_get_angle(const struct tinyui_gauge *gauge)
{
    float angle = 0.0f;

    if (gauge == 0) {
        return 0.0f;
    }
    if (tinyui_backend_gauge_get_angle((struct tinyui_gauge *)gauge, &angle) != 0) {
        return 0.0f;
    }
    return angle;
}

int tinyui_backend_gauge_set_pointer_color(struct tinyui_gauge *gauge, unsigned int pointer_color)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0) {
        return -1;
    }

    ldGaugeSetPointerColor(ld_gauge, tinyui_gauge_rgb_to_ld_color(pointer_color));
    return 0;
}

int tinyui_gauge_set_pointer_color(struct tinyui_gauge *gauge, unsigned int pointer_color)
{
    if (gauge == 0) {
        return -1;
    }
    if (tinyui_backend_gauge_set_pointer_color(gauge, pointer_color) != 0) {
        return -1;
    }
    gauge->pointer_color = pointer_color;
    return 0;
}

int tinyui_backend_gauge_get_pointer_color(struct tinyui_gauge *gauge, unsigned int *pointer_color)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0 || pointer_color == 0) {
        return -1;
    }

    *pointer_color = tinyui_gauge_ld_color_to_rgb(ld_gauge->maskColor);
    return 0;
}

unsigned int tinyui_gauge_get_pointer_color(const struct tinyui_gauge *gauge)
{
    unsigned int pointer_color = 0;

    if (gauge == 0) {
        return 0;
    }
    if (tinyui_backend_gauge_get_pointer_color((struct tinyui_gauge *)gauge, &pointer_color) != 0) {
        return 0;
    }
    return pointer_color;
}

int tinyui_backend_gauge_set_auto_move(struct tinyui_gauge *gauge, int auto_move)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0) {
        return -1;
    }

    ldGaugeSetAutoMove(ld_gauge, auto_move != 0);
    return 0;
}

int tinyui_gauge_set_auto_move(struct tinyui_gauge *gauge, int auto_move)
{
    if (gauge == 0) {
        return -1;
    }
    if (tinyui_backend_gauge_set_auto_move(gauge, auto_move != 0) != 0) {
        return -1;
    }
    gauge->auto_move = auto_move != 0 ? 1 : 0;
    return 0;
}

int tinyui_backend_gauge_get_auto_move(struct tinyui_gauge *gauge, int *auto_move)
{
    ldGauge_t *ld_gauge = tinyui_gauge_get_ld(gauge);

    if (ld_gauge == 0 || auto_move == 0) {
        return -1;
    }

    *auto_move = ld_gauge->isAutoMove ? 1 : 0;
    return 0;
}

int tinyui_gauge_get_auto_move(const struct tinyui_gauge *gauge)
{
    int auto_move = 0;

    if (gauge == 0) {
        return -1;
    }
    if (tinyui_backend_gauge_get_auto_move((struct tinyui_gauge *)gauge, &auto_move) != 0) {
        return -1;
    }
    return auto_move;
}
