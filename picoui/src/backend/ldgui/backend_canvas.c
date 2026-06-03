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

#include "backend.h"
#include "internal.h"
#include "ldCanvas.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static ldColor picoui_backend_canvas_rgb_to_ld(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static arm_2d_align_t picoui_backend_canvas_align_to_ld(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_START:
        return ARM_2D_ALIGN_LEFT;
    case PICOUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case PICOUI_ALIGN_CENTER:
    default:
        return ARM_2D_ALIGN_CENTRE;
    }
}

static struct picoui_backend_app_state *picoui_backend_canvas_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

/**
 * @brief Create backend for canvas
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_canvas(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldCanvas_t *ld_canvas;
    uint16_t name_id;

    if (parent == NULL || id == NULL) {
        return NULL;
    }

    app_state = picoui_backend_canvas_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return NULL;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == NULL) {
        return NULL;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_canvas = ldCanvas_init(app_state->ld_scene, NULL, name_id, parent_widget->ld_name_id, 0, 0, 0, 0);
    if (ld_canvas == NULL) {
        free(widget);
        return NULL;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_CANVAS;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_canvas;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldCanvas_depose(app_state->ld_scene, ld_canvas);
        free(widget);
        return NULL;
    }
    return widget;
}

/**
 * @brief canvas: sync
 *
 * @param[in] canvas Canvas widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_canvas_sync(struct picoui_canvas *canvas)
{
    struct picoui_backend_widget *backend;
    ldCanvas_t *ld_canvas;
    int i;

    if (canvas == NULL || canvas->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)canvas->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_CANVAS || backend->ld_widget == NULL) {
        return -1;
    }

    ld_canvas = (ldCanvas_t *)backend->ld_widget;
    ldCanvasClear(ld_canvas);
    for (i = 0; i < canvas->command_count; ++i) {
        const struct picoui_canvas_command *src = &canvas->commands[i];
        ldCanvasCommand_t command = {
            .kind = (ldCanvasCommandKind_t)src->kind,
            .region = {
                .tLocation = {.iX = (int16_t)src->x, .iY = (int16_t)src->y},
                .tSize = {.iWidth = (int16_t)src->width, .iHeight = (int16_t)src->height},
            },
            .x1 = (int16_t)src->x1,
            .y1 = (int16_t)src->y1,
            .lineSize = (uint8_t)src->line_size,
            .color0 = picoui_backend_canvas_rgb_to_ld(src->rgb0),
            .color1 = picoui_backend_canvas_rgb_to_ld(src->rgb1),
            .opacity0 = (uint8_t)src->opacity0,
            .opacity1 = (uint8_t)src->opacity1,
            .scale = src->scale,
            .align = picoui_backend_canvas_align_to_ld(src->align),
            .pStr = (uint8_t *)src->text,
            .ptFont = (arm_2d_font_t *)(canvas->widget.font != NULL ? canvas->widget.font : (const void *)&ARM_2D_FONT_6x8),
            .ptImgTile = src->source != NULL ? src->source->img_tile : NULL,
            .ptMaskTile = src->source != NULL ? src->source->mask_tile : NULL,
        };

        if (ldCanvasPushCommand(ld_canvas, &command) != 0) {
            ldCanvasClear(ld_canvas);
            return -1;
        }
    }

    return 0;
}
