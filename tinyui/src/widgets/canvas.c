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
#include "canvas.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldCanvas.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static ldColor tinyui_canvas_rgb_to_ld(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static arm_2d_align_t tinyui_canvas_align_to_ld(enum tinyui_align align)
{
    switch (align) {
    case TINYUI_ALIGN_START:
        return ARM_2D_ALIGN_LEFT;
    case TINYUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case TINYUI_ALIGN_CENTER:
    default:
        return ARM_2D_ALIGN_CENTRE;
    }
}

static int tinyui_canvas_push_native(struct tinyui_canvas *canvas,
                                     const struct tinyui_canvas_command *src)
{
    ldCanvas_t *ld_canvas;
    ldCanvasCommand_t command;

    if (canvas == 0 || src == 0 || canvas->widget.kind != TINYUI_BACKEND_WIDGET_CANVAS
        || canvas->widget.ld_widget == 0) {
        return -1;
    }

    ld_canvas = (ldCanvas_t *)canvas->widget.ld_widget;
    command = (ldCanvasCommand_t){
        .kind = (ldCanvasCommandKind_t)src->kind,
        .region = {
            .tLocation = {.iX = (int16_t)src->x, .iY = (int16_t)src->y},
            .tSize = {.iWidth = (int16_t)src->width, .iHeight = (int16_t)src->height},
        },
        .x1 = (int16_t)src->x1,
        .y1 = (int16_t)src->y1,
        .lineSize = (uint8_t)src->line_size,
        .color0 = tinyui_canvas_rgb_to_ld(src->rgb0),
        .color1 = tinyui_canvas_rgb_to_ld(src->rgb1),
        .opacity0 = (uint8_t)src->opacity0,
        .opacity1 = (uint8_t)src->opacity1,
        .scale = src->scale,
        .align = tinyui_canvas_align_to_ld(src->align),
        .pStr = (uint8_t *)src->text,
        .ptFont = (arm_2d_font_t *)(canvas->widget.font != 0 ? canvas->widget.font : (const void *)&ARM_2D_FONT_6x8),
        .ptImgTile = src->source != 0 ? src->source->img_tile : 0,
        .ptMaskTile = src->source != 0 ? src->source->mask_tile : 0,
    };

    return ldCanvasPushCommand(ld_canvas, &command);
}

static int tinyui_canvas_clear_native(struct tinyui_canvas *canvas)
{
    ldCanvas_t *ld_canvas;

    if (canvas == 0 || canvas->widget.kind != TINYUI_BACKEND_WIDGET_CANVAS
        || canvas->widget.ld_widget == 0) {
        return -1;
    }

    ld_canvas = (ldCanvas_t *)canvas->widget.ld_widget;
    ldCanvasClear(ld_canvas);
    return 0;
}

static int tinyui_canvas_is_valid(const struct tinyui_canvas *canvas)
{
    return canvas != 0 && canvas->widget.ld_widget != 0;
}

static int tinyui_canvas_push(struct tinyui_canvas *canvas,
                              const struct tinyui_canvas_command *command)
{
    if (!tinyui_canvas_is_valid(canvas)
        || command == 0
        || canvas->command_count >= TINYUI_CANVAS_MAX_COMMANDS) {
        return -1;
    }

    canvas->commands[canvas->command_count++] = *command;
    if (tinyui_canvas_push_native(canvas, command) != 0) {
        canvas->command_count--;
        return -1;
    }
    return 0;
}

/**
 * @brief Create canvas widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_canvas *tinyui_canvas_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_canvas *canvas;
    struct tinyui_app *app_state;
    ldCanvas_t *ld_canvas;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = parent->widget.owner;
    if (app_state == 0 || app_state->ld_scene == 0 || parent->widget.ld_widget == 0) {
        return 0;
    }

    canvas = calloc(1, sizeof(*canvas));
    if (canvas == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;

    ld_canvas = ldCanvas_init(app_state->ld_scene, NULL, name_id, parent->widget.ld_name_id, 0, 0, 0, 0);
    if (ld_canvas == 0) {
        free(canvas);
        return 0;
    }

    canvas->widget.kind = TINYUI_BACKEND_WIDGET_CANVAS;
    canvas->widget.owner = app_state;
    canvas->widget.ld_widget = ld_canvas;
    canvas->widget.ld_name_id = name_id;
    canvas->widget.visible = 1;
    canvas->widget.enabled = 1;
    canvas->id = id;
    ((ldBase_t *)ld_canvas)->pInfo = &canvas->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&canvas->widget, app_state);
    return canvas;
}

/**
 * @brief canvas clear
 *
 * @param[in] canvas Canvas widget instance
 * @return -1 on failure
 */

