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
#include "ldBase.h"
#include "ldIconSlider.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

static arm_2d_tile_t *const g_icon_slider_tiles[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
};

static arm_2d_tile_t *const g_icon_slider_masks[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
};

#define PICOUI_BACKEND_ICON_SLIDER_NATIVE_MAX_ITEMS 8

static struct picoui_backend_app_state *picoui_backend_icon_slider_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldIconSlider_t *picoui_backend_icon_slider_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldIconSlider_t *)widget->ld_widget;
}

static bool picoui_backend_icon_slider_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend;
    struct picoui_icon_slider *icon_slider;
    int selected_index;
    int previous_selected_index;

    (void)scene;

    if (msg.ptSender == NULL || msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return false;
    }

    icon_slider = (struct picoui_icon_slider *)backend->host_widget;
    selected_index = (int)msg.value;
    if (selected_index < 0 || selected_index >= icon_slider->item_count) {
        return false;
    }

    previous_selected_index = icon_slider->selected_index;
    if (icon_slider->widget.visible == 0 || icon_slider->widget.enabled == 0) {
        return false;
    }

    if (picoui_backend_widget_claim_focus(backend) != 0) {
        return false;
    }

    if (picoui_backend_icon_slider_set_selected_index(backend, selected_index) != 0) {
        return false;
    }

    icon_slider->selected_index = selected_index;
    backend->value = selected_index;
    if (previous_selected_index == selected_index) {
        return false;
    }
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count++;
    if (icon_slider->cb != 0) {
        icon_slider->cb(icon_slider, selected_index, icon_slider->user_data);
    }
    return false;
}

/**
 * @brief Create backend for icon slider
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] icon_width icon width
 * @param[in] icon_space icon space
 * @param[in] columns Column definitions
 * @param[in] rows Row definitions
 * @param[in] pages pages
 */

void *picoui_backend_create_icon_slider(void *parent,
                                        const char *id,
                                        int width,
                                        int height,
                                        int icon_width,
                                        int icon_space,
                                        int columns,
                                        int rows,
                                        int pages)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldIconSlider_t *ld_icon_slider;
    uint16_t name_id;

    if (parent == 0 || id == 0 || width <= 0 || height <= 0 || icon_width <= 0) {
        return 0;
    }

    if (icon_space < 0) {
        icon_space = 0;
    }
    if (columns <= 0) {
        columns = 1;
    }
    if (rows <= 0) {
        rows = 1;
    }
    if (pages <= 0) {
        pages = 1;
    }

    app_state = picoui_backend_icon_slider_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_icon_slider = ldIconSlider_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent_widget->ld_name_id,
                                       0,
                                       0,
                                       (int16_t)width,
                                       (int16_t)height,
                                       (int16_t)icon_width,
                                       (uint8_t)icon_space,
                                       (uint8_t)columns,
                                       (uint8_t)rows,
                                       (uint8_t)pages,
                                       (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_icon_slider == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_ICON_SLIDER;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_icon_slider;
    widget->ld_name_id = name_id;
    widget->value = -1;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief icon: slider add item
 *
 * @param[in] backend_widget backend widget
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_icon_slider_add_item(void *backend_widget, const char *id, const char *text)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldIconSlider_t *ld_icon_slider;
    int index;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_ICON_SLIDER ||
        widget->ld_widget == NULL ||
        id == NULL ||
        text == NULL ||
        widget->list_item_count >= PICOUI_BACKEND_ICON_SLIDER_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_icon_slider = picoui_backend_icon_slider_get_ld(backend_widget);
    if (ld_icon_slider == NULL) {
        return -1;
    }

    index = widget->list_item_count;
    ldIconSliderAddIcon(ld_icon_slider,
                        g_icon_slider_tiles[index % 4],
                        g_icon_slider_masks[index % 4],
                        (const uint8_t *)text);
    widget->list_item_ids[index] = id;
    widget->list_item_count++;
    return 0;
}

