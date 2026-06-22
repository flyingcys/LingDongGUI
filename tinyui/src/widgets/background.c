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
#include "background.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldWindow.h"
#include "../../../src/porting/ldConfig.h"

#include <stdlib.h>

/* Mirror the definition in window.c — background is also a root window and
 * still piggy-backs on window.c's APIs (set_color, set_background_offset,
 * set_padding ...) which dereference window->backend_host.  Once window.c
 * collapses its own wrapper (separate C2 task), this and the host calloc
 * below can both be deleted, leaving only the single calloc of struct
 * tinyui_background that already embeds struct tinyui_window. */
struct tinyui_window_backend_host {
    struct tinyui_backend_widget widget;
    ldPadding_t padding_group;
    int has_padding_group;
};

/**
 * @brief Create background widget
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_background *tinyui_background_create(struct tinyui_app *app, const char *id)
{
    struct tinyui_background *background;
    struct tinyui_window_backend_host *host;
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

    host = calloc(1, sizeof(*host));
    if (host == 0) {
        return 0;
    }

    ld_root = ldWindow_init(app_state->ld_scene, NULL, 0, 0, 0, 0, root_width, root_height);
    if (ld_root == 0) {
        free(host);
        return 0;
    }

    if (tinyui_widget_init_root(&host->widget,
                                app,
                                TINYUI_BACKEND_WIDGET_BACKGROUND,
                                id,
                                app->theme) != 0) {
        ldWindow_depose(app_state->ld_scene, ld_root);
        free(host);
        return 0;
    }
    host->widget.ld_widget  = ld_root;
    host->widget.ld_name_id = 0;

    background = calloc(1, sizeof(*background));
    if (background == 0) {
        ldWindow_depose(app_state->ld_scene, ld_root);
        free(host);
        return 0;
    }

    background->window.id = id;
    background->window.backend_host = host;
    background->window.widget.visible = 1;
    background->window.widget.enabled = 1;
    background->window.flex_flow         = TINYUI_FLEX_FLOW_ROW;
    background->window.flex_main_align   = TINYUI_ALIGN_START;
    background->window.flex_cross_align  = TINYUI_ALIGN_START;
    background->window.flex_track_align  = TINYUI_ALIGN_START;
    background->window.grid_col_align    = TINYUI_ALIGN_START;
    background->window.grid_row_align    = TINYUI_ALIGN_START;
    if (tinyui_runtime_bridge_bind_host(&host->widget,
                                        &background->window.widget) != 0) {
        free(background);
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

int tinyui_background_set_source(struct tinyui_background *background,
                                 struct tinyui_image_source *source)
{
    return tinyui_window_set_background_source((struct tinyui_window *)background, source);
}

/**
 * @brief Set color of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_background_set_color(struct tinyui_background *background, unsigned int rgb)
{
    return tinyui_window_set_color((struct tinyui_window *)background, rgb);
}

/**
 * @brief Get color of background widget
 *
 * @param[out] background Background widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int tinyui_background_get_color(struct tinyui_background *background, unsigned int *rgb)
{
    return tinyui_window_get_color((struct tinyui_window *)background, rgb);
}

/**
 * @brief Set offset of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int tinyui_background_set_offset(struct tinyui_background *background, int offset_x, int offset_y)
{
    return tinyui_window_set_background_offset((struct tinyui_window *)background, offset_x, offset_y);
}

/**
 * @brief Get offset of background widget
 *
 * @param[out] background Background widget instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return The property value, negative on error
 */

int tinyui_background_get_offset(struct tinyui_background *background,
                                 int *offset_x,
                                 int *offset_y)
{
    return tinyui_window_get_background_offset((struct tinyui_window *)background,
                                               offset_x,
                                               offset_y);
}