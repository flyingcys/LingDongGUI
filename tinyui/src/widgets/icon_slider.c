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
#include "icon_slider.h"
#include "widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldIconSlider.h"
#include "../../../src/misc/ldMsg.h"

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

#define PICOUI_ICON_SLIDER_NATIVE_MAX_ITEMS 8

static ldIconSlider_t *tinyui_icon_slider_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 || widget->ld_widget == 0) {
        return 0;
    }

    return (ldIconSlider_t *)widget->ld_widget;
}

static int tinyui_icon_slider_backend_set_selected_index(void *backend_widget, int index);

static bool tinyui_icon_slider_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend;
    struct picoui_icon_slider *icon_slider;
    int selected_index;
    int previous_selected_index;

    (void)scene;

    if (msg.ptSender == 0 || msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == 0 || backend->host_widget == 0) {
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

    if (tinyui_widget_claim_backend_focus(backend) != 0) {
        return false;
    }

    if (tinyui_icon_slider_backend_set_selected_index(backend, selected_index) != 0) {
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

static int tinyui_icon_slider_backend_add_item(void *backend_widget, const char *id, const char *text)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldIconSlider_t *ld_icon_slider;
    int index;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_ICON_SLIDER ||
        widget->ld_widget == 0 ||
        id == 0 ||
        text == 0 ||
        widget->list_item_count >= PICOUI_ICON_SLIDER_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_icon_slider = tinyui_icon_slider_get_ld(backend_widget);
    if (ld_icon_slider == 0) {
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

static int tinyui_icon_slider_backend_add_item_with_source(void *backend_widget,
                                                           const char *id,
                                                           const char *text,
                                                           struct picoui_image_source *source)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldIconSlider_t *ld_icon_slider;
    int index;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_ICON_SLIDER ||
        widget->ld_widget == 0 ||
        id == 0 ||
        text == 0 ||
        source == 0 ||
        source->img_tile == 0 ||
        source->mask_tile == 0 ||
        widget->list_item_count >= PICOUI_ICON_SLIDER_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_icon_slider = tinyui_icon_slider_get_ld(backend_widget);
    if (ld_icon_slider == 0) {
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

static int tinyui_icon_slider_backend_set_selected_index(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldIconSlider_t *ld_icon_slider;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_ICON_SLIDER ||
        widget->ld_widget == 0 ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_icon_slider = tinyui_icon_slider_get_ld(backend_widget);
    if (ld_icon_slider == 0) {
        return -1;
    }

    ld_icon_slider->selectIconOrPage = (uint8_t)index;
    ld_icon_slider->isWaitMove = true;
    widget->value = index;
    return 0;
}

static int tinyui_icon_slider_backend_get_selected_index(void *backend_widget)
{
    ldIconSlider_t *ld_icon_slider = tinyui_icon_slider_get_ld(backend_widget);

    if (ld_icon_slider == 0) {
        return -1;
    }

    return (int)ld_icon_slider->selectIconOrPage;
}

static int tinyui_icon_slider_backend_set_horizontal(void *backend_widget, int horizontal)
{
    ldIconSlider_t *ld_icon_slider = tinyui_icon_slider_get_ld(backend_widget);

    if (ld_icon_slider == 0) {
        return -1;
    }

    ldIconSliderSetHorizontalScroll(ld_icon_slider, horizontal != 0);
    return 0;
}

static int tinyui_icon_slider_backend_get_horizontal(void *backend_widget, int *horizontal)
{
    ldIconSlider_t *ld_icon_slider = tinyui_icon_slider_get_ld(backend_widget);

    if (ld_icon_slider == 0 || horizontal == 0) {
        return -1;
    }

    *horizontal = ld_icon_slider->isHorizontalScroll ? 1 : 0;
    return 0;
}

static int tinyui_icon_slider_backend_set_speed(void *backend_widget, int speed)
{
    ldIconSlider_t *ld_icon_slider = tinyui_icon_slider_get_ld(backend_widget);

    if (ld_icon_slider == 0 || speed <= 0) {
        return -1;
    }

    ldIconSliderSetSpeed(ld_icon_slider, (uint8_t)speed);
    return 0;
}

static int tinyui_icon_slider_bind_host(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldIconSlider_t *ld_icon_slider;

    if (backend == 0) {
        return -1;
    }

    ld_icon_slider = tinyui_icon_slider_get_ld(backend_widget);
    if (ld_icon_slider == 0) {
        return -1;
    }

    if (!ldMsgConnect(ld_icon_slider, SIGNAL_CLICKED_ITEM, tinyui_icon_slider_native_slot)) {
        return -1;
    }
    return 0;
}

static int tinyui_icon_slider_props_are_valid(const struct picoui_icon_slider_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->icon_width >= 0 &&
           props->icon_space >= 0 &&
           props->columns >= 0 &&
           props->rows >= 0 &&
           props->pages >= 0;
}

static struct picoui_icon_slider *tinyui_icon_slider_create_with_backend_config(struct picoui_widget *parent,
                                                                                const char *id,
                                                                                int width,
                                                                                int height,
                                                                                int icon_width,
                                                                                int icon_space,
                                                                                int columns,
                                                                                int rows,
                                                                                int pages)
{
    struct picoui_icon_slider *icon_slider;
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldIconSlider_t *ld_icon_slider;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    if (width <= 0 || height <= 0 || icon_width <= 0) {
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

    parent_backend = (struct picoui_backend_widget *)parent->backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    icon_slider = calloc(1, sizeof(*icon_slider));
    if (icon_slider == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(icon_slider);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(icon_slider);
        return 0;
    }

    ld_icon_slider = ldIconSlider_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent_backend->ld_name_id,
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
    if (ld_icon_slider == 0) {
        free(backend);
        free(icon_slider);
        return 0;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_ICON_SLIDER,
                                         id,
                                         parent_backend->theme) != 0) {
        ldIconSlider_depose(app_state->ld_scene, ld_icon_slider);
        free(backend);
        free(icon_slider);
        return 0;
    }
    backend->ld_widget = ld_icon_slider;
    backend->ld_name_id = name_id;
    backend->value = -1;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldIconSlider_depose(app_state->ld_scene, ld_icon_slider);
        free(backend);
        free(icon_slider);
        return 0;
    }

    icon_slider->widget.backend_widget = backend;
    icon_slider->id = id;
    icon_slider->selected_index = -1;
    icon_slider->horizontal = 1;
    icon_slider->icon_width = icon_width;
    icon_slider->icon_space = icon_space;
    icon_slider->columns = columns;
    icon_slider->rows = rows;
    icon_slider->pages = pages;
    icon_slider->widget.visible = 1;
    icon_slider->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(icon_slider->widget.backend_widget, &icon_slider->widget) != 0 ||
        tinyui_icon_slider_bind_host(icon_slider->widget.backend_widget) != 0) {
        (void)tinyui_runtime_bridge_detach_from_parent(icon_slider->widget.backend_widget);
        ldIconSlider_depose(app_state->ld_scene, ld_icon_slider);
        free(backend);
        free(icon_slider);
        return 0;
    }

    return icon_slider;
}

/**
 * @brief Create icon slider widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_icon_slider *picoui_icon_slider_create(struct picoui_widget *parent, const char *id)
{
    return tinyui_icon_slider_create_with_backend_config(parent, id, 220, 86, 48, 4, 4, 1, 2);
}

/**
 * @brief icon slider init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_icon_slider *picoui_icon_slider_init(struct picoui_widget *parent, const char *id)
{
    return picoui_icon_slider_create(parent, id);
}

/**
 * @brief Create icon slider widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_icon_slider *picoui_icon_slider_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_icon_slider_props *props
)
{
    struct picoui_icon_slider *icon_slider;

    if (!tinyui_icon_slider_props_are_valid(props)) {
        return 0;
    }

    icon_slider = tinyui_icon_slider_create_with_backend_config(parent,
                                                                props->id,
                                                                props->width > 0 ? props->width : 220,
                                                                props->height > 0 ? props->height : 86,
                                                                props->icon_width > 0 ? props->icon_width : 48,
                                                                props->icon_space,
                                                                props->columns > 0 ? props->columns : 4,
                                                                props->rows > 0 ? props->rows : 1,
                                                                props->pages > 0 ? props->pages : 2);
    if (icon_slider == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&icon_slider->widget, props->user_data) != 0 ||
        (props->style_class != 0 &&
         picoui_widget_set_style_class(&icon_slider->widget, props->style_class) != 0) ||
        picoui_icon_slider_set_horizontal(icon_slider, props->horizontal != 0) != 0) {
        free(icon_slider);
        return 0;
    }
    return icon_slider;
}

/**
 * @brief icon slider add item
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_add_item(struct picoui_icon_slider *icon_slider, const char *id, const char *text)
{
    int index;

    if (icon_slider == 0 || id == 0 || text == 0 || icon_slider->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (tinyui_icon_slider_backend_add_item(icon_slider->widget.backend_widget, id, text) != 0) {
        return -1;
    }

    index = icon_slider->item_count++;
    icon_slider->items[index].id = id;
    icon_slider->items[index].text = text;
    return 0;
}

/**
 * @brief icon slider add item with source
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_add_item_with_source(struct picoui_icon_slider *icon_slider,
                                            const char *id,
                                            const char *text,
                                            struct picoui_image_source *source)
{
    int index;

    if (icon_slider == 0
        || id == 0
        || text == 0
        || source == 0
        || source->img_tile == 0
        || source->mask_tile == 0
        || icon_slider->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (tinyui_icon_slider_backend_add_item_with_source(icon_slider->widget.backend_widget,
                                                        id,
                                                        text,
                                                        source) != 0) {
        return -1;
    }

    index = icon_slider->item_count++;
    icon_slider->items[index].id = id;
    icon_slider->items[index].text = text;
    icon_slider->item_sources[index] = source;
    return 0;
}

/**
 * @brief icon slider add icon
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_add_icon(struct picoui_icon_slider *icon_slider,
                                const char *id,
                                const char *text,
                                struct picoui_image_source *source)
{
    return picoui_icon_slider_add_item_with_source(icon_slider, id, text, source);
}

/**
 * @brief Set selected index of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_set_selected_index(struct picoui_icon_slider *icon_slider, int index)
{
    if (icon_slider == 0 || index < 0 || index >= icon_slider->item_count) {
        return -1;
    }

    if (tinyui_icon_slider_backend_set_selected_index(icon_slider->widget.backend_widget, index) != 0) {
        return -1;
    }

    icon_slider->selected_index = index;
    return 0;
}

/**
 * @brief Get selected index of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @return -1 on failure
 */

int picoui_icon_slider_get_selected_index(const struct picoui_icon_slider *icon_slider)
{
    int selected_index;

    if (icon_slider == 0) {
        return -1;
    }

    selected_index =
        tinyui_icon_slider_backend_get_selected_index((void *)icon_slider->widget.backend_widget);
    if (selected_index >= 0 && selected_index < icon_slider->item_count) {
        ((struct picoui_icon_slider *)icon_slider)->selected_index = selected_index;
        return selected_index;
    }

    return icon_slider->selected_index;
}

/**
 * @brief Set horizontal of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_set_horizontal(struct picoui_icon_slider *icon_slider, int horizontal)
{
    if (icon_slider == 0) {
        return -1;
    }

    if (tinyui_icon_slider_backend_set_horizontal(icon_slider->widget.backend_widget, horizontal != 0) != 0) {
        return -1;
    }

    icon_slider->horizontal = horizontal != 0 ? 1 : 0;
    return 0;
}

/**
 * @brief Set horizontal scroll of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_set_horizontal_scroll(struct picoui_icon_slider *icon_slider, int horizontal)
{
    return picoui_icon_slider_set_horizontal(icon_slider, horizontal);
}

/**
 * @brief Get horizontal of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_get_horizontal(const struct picoui_icon_slider *icon_slider, int *horizontal)
{
    if (icon_slider == 0 || horizontal == 0) {
        return -1;
    }

    if (tinyui_icon_slider_backend_get_horizontal((void *)icon_slider->widget.backend_widget, horizontal) == 0) {
        return 0;
    }

    *horizontal = icon_slider->horizontal;
    return 0;
}

/**
 * @brief Set speed of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] speed speed
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_set_speed(struct picoui_icon_slider *icon_slider, int speed)
{
    if (icon_slider == 0 || speed <= 0) {
        return -1;
    }

    if (tinyui_icon_slider_backend_set_speed(icon_slider->widget.backend_widget, speed) != 0) {
        return -1;
    }

    icon_slider->speed = speed;
    return 0;
}

/**
 * @brief Set on selected of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void picoui_icon_slider_set_on_selected(struct picoui_icon_slider *icon_slider,
                                        void (*callback)(struct picoui_icon_slider *icon_slider,
                                                         int index,
                                                         void *user_data),
                                        void *user_data)
{
    if (icon_slider == 0) {
        return;
    }

    icon_slider->cb = callback;
    icon_slider->user_data = user_data;
}
