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


static struct tinyui_progress_bar *tinyui_progress_bar_as_progress_bar(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_PROGRESS_BAR)) {
        return 0;
    }
    return (struct tinyui_progress_bar *)w;
}

static const struct tinyui_progress_bar *tinyui_progress_bar_as_progress_bar_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_PROGRESS_BAR)) {
        return 0;
    }
    return (const struct tinyui_progress_bar *)w;
}



static void tinyui_progress_bar_rollback(struct tinyui_progress_bar *bar)
{
    if (bar == 0) {
        return;
    }
    if (bar->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&bar->widget);
    } else {
        ldFree(bar);
    }
}

void tinyui_progress_bar_test_destroy(struct tinyui_progress_bar *bar)
{
    tinyui_progress_bar_rollback(bar);
}

static int tinyui_progress_bar_props_are_valid(const tinyui_progress_bar_props_t *props)
{
    return props != 0
        && props->percent >= 0
        && props->percent <= 100;
}

static void *tinyui_runtime_internal_progress_bar_ld_init(void *ctx,
                                         struct ld_scene_t *scene,
                                         uint16_t name_id,
                                         uint16_t parent_name_id)
{
    (void)ctx;
    return ldProgressBar_init(scene, NULL, name_id, parent_name_id, 0, 0, 220, 24);
}

