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
#include "widgets/progress_bar.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldProgressBar.h"

#include <string.h>



static void tinyui_progress_bar_rollback(struct tinyui_progress_bar *bar)
{
    if (bar == 0) {
        return;
    }
    if (bar->widget.ld_widget != 0) {
        tinyui_widget_destroy_common(&bar->widget);
    } else {
        ldFree(bar);
    }
}

void tinyui_progress_bar_test_destroy(struct tinyui_progress_bar *bar)
{
    tinyui_progress_bar_rollback(bar);
}

static int tinyui_progress_bar_props_are_valid(const struct tinyui_progress_bar_props *props)
{
    return props != 0
        && props->id != 0
        && props->percent >= 0
        && props->percent <= 100;
}

static void *tinyui_progress_bar_ld_init(void *ctx,
                                         struct ld_scene_t *scene,
                                         uint16_t name_id,
                                         uint16_t parent_name_id)
{
    (void)ctx;
    return ldProgressBar_init(scene, NULL, name_id, parent_name_id, 0, 0, 220, 24);
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
        tinyui_progress_bar_rollback(bar);
        return 0;
    }
    if (props->style_class != 0
        && tinyui_widget_set_style_class(&bar->widget, props->style_class) != 0) {
        tinyui_progress_bar_rollback(bar);
        return 0;
    }
    if (tinyui_progress_bar_set_percent(bar, props->percent) != 0
        || tinyui_progress_bar_set_horizontal(bar, props->horizontal) != 0
        || fail_before_inverted != 0
        || tinyui_progress_bar_set_inverted(bar, props->inverted) != 0) {
        tinyui_progress_bar_rollback(bar);
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

    if (parent == 0 || id == 0) {
        return 0;
    }
    bar = (struct tinyui_progress_bar *)tinyui_widget_create_leaf(&parent->widget,
                                                                  TINYUI_BACKEND_WIDGET_PROGRESS_BAR,
                                                                  tinyui_progress_bar_ld_init,
                                                                  0,
                                                                  sizeof(*bar));
    if (bar == 0) {
        return 0;
    }
    bar->id = id;

    if (tinyui_progress_bar_set_percent(bar, 0) != 0
        || tinyui_progress_bar_set_horizontal(bar, 0) != 0
        || tinyui_progress_bar_set_color(bar, 0xDDE2EAU, 0x2057C4U) != 0
        || tinyui_progress_bar_set_frame_color(bar, 0x586277U, 1) != 0
        || tinyui_progress_bar_set_inverted(bar, 0) != 0) {
        tinyui_progress_bar_rollback(bar);
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
    if (bar == 0 || percent < 0 || percent > 100
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ldProgressBarSetPercent((ldProgressBar_t *)bar->widget.ld_widget, (float)percent);
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
    if (bar == 0 || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    return (int)(((ldProgressBar_t *)bar->widget.ld_widget)->permille / 10U);
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
    if (bar == 0 || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ldProgressBarSetHorizontal((ldProgressBar_t *)bar->widget.ld_widget, horizontal != 0);
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
    if (bar == 0 || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    return ((ldProgressBar_t *)bar->widget.ld_widget)->isHorizontal ? 1 : 0;
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

    if (bar == 0 || source == 0 || source->img_tile == 0
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ld_progress_bar = (ldProgressBar_t *)bar->widget.ld_widget;
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

    if (bar == 0 || source == 0 || source->img_tile == 0
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ld_progress_bar = (ldProgressBar_t *)bar->widget.ld_widget;
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
    if (bar == 0 || source == 0 || source->img_tile == 0
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ldProgressBarSetFrameImage((ldProgressBar_t *)bar->widget.ld_widget,
                               source->img_tile,
                               source->mask_tile);
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
    if (bar == 0 || bg_color > 0xFFFFFFU || fg_color > 0xFFFFFFU
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ldProgressBarSetColor((ldProgressBar_t *)bar->widget.ld_widget,
                          (ldColor)tinyui_rgb_to_ld_color(bg_color),
                          (ldColor)tinyui_rgb_to_ld_color(fg_color));
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
    if (bar == 0 || frame_color > 0xFFFFFFU || frame_color_size < 0 || frame_color_size > 255
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ldProgressBarSetFrameColor((ldProgressBar_t *)bar->widget.ld_widget,
                               (ldColor)tinyui_rgb_to_ld_color(frame_color),
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
    if (bar == 0 || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ldProgressBarSetInverted((ldProgressBar_t *)bar->widget.ld_widget, inverted != 0);
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
    if (bar == 0 || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    return ((ldProgressBar_t *)bar->widget.ld_widget)->isInverted ? 1 : 0;
}