int tinyui_canvas_clear(struct tinyui_canvas *canvas)
{
    if (!tinyui_canvas_is_valid(canvas)) {
        return -1;
    }

    canvas->command_count = 0;
    return tinyui_canvas_clear_native(canvas);
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

int tinyui_canvas_fill_rect(struct tinyui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            unsigned int rgb,
                            int opacity)
{
    struct tinyui_canvas_command command;

    if (width < 0 || height < 0 || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct tinyui_canvas_command){
        .kind = TINYUI_CANVAS_COMMAND_FILL_RECT,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = rgb,
        .opacity0 = opacity,
    };
    return tinyui_canvas_push(canvas, &command);
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

int tinyui_canvas_draw_line(struct tinyui_canvas *canvas,
                            int x0,
                            int y0,
                            int x1,
                            int y1,
                            int line_size,
                            unsigned int rgb,
                            int opacity_max,
                            int opacity_min)
{
    struct tinyui_canvas_command command;

    if (line_size <= 0
        || opacity_max < 0
        || opacity_max > 255
        || opacity_min < 0
        || opacity_min > 255) {
        return -1;
    }

    command = (struct tinyui_canvas_command){
        .kind = TINYUI_CANVAS_COMMAND_DRAW_LINE,
        .x = x0,
        .y = y0,
        .x1 = x1,
        .y1 = y1,
        .line_size = line_size,
        .rgb0 = rgb,
        .opacity0 = opacity_max,
        .opacity1 = opacity_min,
    };
    return tinyui_canvas_push(canvas, &command);
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

int tinyui_canvas_draw_image(struct tinyui_canvas *canvas,
                             int x,
                             int y,
                             int width,
                             int height,
                             struct tinyui_image_source *source,
                             unsigned int mask_color,
                             int opacity)
{
    struct tinyui_canvas_command command;

    if (source == 0 || source->img_tile == 0 || width < 0 || height < 0 || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct tinyui_canvas_command){
        .kind = TINYUI_CANVAS_COMMAND_DRAW_IMAGE,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = mask_color,
        .opacity0 = opacity,
        .source = source,
    };
    return tinyui_canvas_push(canvas, &command);
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

int tinyui_canvas_draw_image_scaled(struct tinyui_canvas *canvas,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    struct tinyui_image_source *source,
                                    float scale,
                                    int opacity)
{
    struct tinyui_canvas_command command;

    if (source == 0 || source->img_tile == 0 || width < 0 || height < 0 || scale <= 0.0f || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct tinyui_canvas_command){
        .kind = TINYUI_CANVAS_COMMAND_DRAW_IMAGE_SCALED,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .scale = scale,
        .opacity0 = opacity,
        .source = source,
    };
    return tinyui_canvas_push(canvas, &command);
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

int tinyui_canvas_draw_text(struct tinyui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            const char *text,
                            enum tinyui_align align,
                            unsigned int text_color,
                            int opacity)
{
    struct tinyui_canvas_command command;

    if (text == 0
        || width < 0
        || height < 0
        || opacity < 0
        || opacity > 255
        || (align != TINYUI_ALIGN_START && align != TINYUI_ALIGN_CENTER && align != TINYUI_ALIGN_END)) {
        return -1;
    }

    command = (struct tinyui_canvas_command){
        .kind = TINYUI_CANVAS_COMMAND_DRAW_TEXT,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = text_color,
        .opacity0 = opacity,
        .align = align,
        .text = text,
    };
    return tinyui_canvas_push(canvas, &command);
}

/**
 * @brief Get command count of canvas widget
 *
 * @param[in] canvas Canvas widget instance
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int tinyui_canvas_get_command_count(const struct tinyui_canvas *canvas, int *count)
{
    if (!tinyui_canvas_is_valid(canvas) || count == 0) {
        return -1;
    }

    *count = canvas->command_count;
    return 0;
}
