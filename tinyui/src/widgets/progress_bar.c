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
#include "picoui/progress_bar.h"
#include "../core/runtime_bridge.h"
#include "../backend/ldgui/backend.h"
#include "../../../src/gui/ldProgressBar.h"

#include <stdlib.h>
#include <string.h>

int picoui_backend_widget_unbind_host(void *backend_widget);
int picoui_backend_widget_detach_from_parent(void *backend_widget);

struct picoui_progress_bar_test_dispose_snapshot {
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

static struct picoui_progress_bar_test_dispose_snapshot
    picoui_progress_bar_last_dispose_snapshot;
static int picoui_progress_bar_last_dispose_snapshot_valid = 0;

static void picoui_progress_bar_test_reset_internal_state(void)
{
    memset(&picoui_progress_bar_last_dispose_snapshot, 0, sizeof(picoui_progress_bar_last_dispose_snapshot));
    picoui_progress_bar_last_dispose_snapshot_valid = 0;
}

static ldColor picoui_progress_bar_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static struct picoui_backend_widget *picoui_progress_bar_backend(struct picoui_progress_bar *bar)
{
    struct picoui_backend_widget *backend;

    if (bar == 0 || bar->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)bar->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_PROGRESS_BAR || backend->ld_widget == 0) {
        return 0;
    }

    return backend;
}

static ldProgressBar_t *picoui_progress_bar_get_ld(struct picoui_progress_bar *bar)
{
    struct picoui_backend_widget *backend = picoui_progress_bar_backend(bar);

    if (backend == 0) {
        return 0;
    }

    return (ldProgressBar_t *)backend->ld_widget;
}

