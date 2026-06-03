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
#include "picoui/canvas.h"

#include <stdlib.h>

static int picoui_canvas_is_valid(const struct picoui_canvas *canvas)
{
    return canvas != 0 && canvas->widget.backend_widget != 0;
}

static int picoui_canvas_push(struct picoui_canvas *canvas,
                              const struct picoui_canvas_command *command)
{
    if (!picoui_canvas_is_valid(canvas)
        || command == 0
        || canvas->command_count >= PICOUI_CANVAS_MAX_COMMANDS) {
        return -1;
    }

    canvas->commands[canvas->command_count++] = *command;
    return picoui_backend_canvas_sync(canvas);
}

/**
 * @brief Create canvas widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_canvas *picoui_canvas_create(struct picoui_window *parent, const char *id)
{
    struct picoui_canvas *canvas;

    if (parent == 0 || id == 0) {
        return 0;
    }

    canvas = calloc(1, sizeof(*canvas));
    if (canvas == 0) {
        return 0;
    }

    canvas->widget.backend_widget = picoui_backend_create_canvas(parent->widget.backend_widget, id);
    if (canvas->widget.backend_widget == 0) {
        free(canvas);
        return 0;
    }

    canvas->id = id;
    canvas->widget.visible = 1;
    canvas->widget.enabled = 1;
    return canvas;
}

/**
 * @brief canvas clear
 *
 * @param[in] canvas Canvas widget instance
 * @return -1 on failure
 */

int picoui_canvas_clear(struct picoui_canvas *canvas)
{
    if (!picoui_canvas_is_valid(canvas)) {
        return -1;
    }

    canvas->command_count = 0;
    return picoui_backend_canvas_sync(canvas);
}

/**
 * @brief canvas fill rect
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @param[in] opacity Opacity (0-255)
 * @return -1 on failure
 */

int picoui_canvas_fill_rect(struct picoui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            unsigned int rgb,
                            int opacity)
{
    struct picoui_canvas_command command;

    if (width < 0 || height < 0 || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_FILL_RECT,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = rgb,
        .opacity0 = opacity,
    };
    return picoui_canvas_push(canvas, &command);
}

/**
 * @brief canvas draw line
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x0 x0
 * @param[in] y0 y0
 * @param[in] x1 x1
 * @param[in] y1 y1
 * @param[in] line_size line size
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @param[in] opacity_max opacity max
 * @param[in] opacity_min opacity min
 * @return -1 on failure
 */

int picoui_canvas_draw_line(struct picoui_canvas *canvas,
                            int x0,
                            int y0,
                            int x1,
                            int y1,
                            int line_size,
                            unsigned int rgb,
                            int opacity_max,
                            int opacity_min)
{
    struct picoui_canvas_command command;

    if (line_size <= 0
        || opacity_max < 0
        || opacity_max > 255
        || opacity_min < 0
        || opacity_min > 255) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_DRAW_LINE,
        .x = x0,
        .y = y0,
        .x1 = x1,
        .y1 = y1,
        .line_size = line_size,
        .rgb0 = rgb,
        .opacity0 = opacity_max,
        .opacity1 = opacity_min,
    };
    return picoui_canvas_push(canvas, &command);
}

/**
 * @brief canvas draw image
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] source Image source
 * @param[in] mask_color mask color
 * @param[in] opacity Opacity (0-255)
 * @return -1 on failure
 */

int picoui_canvas_draw_image(struct picoui_canvas *canvas,
                             int x,
                             int y,
                             int width,
                             int height,
                             struct picoui_image_source *source,
                             unsigned int mask_color,
                             int opacity)
{
    struct picoui_canvas_command command;

    if (source == 0 || source->img_tile == 0 || width < 0 || height < 0 || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_DRAW_IMAGE,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = mask_color,
        .opacity0 = opacity,
        .source = source,
    };
    return picoui_canvas_push(canvas, &command);
}

/**
 * @brief canvas draw image scaled
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] source Image source
 * @param[in] scale Scale factor
 * @param[in] opacity Opacity (0-255)
 * @return -1 on failure
 */

int picoui_canvas_draw_image_scaled(struct picoui_canvas *canvas,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    struct picoui_image_source *source,
                                    float scale,
                                    int opacity)
{
    struct picoui_canvas_command command;

    if (source == 0 || source->img_tile == 0 || width < 0 || height < 0 || scale <= 0.0f || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_DRAW_IMAGE_SCALED,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .scale = scale,
        .opacity0 = opacity,
        .source = source,
    };
    return picoui_canvas_push(canvas, &command);
}

/**
 * @brief canvas draw text
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] text Text widget instance
 * @param[in] align align
 * @param[in] text_color Text color
 * @param[in] opacity Opacity (0-255)
 * @return -1 on failure
 */

int picoui_canvas_draw_text(struct picoui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            const char *text,
                            enum picoui_align align,
                            unsigned int text_color,
                            int opacity)
{
    struct picoui_canvas_command command;

    if (text == 0
        || width < 0
        || height < 0
        || opacity < 0
        || opacity > 255
        || (align != PICOUI_ALIGN_START && align != PICOUI_ALIGN_CENTER && align != PICOUI_ALIGN_END)) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_DRAW_TEXT,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = text_color,
        .opacity0 = opacity,
        .align = align,
        .text = text,
    };
    return picoui_canvas_push(canvas, &command);
}

/**
 * @brief Get command count of canvas widget
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_canvas_get_command_count(const struct picoui_canvas *canvas, int *count)
{
    if (!picoui_canvas_is_valid(canvas) || count == 0) {
        return -1;
    }

    *count = canvas->command_count;
    return 0;
}
