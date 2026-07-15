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
#include "widgets/background.h"
#include "widgets/window.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldWindow.h"
#include "../../../src/porting/ldConfig.h"

static struct tinyui_background *tinyui_background_as_background(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_BACKGROUND)) {
        return 0;
    }
    return (struct tinyui_background *)w;
}

static const struct tinyui_background *tinyui_background_as_background_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_BACKGROUND)) {
        return 0;
    }
    return (const struct tinyui_background *)w;
}

static ldWindow_t *tinyui_background_ld_of(struct tinyui_background *background)
{
    ldBase_t *ld_base;

    if (background == 0 || background->window.widget.ld_widget == 0) {
        return 0;
    }
    if (background->window.widget.kind != TINYUI_BACKEND_WIDGET_BACKGROUND) {
        return 0;
    }

    ld_base = (ldBase_t *)background->window.widget.ld_widget;
    if (ld_base->widgetType != widgetTypeBackground
        && ld_base->widgetType != widgetTypeWindow) {
        return 0;
    }
    return (ldWindow_t *)ld_base;
}

/* C3-T4: background is a single calloc of struct tinyui_background (which
 * embeds struct tinyui_window) — no separate host wrapper.  Binding state
 * is folded directly onto background->window.widget, mirroring window.c's
 * create path. */

/**
 * @brief Create background widget
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_background *tinyui_legacy_background_create(struct tinyui_app *app, const char *id)
{
    struct tinyui_background *background;
    struct tinyui_app *app_state;
    ldWindow_t *ld_root;
    int16_t root_width = LD_CFG_SCREEN_WIDTH;
    int16_t root_height = LD_CFG_SCREEN_HEIGHT;
    struct tinyui_display_config config = {0};

    if (app == 0 || id == 0) {
        return 0;
    }

    app_state = app;
    if (app_state->ld_scene == 0) {
        return 0;
    }

    if (tinyui_display_get_config(app, &config) == 0 && config.width > 0 && config.height > 0) {
        root_width  = (int16_t)config.width;
        root_height = (int16_t)config.height;
    }

    /* nameId=0 creates the LD scene root with widgetTypeBackground. */
    ld_root = ldWindow_init(app_state->ld_scene, NULL, 0, 0, 0, 0, root_width, root_height);
    if (ld_root == 0) {
        return 0;
    }

    /* C3-T4: single calloc — fold binding state directly onto the window. */
    background = ldCalloc(1, sizeof(*background));
    if (background == 0) {
        ldWindow_depose(app_state->ld_scene, ld_root);
        return 0;
    }

    background->window.id = id;
    background->window.widget.visible = 1;
    background->window.widget.enabled = 1;
    background->window.flex_flow         = TINYUI_FLEX_FLOW_ROW;
    background->window.flex_main_align   = TINYUI_ALIGN_START;
    background->window.flex_cross_align  = TINYUI_ALIGN_START;
    background->window.flex_track_align  = TINYUI_ALIGN_START;
    background->window.grid_col_align    = TINYUI_ALIGN_START;
    background->window.grid_row_align    = TINYUI_ALIGN_START;

    /* Fold the binding directly onto the widget struct (no host wrapper). */
    background->window.widget.ld_widget  = ld_root;
    background->window.widget.ld_name_id = 0;
    background->window.widget.kind       = TINYUI_BACKEND_WIDGET_BACKGROUND;
    background->window.widget.owner      = app_state;
    tinyui_runtime_internal_app_register_host(app_state, &background->window.widget);

    if (tinyui_runtime_bridge_bind_leaf_widget(&background->window.widget, app_state) != 0) {
        tinyui_runtime_internal_app_unregister_host(app_state, &background->window.widget);
        background->window.widget.ld_widget = 0;
        ldWindow_depose(app_state->ld_scene, ld_root);
        ldFree(background);
        return 0;
    }

    return background;
}

/**
 * @brief Set source of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_background_set_source(tinyui_obj_t *background_obj, struct tinyui_image_source *source)
{
    struct tinyui_background *background = tinyui_background_as_background(background_obj);
    ldWindow_t *ld_window;
    arm_2d_tile_t *img_tile = 0;
    arm_2d_tile_t *mask_tile = 0;

    if (background == 0) {
        return -1;
    }
    ld_window = tinyui_background_ld_of(background);
    if (ld_window == 0) {
        return -1;
    }

    if (source != 0) {
        img_tile = tinyui_image_source_get_image_tile(source);
        mask_tile = tinyui_image_source_get_mask_tile(source);
        if (img_tile == 0) {
            return -1;
        }
    }

    ldWindowSetImage(ld_window, img_tile, mask_tile);
    return 0;
}

/**
 * @brief Set color of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_background_set_color(tinyui_obj_t *background_obj, unsigned int rgb)
{
    struct tinyui_background *background = tinyui_background_as_background(background_obj);
    ldWindow_t *ld_window;

    if (background == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }
    ld_window = tinyui_background_ld_of(background);
    if (ld_window == 0) {
        return -1;
    }

    background->window.widget.bg_color = rgb;
    ldWindowSetColor(ld_window, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}


/**
 * @brief Get color of background widget
 *
 * @param[out] background Background widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int tinyui_background_get_color(tinyui_obj_t *background_obj, unsigned int *rgb)
{
    struct tinyui_background *background = tinyui_background_as_background(background_obj);
    ldWindow_t *ld_window;
    unsigned int color;
    uint32_t red;
    uint32_t green;
    uint32_t blue;

    if (background == 0 || rgb == 0) {
        return -1;
    }
    ld_window = tinyui_background_ld_of(background);
    if (ld_window == 0) {
        return -1;
    }

    /* Keep window-compatible RGB565→RGB888 round-trip formula. */
    color = (unsigned int)ldWindowGetColor(ld_window);
    red = (color >> 11) & 0x1FU;
    green = (color >> 5) & 0x3FU;
    blue = color & 0x1FU;
    red = (red * 255U) / 31U;
    green = (green * 255U) / 63U;
    blue = (blue * 255U) / 31U;
    *rgb = (red << 16) | (green << 8) | blue;
    return 0;
}


/**
 * @brief Set offset of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int tinyui_background_set_offset(tinyui_obj_t *background_obj, int offset_x, int offset_y)
{
    struct tinyui_background *background = tinyui_background_as_background(background_obj);
    if (background == 0 || tinyui_background_ld_of(background) == 0) {
        return -1;
    }

    background->window.background_offset_x = offset_x;
    background->window.background_offset_y = offset_y;
    return 0;
}

/**
 * @brief Get offset of background widget
 *
 * @param[out] background Background widget instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return The property value, negative on error
 */

int tinyui_background_get_offset(tinyui_obj_t *background_obj, int *offset_x, int *offset_y)
{
    struct tinyui_background *background = tinyui_background_as_background(background_obj);
    if (background == 0 || offset_x == 0 || offset_y == 0
        || tinyui_background_ld_of(background) == 0) {
        return -1;
    }

    *offset_x = background->window.background_offset_x;
    *offset_y = background->window.background_offset_y;
    return 0;
}