static int picoui_progress_bar_finish_detach_after_backend_failure(
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

static void picoui_progress_bar_dispose_partial_impl(struct picoui_progress_bar *bar)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldBase_t *ld_base;
    int detach_result = 0;
    int unbind_result = 0;

    if (bar == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)bar->widget.backend_widget;
    if (backend != 0) {
        app_state = picoui_runtime_bridge_backend_state(backend->owner);
        ld_base = (ldBase_t *)backend->ld_widget;
        memset(&picoui_progress_bar_last_dispose_snapshot, 0, sizeof(picoui_progress_bar_last_dispose_snapshot));
        picoui_progress_bar_last_dispose_snapshot.kind = backend->kind;
        if (backend->parent != 0) {
            detach_result = picoui_backend_widget_detach_from_parent(backend);
            if (detach_result != 0) {
                detach_result = picoui_progress_bar_finish_detach_after_backend_failure(backend);
            }
        } else {
            picoui_progress_bar_last_dispose_snapshot.detached = 1;
        }
        unbind_result = picoui_backend_widget_unbind_host(backend);
        picoui_progress_bar_last_dispose_snapshot.detach_result = detach_result;
        picoui_progress_bar_last_dispose_snapshot.unbind_result = unbind_result;
        picoui_progress_bar_last_dispose_snapshot.cleanup_complete =
            (detach_result == 0 && unbind_result == 0);
        picoui_progress_bar_last_dispose_snapshot.cleanup_incomplete =
            (detach_result != 0 || unbind_result != 0);
        picoui_progress_bar_last_dispose_snapshot.detached = (detach_result == 0 && backend->parent == 0);
        picoui_progress_bar_last_dispose_snapshot.owner_cleared = (backend->owner == 0);
        picoui_progress_bar_last_dispose_snapshot.root_cleared = (backend->root == 0);
        picoui_progress_bar_last_dispose_snapshot.parent_cleared = (backend->parent == 0);
        picoui_progress_bar_last_dispose_snapshot.next_sibling_cleared = (backend->next_sibling == 0);
        picoui_progress_bar_last_dispose_snapshot.host_cleared = (backend->host_widget == 0);
        picoui_progress_bar_last_dispose_snapshot.event_bridge_cleared =
            (backend->ld_event_bridge_scene == 0
             && backend->ld_event_bridge_sender == 0
             && backend->ld_event_bridge_next == 0);
        picoui_progress_bar_last_dispose_snapshot.ld_pinfo_cleared =
            (ld_base == 0 || ld_base->pInfo == 0);
        picoui_progress_bar_last_dispose_snapshot_valid = 1;
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldProgressBar_depose(app_state->ld_scene, (ldProgressBar_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(bar);
}

void picoui_backend_progress_bar_test_dispose_partial(struct picoui_progress_bar *bar)
{
    picoui_progress_bar_dispose_partial_impl(bar);
}

void picoui_backend_progress_bar_test_reset_state(void)
{
    picoui_progress_bar_test_reset_internal_state();
}

int picoui_backend_progress_bar_test_take_last_dispose_snapshot(
    struct picoui_progress_bar_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || picoui_progress_bar_last_dispose_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = picoui_progress_bar_last_dispose_snapshot;
    memset(&picoui_progress_bar_last_dispose_snapshot, 0, sizeof(picoui_progress_bar_last_dispose_snapshot));
    picoui_progress_bar_last_dispose_snapshot_valid = 0;
    return 0;
}

static int picoui_progress_bar_props_are_valid(const struct picoui_progress_bar_props *props)
{
    return props != 0
        && props->id != 0
        && props->percent >= 0
        && props->percent <= 100;
}

static struct picoui_progress_bar *picoui_progress_bar_create_with_props_impl(
    struct picoui_window *parent,
    const struct picoui_progress_bar_props *props,
    int fail_before_inverted)
{
    struct picoui_progress_bar *bar;

    if (!picoui_progress_bar_props_are_valid(props)) {
        return 0;
    }

    bar = picoui_progress_bar_create(parent, props->id);
    if (bar == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&bar->widget, props->user_data) != 0) {
        picoui_progress_bar_dispose_partial_impl(bar);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&bar->widget, props->style_class) != 0) {
        picoui_progress_bar_dispose_partial_impl(bar);
        return 0;
    }
    if (picoui_progress_bar_set_percent(bar, props->percent) != 0
        || picoui_progress_bar_set_horizontal(bar, props->horizontal) != 0
        || fail_before_inverted != 0
        || picoui_progress_bar_set_inverted(bar, props->inverted) != 0) {
        picoui_progress_bar_dispose_partial_impl(bar);
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

struct picoui_progress_bar *picoui_progress_bar_create(struct picoui_window *parent, const char *id)
{
    struct picoui_progress_bar *bar;
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldProgressBar_t *ld_progress_bar;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->widget.backend_widget;
    app_state = picoui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    bar = calloc(1, sizeof(*bar));
    if (bar == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(bar);
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(bar);
        return 0;
    }

    ld_progress_bar = ldProgressBar_init(app_state->ld_scene,
                                         NULL,
                                         name_id,
                                         parent_backend->ld_name_id,
                                         0,
                                         0,
                                         220,
                                         24);
    if (ld_progress_bar == 0) {
        free(backend);
        free(bar);
        return 0;
    }

    if (picoui_backend_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_PROGRESS_BAR,
                                         id,
                                         parent_backend->theme) != 0) {
        ldProgressBar_depose(app_state->ld_scene, ld_progress_bar);
        free(backend);
        free(bar);
        return 0;
    }
    backend->ld_widget = ld_progress_bar;
    backend->ld_name_id = name_id;
    backend->value = 0;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent_backend, backend) != 0) {
        ldProgressBar_depose(app_state->ld_scene, ld_progress_bar);
        free(backend);
        free(bar);
        return 0;
    }

    bar->widget.backend_widget = backend;
    bar->id = id;
    bar->widget.visible = 1;
    bar->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(bar->widget.backend_widget, &bar->widget) != 0) {
        picoui_progress_bar_dispose_partial_impl(bar);
        return 0;
    }
    if (picoui_progress_bar_set_percent(bar, 0) != 0
        || picoui_progress_bar_set_horizontal(bar, 0) != 0
        || picoui_progress_bar_set_color(bar, 0xDDE2EAU, 0x2057C4U) != 0
        || picoui_progress_bar_set_frame_color(bar, 0x586277U, 1) != 0
        || picoui_progress_bar_set_inverted(bar, 0) != 0) {
        picoui_progress_bar_dispose_partial_impl(bar);
        return 0;
    }
    return bar;
}

