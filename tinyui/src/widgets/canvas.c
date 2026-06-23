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

static int canvas_push(struct tinyui_canvas *canvas,
                       const struct tinyui_canvas_command *command)
{
    ldCanvas_t *ld_canvas;
    ldCanvasCommand_t native;
    int rc;

    if (canvas == 0
        || command == 0
        || canvas->widget.ld_widget == 0
        || canvas->widget.kind != TINYUI_BACKEND_WIDGET_CANVAS
        || canvas->command_count >= TINYUI_CANVAS_MAX_COMMANDS) {
        return -1;
    }

    ld_canvas = (ldCanvas_t *)canvas->widget.ld_widget;
    native = (ldCanvasCommand_t){
        .kind = (ldCanvasCommandKind_t)command->kind,
        .region = {
            .tLocation = {.iX = (int16_t)command->x, .iY = (int16_t)command->y},
            .tSize = {.iWidth = (int16_t)command->width, .iHeight = (int16_t)command->height},
        },
        .x1 = (int16_t)command->x1,
        .y1 = (int16_t)command->y1,
        .lineSize = (uint8_t)command->line_size,
        .color0 = (ldColor)tinyui_rgb_to_ld_color(command->rgb0),
        .color1 = (ldColor)tinyui_rgb_to_ld_color(command->rgb1),
        .opacity0 = (uint8_t)command->opacity0,
        .opacity1 = (uint8_t)command->opacity1,
        .scale = command->scale,
        .align = (arm_2d_align_t)tinyui_align_to_arm2d(command->align),
        .pStr = (uint8_t *)command->text,
        .ptFont = (arm_2d_font_t *)(canvas->widget.font != 0 ? canvas->widget.font : (const void *)&ARM_2D_FONT_6x8),
        .ptImgTile = command->source != 0 ? command->source->img_tile : 0,
        .ptMaskTile = command->source != 0 ? command->source->mask_tile : 0,
    };

    canvas->commands[canvas->command_count++] = *command;
    rc = ldCanvasPushCommand(ld_canvas, &native);
    if (rc != 0) {
        canvas->command_count--;
        return -1;
    }
    return 0;
}

static void *tinyui_canvas_ld_init(void *ctx,
                                   struct ld_scene_t *scene,
                                   uint16_t name_id,
                                   uint16_t parent_name_id)
{
    (void)ctx;
    return ldCanvas_init(scene, NULL, name_id, parent_name_id, 0, 0, 0, 0);
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

    if (parent == 0 || id == 0) {
        return 0;
    }
    canvas = (struct tinyui_canvas *)tinyui_widget_create_leaf(&parent->widget,
                                                               TINYUI_BACKEND_WIDGET_CANVAS,
                                                               tinyui_canvas_ld_init,
                                                               0,
                                                               sizeof(*canvas));
    if (canvas == 0) {
        return 0;
    }
    canvas->id = id;
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
    if (canvas == 0 || canvas->widget.ld_widget == 0
        || canvas->widget.kind != TINYUI_BACKEND_WIDGET_CANVAS) {
        return -1;
    }

    canvas->command_count = 0;
    ldCanvasClear((ldCanvas_t *)canvas->widget.ld_widget);
    return 0;
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
    return canvas_push(canvas, &command);
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
    return canvas_push(canvas, &command);
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
    return canvas_push(canvas, &command);
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
    return canvas_push(canvas, &command);
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
    return canvas_push(canvas, &command);
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
    if (canvas == 0 || canvas->widget.ld_widget == 0 || count == 0) {
        return -1;
    }

    *count = canvas->command_count;
    return 0;
}
