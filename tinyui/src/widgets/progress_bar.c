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
#include "progress_bar.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldProgressBar.h"

#include <stdlib.h>
#include <string.h>

int tinyui_runtime_bridge_unbind_host(void *backend_widget);

struct tinyui_progress_bar_test_dispose_snapshot {
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

static struct tinyui_progress_bar_test_dispose_snapshot
    tinyui_progress_bar_last_dispose_snapshot;
static int tinyui_progress_bar_last_dispose_snapshot_valid = 0;

static void tinyui_progress_bar_test_reset_internal_state(void)
{
    memset(&tinyui_progress_bar_last_dispose_snapshot, 0, sizeof(tinyui_progress_bar_last_dispose_snapshot));
    tinyui_progress_bar_last_dispose_snapshot_valid = 0;
}

static ldColor tinyui_progress_bar_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static ldProgressBar_t *tinyui_progress_bar_get_ld(struct tinyui_progress_bar *bar)
{
    if (bar == 0 || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return 0;
    }

    return (ldProgressBar_t *)bar->widget.ld_widget;
}

static void tinyui_progress_bar_dispose_partial_impl(struct tinyui_progress_bar *bar)
{
    struct tinyui_app *app_state;
    int detach_result = 0;
    int unbind_result = 0;

    if (bar == 0) {
        return;
    }

    if (bar->widget.ld_widget != 0) {
        void *saved_ld_widget = bar->widget.ld_widget;
        app_state = bar->widget.owner != 0
            ? tinyui_runtime_bridge_backend_state(bar->widget.owner)
            : 0;
        memset(&tinyui_progress_bar_last_dispose_snapshot, 0,
               sizeof(tinyui_progress_bar_last_dispose_snapshot));
        tinyui_progress_bar_last_dispose_snapshot.kind = (int)bar->widget.kind;
        detach_result = tinyui_widget_detach_from_parent(&bar->widget);
        unbind_result = tinyui_runtime_bridge_unbind_host(&bar->widget);
        tinyui_progress_bar_last_dispose_snapshot.detach_result = detach_result;
        tinyui_progress_bar_last_dispose_snapshot.unbind_result = unbind_result;
        tinyui_progress_bar_last_dispose_snapshot.cleanup_complete =
            (detach_result == 0 && unbind_result == 0);
        tinyui_progress_bar_last_dispose_snapshot.cleanup_incomplete =
            (detach_result != 0 || unbind_result != 0);
        tinyui_progress_bar_last_dispose_snapshot.detached = (detach_result == 0);
        tinyui_progress_bar_last_dispose_snapshot.owner_cleared = 1;
        tinyui_progress_bar_last_dispose_snapshot.root_cleared = 1;
        tinyui_progress_bar_last_dispose_snapshot.parent_cleared = 1;
        tinyui_progress_bar_last_dispose_snapshot.next_sibling_cleared = 1;
        tinyui_progress_bar_last_dispose_snapshot.host_cleared = 1;
        tinyui_progress_bar_last_dispose_snapshot.event_bridge_cleared = 1;
        tinyui_progress_bar_last_dispose_snapshot.ld_pinfo_cleared = 1;
        tinyui_progress_bar_last_dispose_snapshot_valid = 1;
        if (app_state != 0 && app_state->ld_scene != 0) {
            ldProgressBar_depose(app_state->ld_scene,
                                 (ldProgressBar_t *)saved_ld_widget);
        }
    }

    free(bar);
}

void tinyui_progress_bar_test_dispose_partial(struct tinyui_progress_bar *bar)
{
    tinyui_progress_bar_dispose_partial_impl(bar);
}

void tinyui_progress_bar_test_reset_state(void)
{
    tinyui_progress_bar_test_reset_internal_state();
}

int tinyui_progress_bar_test_take_last_dispose_snapshot(
    struct tinyui_progress_bar_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || tinyui_progress_bar_last_dispose_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = tinyui_progress_bar_last_dispose_snapshot;
    memset(&tinyui_progress_bar_last_dispose_snapshot, 0, sizeof(tinyui_progress_bar_last_dispose_snapshot));
    tinyui_progress_bar_last_dispose_snapshot_valid = 0;
    return 0;
}

static int tinyui_progress_bar_props_are_valid(const struct tinyui_progress_bar_props *props)
{
    return props != 0
        && props->id != 0
        && props->percent >= 0
        && props->percent <= 100;
}

static struct tinyui_progress_bar *tinyui_progress_bar_create_with_props_impl(
    struct tinyui_window *parent,
    const struct tinyui_progress_bar_props *props,
    int fail_before_inverted)
{
    struct tinyui_progress_bar *bar;

    if (!tinyui_progress_bar_props_are_valid(props)) {
        return 0;
    }

    bar = tinyui_progress_bar_create(parent, props->id);
    if (bar == 0) {
        return 0;
    }

    if (tinyui_widget_set_user_data(&bar->widget, props->user_data) != 0) {
        tinyui_progress_bar_dispose_partial_impl(bar);
        return 0;
    }
    if (props->style_class != 0
        && tinyui_widget_set_style_class(&bar->widget, props->style_class) != 0) {
        tinyui_progress_bar_dispose_partial_impl(bar);
        return 0;
    }
    if (tinyui_progress_bar_set_percent(bar, props->percent) != 0
        || tinyui_progress_bar_set_horizontal(bar, props->horizontal) != 0
        || fail_before_inverted != 0
        || tinyui_progress_bar_set_inverted(bar, props->inverted) != 0) {
        tinyui_progress_bar_dispose_partial_impl(bar);
        return 0;
    }

    return bar;
}

/**
 * @brief Create progress bar widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_progress_bar *tinyui_progress_bar_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_progress_bar *bar;
    struct tinyui_app *app_state;
    ldProgressBar_t *ld_progress_bar;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = parent->widget.owner;
    if (parent->widget.ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    bar = calloc(1, sizeof(*bar));
    if (bar == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;

    ld_progress_bar = ldProgressBar_init(app_state->ld_scene,
                                         NULL,
                                         name_id,
                                         parent->widget.ld_name_id,
                                         0,
                                         0,
                                         220,
                                         24);
    if (ld_progress_bar == 0) {
        free(bar);
        return 0;
    }

    bar->id = id;
    bar->widget.ld_widget  = ld_progress_bar;
    bar->widget.ld_name_id = name_id;
    bar->widget.kind       = TINYUI_BACKEND_WIDGET_PROGRESS_BAR;
    bar->widget.owner      = app_state;
    bar->widget.visible    = 1;
    bar->widget.enabled    = 1;
    ((ldBase_t *)ld_progress_bar)->pInfo = &bar->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&bar->widget, app_state);

    if (tinyui_progress_bar_set_percent(bar, 0) != 0
        || tinyui_progress_bar_set_horizontal(bar, 0) != 0
        || tinyui_progress_bar_set_color(bar, 0xDDE2EAU, 0x2057C4U) != 0
        || tinyui_progress_bar_set_frame_color(bar, 0x586277U, 1) != 0
        || tinyui_progress_bar_set_inverted(bar, 0) != 0) {
        tinyui_progress_bar_dispose_partial_impl(bar);
        return 0;
    }
    return bar;
}

/**
 * @brief Create progress bar widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_progress_bar *tinyui_progress_bar_create_with_props(
    struct tinyui_window *parent,
    const struct tinyui_progress_bar_props *props)
{
    return tinyui_progress_bar_create_with_props_impl(parent, props, 0);
}

struct tinyui_progress_bar *tinyui_progress_bar_test_create_with_props_fail_before_inverted(
    struct tinyui_window *parent,
    const struct tinyui_progress_bar_props *props)
{
    return tinyui_progress_bar_create_with_props_impl(parent, props, 1);
}

/**
 * @brief Set percent of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_percent(struct tinyui_progress_bar *bar, int percent)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || percent < 0 || percent > 100) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetPercent(ld_progress_bar, (float)percent);
    bar->widget.value = percent;
    bar->percent = percent;
    return 0;
}

/**
 * @brief Get percent of progress bar widget
 *
 * @param[in] bar bar
 * @return -1 on failure
 */

int tinyui_progress_bar_get_percent(const struct tinyui_progress_bar *bar)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld((struct tinyui_progress_bar *)bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    return (int)(ld_progress_bar->permille / 10U);
}

/**
 * @brief Set horizontal of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_horizontal(struct tinyui_progress_bar *bar, int horizontal)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetHorizontal(ld_progress_bar, horizontal != 0);
    bar->horizontal = horizontal != 0 ? 1 : 0;
    return 0;
}

/**
 * @brief Get horizontal of progress bar widget
 *
 * @param[in] bar bar
 * @return -1 on failure
 */

int tinyui_progress_bar_get_horizontal(const struct tinyui_progress_bar *bar)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld((struct tinyui_progress_bar *)bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    return ld_progress_bar->isHorizontal ? 1 : 0;
}

/**
 * @brief Set image of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] bg_source bg source
 * @param[in] fg_source fg source
 * @return -1 on failure
 */

int tinyui_progress_bar_set_image(struct tinyui_progress_bar *bar,
                                  struct tinyui_image_source *bg_source,
                                  struct tinyui_image_source *fg_source)
{
    if (tinyui_progress_bar_set_bg_source(bar, bg_source) != 0) {
        return -1;
    }
    return tinyui_progress_bar_set_fg_source(bar, fg_source);
}

/**
 * @brief Set bg source of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_bg_source(struct tinyui_progress_bar *bar, struct tinyui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetImage(ld_progress_bar,
                          source->img_tile,
                          source->mask_tile,
                          ld_progress_bar->ptFgImgTile,
                          ld_progress_bar->ptFgMaskTile);
    bar->bg_source = source;
    return 0;
}

/**
 * @brief Set fg source of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_fg_source(struct tinyui_progress_bar *bar, struct tinyui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetImage(ld_progress_bar,
                          ld_progress_bar->ptBgImgTile,
                          ld_progress_bar->ptBgMaskTile,
                          source->img_tile,
                          source->mask_tile);
    bar->fg_source = source;
    return 0;
}

/**
 * @brief Set frame source of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_frame_source(struct tinyui_progress_bar *bar, struct tinyui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetFrameImage(ld_progress_bar, source->img_tile, source->mask_tile);
    bar->frame_source = source;
    return 0;
}

/**
 * @brief Set color of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] bg_color Background color
 * @param[in] fg_color Foreground color
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_color(struct tinyui_progress_bar *bar, unsigned int bg_color, unsigned int fg_color)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || bg_color > 0xFFFFFFU || fg_color > 0xFFFFFFU) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetColor(ld_progress_bar,
                          tinyui_progress_bar_rgb_to_ld_color(bg_color),
                          tinyui_progress_bar_rgb_to_ld_color(fg_color));
    bar->bg_color = bg_color;
    bar->fg_color = fg_color;
    return 0;
}

/**
 * @brief Set frame color of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] frame_color frame color
 * @param[in] frame_color_size frame color size
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_frame_color(struct tinyui_progress_bar *bar,
                                        unsigned int frame_color,
                                        int frame_color_size)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || frame_color > 0xFFFFFFU || frame_color_size < 0 || frame_color_size > 255) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetFrameColor(ld_progress_bar,
                               tinyui_progress_bar_rgb_to_ld_color(frame_color),
                               (uint8_t)frame_color_size);
    bar->frame_color = frame_color;
    bar->frame_color_size = frame_color_size;
    return 0;
}

/**
 * @brief Set inverted of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] inverted inverted
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_inverted(struct tinyui_progress_bar *bar, int inverted)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetInverted(ld_progress_bar, inverted != 0);
    bar->inverted = inverted != 0 ? 1 : 0;
    return 0;
}

/**
 * @brief Get inverted of progress bar widget
 *
 * @param[in] bar bar
 * @return -1 on failure
 */

int tinyui_progress_bar_get_inverted(const struct tinyui_progress_bar *bar)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = tinyui_progress_bar_get_ld((struct tinyui_progress_bar *)bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    return ld_progress_bar->isInverted ? 1 : 0;
}