static struct tinyui_progress_bar *tinyui_progress_bar_create_with_props_impl(
    struct tinyui_window *parent,
    const tinyui_progress_bar_props_t *props,
    int fail_before_inverted)
{
    struct tinyui_progress_bar *bar;

    if (!tinyui_progress_bar_props_are_valid(props)) {
        return 0;
    }

    bar = tinyui_progress_bar_create(parent);
    if (bar == 0) {
        return 0;
    }

    if (tinyui_runtime_internal_widget_set_user_data(&bar->widget, props->user_data) != 0) {
        tinyui_progress_bar_rollback(bar);
        return 0;
    }
    if (props->style_class != 0
        && tinyui_runtime_internal_widget_set_style_class(&bar->widget, props->style_class) != 0) {
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

tinyui_obj_t *tinyui_progress_bar_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "progress_bar";
    if (parent_w == 0) { return 0; }

    struct tinyui_progress_bar *bar;

    if (parent_w == 0 || id == 0) {
        return 0;
    }
    bar = (struct tinyui_progress_bar *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                                  TINYUI_BACKEND_WIDGET_PROGRESS_BAR,
                                                                  tinyui_runtime_internal_progress_bar_ld_init,
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
    return (tinyui_obj_t *)bar;
}

/**
 * @brief Create progress bar widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_progress_bar_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_progress_bar_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_progress_bar *bar;

    if (props == 0) {
        return tinyui_progress_bar_create(parent);
    }

    obj = tinyui_progress_bar_create(parent);
    if (obj == 0) {
        return 0;
    }
    bar = (struct tinyui_progress_bar *)(void *)obj;

    if ((props->fields & TINYUI_PROGRESS_BAR_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_PROGRESS_BAR_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&bar->widget, props->user_data) != 0) {
        tinyui_progress_bar_rollback(bar);
        return 0;
    }
    }
    if ((props->fields & TINYUI_PROGRESS_BAR_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&bar->widget, props->style_class) != 0) {
        tinyui_progress_bar_rollback(bar);
        return 0;
    }
    }
    if ((props->fields & TINYUI_PROGRESS_BAR_FIELD_PERCENT) != 0) {
    if (tinyui_progress_bar_set_percent((tinyui_obj_t *)bar, props->percent) != 0) {
        tinyui_progress_bar_rollback(bar);
        return 0;
    }
    }
    if ((props->fields & TINYUI_PROGRESS_BAR_FIELD_HORIZONTAL) != 0) {
    if (tinyui_progress_bar_set_horizontal((tinyui_obj_t *)bar, props->horizontal) != 0) {
        tinyui_progress_bar_rollback(bar);
        return 0;
    }
    }
    if ((props->fields & TINYUI_PROGRESS_BAR_FIELD_INVERTED) != 0) {
    if (tinyui_progress_bar_set_inverted((tinyui_obj_t *)bar, props->inverted) != 0) {
        tinyui_progress_bar_rollback(bar);
        return 0;
    }
    }

    return obj;
}


struct tinyui_progress_bar *tinyui_progress_bar_test_create_with_props_fail_before_inverted(
    struct tinyui_window *parent,
    const tinyui_progress_bar_props_t *props)
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

int tinyui_progress_bar_set_percent(tinyui_obj_t *bar_obj, int percent)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

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

int tinyui_progress_bar_get_percent(const tinyui_obj_t *bar_obj)
{
    const struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar_const(bar_obj);
    if (bar == 0) { return -1; }

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

int tinyui_progress_bar_set_horizontal(tinyui_obj_t *bar_obj, int horizontal)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

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

int tinyui_progress_bar_get_horizontal(const tinyui_obj_t *bar_obj)
{
    const struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar_const(bar_obj);
    if (bar == 0) { return -1; }

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

int tinyui_progress_bar_set_image(tinyui_obj_t *bar_obj, struct tinyui_image_source *bg_source, struct tinyui_image_source *fg_source)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

    if (tinyui_progress_bar_set_bg_source((tinyui_obj_t *)bar, bg_source) != 0) {
        return -1;
    }
    return tinyui_progress_bar_set_fg_source((tinyui_obj_t *)bar, fg_source);
}

/**
 * @brief Set bg source of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_progress_bar_set_bg_source(tinyui_obj_t *bar_obj, struct tinyui_image_source *source)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || source == 0 || source->kind == TINYUI_IMAGE_SOURCE_EMPTY
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ld_progress_bar = (ldProgressBar_t *)bar->widget.ld_widget;
    ldProgressBarSetImage(ld_progress_bar,
                          tinyui_image_source_get_image_tile(source),
                          tinyui_image_source_get_mask_tile(source),
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

int tinyui_progress_bar_set_fg_source(tinyui_obj_t *bar_obj, struct tinyui_image_source *source)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

    ldProgressBar_t *ld_progress_bar;

    if (bar == 0 || source == 0 || source->kind == TINYUI_IMAGE_SOURCE_EMPTY
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ld_progress_bar = (ldProgressBar_t *)bar->widget.ld_widget;
    ldProgressBarSetImage(ld_progress_bar,
                          ld_progress_bar->ptBgImgTile,
                          ld_progress_bar->ptBgMaskTile,
                          tinyui_image_source_get_image_tile(source),
                          tinyui_image_source_get_mask_tile(source));
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

int tinyui_progress_bar_set_frame_source(tinyui_obj_t *bar_obj, struct tinyui_image_source *source)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

    if (bar == 0 || source == 0 || source->kind == TINYUI_IMAGE_SOURCE_EMPTY
        || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    ldProgressBarSetFrameImage((ldProgressBar_t *)bar->widget.ld_widget,
                               tinyui_image_source_get_image_tile(source),
                               tinyui_image_source_get_mask_tile(source));
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

int tinyui_progress_bar_set_color(tinyui_obj_t *bar_obj, unsigned int bg_color, unsigned int fg_color)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

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

int tinyui_progress_bar_set_frame_color(tinyui_obj_t *bar_obj, unsigned int frame_color, int frame_color_size)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

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

int tinyui_progress_bar_set_inverted(tinyui_obj_t *bar_obj, int inverted)
{
    struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar(bar_obj);
    if (bar == 0) { return -1; }

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

int tinyui_progress_bar_get_inverted(const tinyui_obj_t *bar_obj)
{
    const struct tinyui_progress_bar *bar = tinyui_progress_bar_as_progress_bar_const(bar_obj);
    if (bar == 0) { return -1; }

    if (bar == 0 || bar->widget.ld_widget == 0
        || bar->widget.kind != TINYUI_BACKEND_WIDGET_PROGRESS_BAR) {
        return -1;
    }

    return ((ldProgressBar_t *)bar->widget.ld_widget)->isInverted ? 1 : 0;
}