/**
 * @brief icon: slider add item with source
 *
 * @param[in] backend_widget backend widget
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_icon_slider_add_item_with_source(void *backend_widget,
                                                    const char *id,
                                                    const char *text,
                                                    struct picoui_image_source *source)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldIconSlider_t *ld_icon_slider;
    int index;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_ICON_SLIDER ||
        widget->ld_widget == NULL ||
        id == NULL ||
        text == NULL ||
        source == NULL ||
        source->img_tile == NULL ||
        source->mask_tile == NULL ||
        widget->list_item_count >= PICOUI_BACKEND_ICON_SLIDER_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_icon_slider = picoui_backend_icon_slider_get_ld(backend_widget);
    if (ld_icon_slider == NULL) {
        return -1;
    }

    index = widget->list_item_count;
    ldIconSliderAddIcon(ld_icon_slider,
                        source->img_tile,
                        source->mask_tile,
                        (const uint8_t *)text);
    widget->list_item_ids[index] = id;
    widget->list_item_count++;
    return 0;
}

/**
 * @brief icon: slider set selected index
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_icon_slider_set_selected_index(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldIconSlider_t *ld_icon_slider;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_ICON_SLIDER ||
        widget->ld_widget == NULL ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_icon_slider = picoui_backend_icon_slider_get_ld(backend_widget);
    if (ld_icon_slider == NULL) {
        return -1;
    }

    ld_icon_slider->selectIconOrPage = (uint8_t)index;
    ld_icon_slider->isWaitMove = true;
    widget->value = index;
    return 0;
}

/**
 * @brief icon: slider get selected index
 *
 * @param[in] backend_widget backend widget
 * @return -1 on failure
 */

int picoui_backend_icon_slider_get_selected_index(void *backend_widget)
{
    ldIconSlider_t *ld_icon_slider = picoui_backend_icon_slider_get_ld(backend_widget);

    if (ld_icon_slider == NULL) {
        return -1;
    }

    return (int)ld_icon_slider->selectIconOrPage;
}

/**
 * @brief icon: slider set horizontal
 *
 * @param[in] backend_widget backend widget
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_backend_icon_slider_set_horizontal(void *backend_widget, int horizontal)
{
    ldIconSlider_t *ld_icon_slider = picoui_backend_icon_slider_get_ld(backend_widget);

    if (ld_icon_slider == NULL) {
        return -1;
    }

    ldIconSliderSetHorizontalScroll(ld_icon_slider, horizontal != 0);
    return 0;
}

/**
 * @brief icon: slider get horizontal
 *
 * @param[in] backend_widget backend widget
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_backend_icon_slider_get_horizontal(void *backend_widget, int *horizontal)
{
    ldIconSlider_t *ld_icon_slider = picoui_backend_icon_slider_get_ld(backend_widget);

    if (ld_icon_slider == NULL || horizontal == NULL) {
        return -1;
    }

    *horizontal = ld_icon_slider->isHorizontalScroll ? 1 : 0;
    return 0;
}

/**
 * @brief icon: slider set speed
 *
 * @param[in] backend_widget backend widget
 * @param[in] speed speed
 * @return 0 on success, -1 on failure
 */

int picoui_backend_icon_slider_set_speed(void *backend_widget, int speed)
{
    ldIconSlider_t *ld_icon_slider = picoui_backend_icon_slider_get_ld(backend_widget);

    if (ld_icon_slider == NULL || speed <= 0) {
        return -1;
    }

    ldIconSliderSetSpeed(ld_icon_slider, (uint8_t)speed);
    return 0;
}

/**
 * @brief icon: slider bind host
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_icon_slider_bind_host(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldIconSlider_t *ld_icon_slider;

    if (backend == NULL) {
        return -1;
    }

    ld_icon_slider = picoui_backend_icon_slider_get_ld(backend_widget);
    if (ld_icon_slider == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_icon_slider, SIGNAL_CLICKED_ITEM, picoui_backend_icon_slider_native_slot)) {
        return -1;
    }
    return 0;
}