/**
 * @brief progress bar init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_progress_bar *picoui_progress_bar_init(struct picoui_window *parent, const char *id)
{
    return picoui_progress_bar_create(parent, id);
}

/**
 * @brief Create progress bar widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_progress_bar *picoui_progress_bar_create_with_props(
    struct picoui_window *parent,
    const struct picoui_progress_bar_props *props)
{
    return picoui_progress_bar_create_with_props_impl(parent, props, 0);
}

struct picoui_progress_bar *picoui_backend_progress_bar_test_create_with_props_fail_before_inverted(
    struct picoui_window *parent,
    const struct picoui_progress_bar_props *props)
{
    return picoui_progress_bar_create_with_props_impl(parent, props, 1);
}

/**
 * @brief Set percent of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent)
{
    ldProgressBar_t *ld_progress_bar;
    struct picoui_backend_widget *backend;

    if (bar == 0 || percent < 0 || percent > 100) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld(bar);
    backend = picoui_progress_bar_backend(bar);
    if (ld_progress_bar == 0 || backend == 0) {
        return -1;
    }

    ldProgressBarSetPercent(ld_progress_bar, (float)percent);
    backend->value = percent;
    bar->percent = percent;
    return 0;
}

/**
 * @brief Get percent of progress bar widget
 *
 * @param[in] bar bar
 * @return -1 on failure
 */

int picoui_progress_bar_get_percent(const struct picoui_progress_bar *bar)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld((struct picoui_progress_bar *)bar);
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

int picoui_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld(bar);
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

int picoui_progress_bar_get_horizontal(const struct picoui_progress_bar *bar)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld((struct picoui_progress_bar *)bar);
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

int picoui_progress_bar_set_image(struct picoui_progress_bar *bar,
                                  struct picoui_image_source *bg_source,
                                  struct picoui_image_source *fg_source)
{
    if (picoui_progress_bar_set_bg_source(bar, bg_source) != 0) {
        return -1;
    }
    return picoui_progress_bar_set_fg_source(bar, fg_source);
}

/**
 * @brief Set bg source of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_bg_source(struct picoui_progress_bar *bar, struct picoui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld(bar);
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

int picoui_progress_bar_set_fg_source(struct picoui_progress_bar *bar, struct picoui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld(bar);
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

int picoui_progress_bar_set_frame_source(struct picoui_progress_bar *bar, struct picoui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld(bar);
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

int picoui_progress_bar_set_color(struct picoui_progress_bar *bar, unsigned int bg_color, unsigned int fg_color)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || bg_color > 0xFFFFFFU || fg_color > 0xFFFFFFU) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetColor(ld_progress_bar,
                          picoui_progress_bar_rgb_to_ld_color(bg_color),
                          picoui_progress_bar_rgb_to_ld_color(fg_color));
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

int picoui_progress_bar_set_frame_color(struct picoui_progress_bar *bar,
                                        unsigned int frame_color,
                                        int frame_color_size)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || frame_color > 0xFFFFFFU || frame_color_size < 0 || frame_color_size > 255) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld(bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    ldProgressBarSetFrameColor(ld_progress_bar,
                               picoui_progress_bar_rgb_to_ld_color(frame_color),
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

int picoui_progress_bar_set_inverted(struct picoui_progress_bar *bar, int inverted)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld(bar);
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

int picoui_progress_bar_get_inverted(const struct picoui_progress_bar *bar)
{
    ldProgressBar_t *ld_progress_bar;

    if (bar == 0) {
        return -1;
    }

    ld_progress_bar = picoui_progress_bar_get_ld((struct picoui_progress_bar *)bar);
    if (ld_progress_bar == 0) {
        return -1;
    }

    return ld_progress_bar->isInverted ? 1 : 0;
}
