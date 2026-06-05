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
#include "picoui/background.h"
#include "picoui/display.h"
#include "picoui/screen.h"

#include <stdlib.h>

static void picoui_background_init_defaults(struct picoui_background *background,
                                            void *backend_widget,
                                            const char *id)
{
    background->window.id = id;
    background->window.widget.backend_widget = backend_widget;
    background->window.widget.visible = 1;
    background->window.widget.enabled = 1;
    background->window.flex_flow = PICOUI_FLEX_FLOW_ROW;
    background->window.flex_main_align = PICOUI_ALIGN_START;
    background->window.flex_cross_align = PICOUI_ALIGN_START;
    background->window.flex_track_align = PICOUI_ALIGN_START;
    background->window.grid_col_align = PICOUI_ALIGN_START;
    background->window.grid_row_align = PICOUI_ALIGN_START;
}

static struct picoui_background *picoui_background_create_from_app(struct picoui_app *app,
                                                                   const char *id)
{
    struct picoui_background *background;
    void *backend_widget;

    if (app == 0 || id == 0) {
        return 0;
    }

    backend_widget = picoui_backend_create_background(app, id);
    if (backend_widget == 0) {
        return 0;
    }

    background = calloc(1, sizeof(*background));
    if (background == 0) {
        free(backend_widget);
        return 0;
    }

    picoui_background_init_defaults(background, backend_widget, id);
    if (picoui_backend_widget_bind_host(background->window.widget.backend_widget,
                                        &background->window.widget) != 0) {
        free(background);
        return 0;
    }

    return background;
}

/**
 * @brief Create background widget
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_background *picoui_background_create(struct picoui_app *app, const char *id)
{
    return picoui_background_create_from_app(app, id);
}

struct picoui_background *picoui_background_create_root(struct picoui_screen *screen, const char *id)
{
    struct picoui_app *app;
    struct picoui_background *background;
    struct picoui_display *display;
    struct picoui_display_config config = {0};
    int width;
    int height;

    if (screen == 0 || id == 0) {
        return 0;
    }

    app = picoui_app_create();
    if (app == 0) {
        return 0;
    }

    display = picoui_display_get_default();
    if (display != 0 && picoui_display_get_size(display, &width, &height) == 0) {
        config.width = width;
        config.height = height;
        config.color_format = PICOUI_COLOR_FORMAT_RGB565;
        config.buffer_height = 0;
        if (picoui_display_set_config(app, &config) != 0) {
            return 0;
        }
    }

    background = picoui_background_create_from_app(app, id);
    if (background == 0) {
        return 0;
    }

    if (picoui_screen_set_root_window(screen, (struct picoui_window *)background) != 0) {
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

int picoui_background_set_source(struct picoui_background *background,
                                 struct picoui_image_source *source)
{
    return picoui_window_set_background_source((struct picoui_window *)background, source);
}

/**
 * @brief Set color of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_background_set_color(struct picoui_background *background, unsigned int rgb)
{
    return picoui_window_set_color((struct picoui_window *)background, rgb);
}

/**
 * @brief Get color of background widget
 *
 * @param[out] background Background widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int picoui_background_get_color(struct picoui_background *background, unsigned int *rgb)
{
    return picoui_window_get_color((struct picoui_window *)background, rgb);
}

/**
 * @brief Set offset of background widget
 *
 * @param[in] background Background widget instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int picoui_background_set_offset(struct picoui_background *background, int offset_x, int offset_y)
{
    return picoui_window_set_background_offset((struct picoui_window *)background, offset_x, offset_y);
}

/**
 * @brief Get offset of background widget
 *
 * @param[out] background Background widget instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return The property value, negative on error
 */

int picoui_background_get_offset(struct picoui_background *background,
                                 int *offset_x,
                                 int *offset_y)
{
    return picoui_window_get_background_offset((struct picoui_window *)background,
                                               offset_x,
                                               offset_y);
}
