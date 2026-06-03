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
#include "ldWindow.h"

#include <stdlib.h>

#define PICOUI_RUNTIME_ROOT_WIDTH 480
#define PICOUI_RUNTIME_ROOT_HEIGHT 320

struct picoui_backend_window_host {
    struct picoui_backend_widget widget;
    ldPadding_t padding_group;
    int has_padding_group;
};

static ldColor picoui_backend_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static unsigned int picoui_backend_ld_color_to_rgb(ldColor color)
{
    uint32_t red = ((uint32_t)color >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)color >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)color & 0x1FU;

    red = (red * 255U) / 31U;
    green = (green * 255U) / 63U;
    blue = (blue * 255U) / 31U;
    return (red << 16) | (green << 8) | blue;
}

static struct picoui_backend_app_state *picoui_backend_window_get_app_state(struct picoui_app *app)
{
    if (app == NULL || app->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)app->backend_app;
}

static void *picoui_backend_create_root_widget(struct picoui_app *app,
                                               const char *id,
                                               enum picoui_backend_widget_kind kind)
{
    struct picoui_backend_window_host *host;
    struct picoui_backend_app_state *app_state;
    ldWindow_t *ld_root;

    if (app == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_window_get_app_state(app);
    if (app_state == NULL || app_state->ld_scene == NULL) {
        return 0;
    }

    host = calloc(1, sizeof(*host));
    if (host == 0) {
        return 0;
    }

    ld_root = ldWindow_init(app_state->ld_scene,
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            PICOUI_RUNTIME_ROOT_WIDTH,
                            PICOUI_RUNTIME_ROOT_HEIGHT);
    if (ld_root == NULL) {
        free(host);
        return 0;
    }

    host->widget.id = id;
    host->widget.owner = app;
    host->widget.kind = kind;
    host->widget.root = &host->widget;
    host->widget.theme = app->theme;
    host->widget.ld_widget = ld_root;
    host->widget.ld_name_id = 0;
    return &host->widget;
}

/**
 * @brief Create backend for window
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_window(struct picoui_app *app, const char *id)
{
    return picoui_backend_create_root_widget(app, id, PICOUI_BACKEND_WIDGET_WINDOW);
}

/**
 * @brief Create backend for background
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_background(struct picoui_app *app, const char *id)
{
    return picoui_backend_create_root_widget(app, id, PICOUI_BACKEND_WIDGET_BACKGROUND);
}

static struct picoui_backend_widget *picoui_backend_window_get(struct picoui_window *window)
{
    if (window == NULL) {
        return NULL;
    }
    return (struct picoui_backend_widget *)window->widget.backend_widget;
}

static struct picoui_backend_window_host *picoui_backend_window_get_host(struct picoui_window *window)
{
    return (struct picoui_backend_window_host *)picoui_backend_window_get(window);
}

static ldWindow_t *picoui_backend_window_get_ld(struct picoui_window *window)
{
    struct picoui_backend_widget *backend = picoui_backend_window_get(window);

    if (backend == NULL || backend->ld_widget == NULL) {
        return NULL;
    }
    return (ldWindow_t *)backend->ld_widget;
}

static int picoui_backend_window_set_padding_group_ptr(struct picoui_window *window,
                                                       int left,
                                                       int top,
                                                       int right,
                                                       int bottom)
{
    struct picoui_backend_window_host *host = picoui_backend_window_get_host(window);
    ldWindow_t *ld_window;

    if (host == NULL) {
        return -1;
    }

    ld_window = picoui_backend_window_get_ld(window);
    if (ld_window == NULL) {
        return -1;
    }

    if (!host->has_padding_group || ld_window->pLayoutPaddingGroup != &host->padding_group) {
        ldWindowSetPaddingGroup(ld_window, &host->padding_group);
        host->has_padding_group = 1;
    }

    host->padding_group.left = (int16_t)left;
    host->padding_group.top = (int16_t)top;
    host->padding_group.right = (int16_t)right;
    host->padding_group.bottom = (int16_t)bottom;
    return 0;
}

/**
 * @brief Set background source of window backend
 *
 * @param[in] window Window instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_background_source(struct picoui_window *window,
                                                struct picoui_image_source *source)
{
    ldWindow_t *ld_window;

    if (picoui_backend_window_get(window) == NULL) {
        return -1;
    }
    if (source != NULL && source->img_tile == NULL) {
        return -1;
    }

    ld_window = picoui_backend_window_get_ld(window);
    if (ld_window == NULL) {
        return -1;
    }

    ldWindowSetImage(ld_window,
                     source != NULL ? source->img_tile : NULL,
                     source != NULL ? source->mask_tile : NULL);
    return 0;
}

/**
 * @brief Set background offset of window backend
 *
 * @param[in] window Window instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_background_offset(struct picoui_window *window,
                                                int offset_x,
                                                int offset_y)
{
    struct picoui_backend_widget *backend = picoui_backend_window_get(window);
    struct picoui_backend_app_state *app_state;
    ldWindow_t *ld_window;
    int16_t bg_width;
    int16_t bg_height;

    if (backend == NULL || backend->owner == NULL) {
        return -1;
    }

    app_state = picoui_backend_window_get_app_state(backend->owner);
    ld_window = picoui_backend_window_get_ld(window);
    if (app_state == NULL || app_state->ld_scene == NULL || ld_window == NULL) {
        return -1;
    }

    bg_width = ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth;
    bg_height = ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight;
    if (bg_width <= 0) {
        bg_width = PICOUI_RUNTIME_ROOT_WIDTH;
    }
    if (bg_height <= 0) {
        bg_height = PICOUI_RUNTIME_ROOT_HEIGHT;
    }

    ldBaseBgMove(app_state->ld_scene, bg_width, bg_height, (int16_t)offset_x, (int16_t)offset_y);
    return 0;
}

/**
 * @brief Set bg color of window backend
 *
 * @param[in] window Window instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_bg_color(struct picoui_window *window, unsigned int rgb)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL) {
        return -1;
    }

    ldWindowSetColor(ld_window, picoui_backend_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Get bg color from window backend
 *
 * @param[out] window Window instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_get_bg_color(struct picoui_window *window, unsigned int *rgb)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || rgb == NULL) {
        return -1;
    }

    *rgb = picoui_backend_ld_color_to_rgb(ldWindowGetColor(ld_window));
    return 0;
}

/**
 * @brief Set padding group of window backend
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return -1 on failure
 */

int picoui_backend_window_set_padding_group(struct picoui_window *window,
                                            int left,
                                            int top,
                                            int right,
                                            int bottom)
{
    if (left < 0 || top < 0 || right < 0 || bottom < 0) {
        return -1;
    }
    return picoui_backend_window_set_padding_group_ptr(window, left, top, right, bottom);
}

/**
 * @brief Get padding left from window backend
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int picoui_backend_window_get_padding_left(struct picoui_window *window)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || ld_window->pLayoutPaddingGroup == NULL) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->left;
}

/**
 * @brief Get padding top from window backend
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int picoui_backend_window_get_padding_top(struct picoui_window *window)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || ld_window->pLayoutPaddingGroup == NULL) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->top;
}

/**
 * @brief Get padding right from window backend
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int picoui_backend_window_get_padding_right(struct picoui_window *window)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || ld_window->pLayoutPaddingGroup == NULL) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->right;
}

/**
 * @brief Get padding bottom from window backend
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int picoui_backend_window_get_padding_bottom(struct picoui_window *window)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || ld_window->pLayoutPaddingGroup == NULL) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->bottom;
}
