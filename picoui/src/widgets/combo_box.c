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
#include "picoui/combo_box.h"

#include <stdlib.h>

int picoui_native_combo_box_render(const struct picoui_backend_widget *backend);
int picoui_backend_combo_box_sync_selected_index(struct picoui_combo_box *combo_box,
                                                 int *selected_index_out);
int picoui_backend_combo_box_get_open(void *backend_widget, int *is_open);

int picoui_combo_box_apply_open_state(struct picoui_combo_box *combo_box, int is_open)
{
    struct picoui_backend_widget *backend;

    if (combo_box == 0 || combo_box->widget.backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    backend->open = is_open ? 1 : 0;
    if (backend->ld_widget != 0) {
        (void)picoui_backend_combo_box_set_open(backend, is_open);
    }
    return 0;
}

int picoui_combo_box_apply_selected_index_internal(struct picoui_combo_box *combo_box,
                                                   int index,
                                                   int emit_callback)
{
    struct picoui_backend_widget *backend;

    if (combo_box == 0 || combo_box->widget.backend_widget == 0 || index < 0 || index >= combo_box->item_count) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    combo_box->selected_index = index;
    backend->value = index;
    if (backend->ld_widget != 0) {
        (void)picoui_backend_combo_box_set_selected_index(backend, index);
    }
    backend->data_model_epoch++;
    backend->last_data_source = emit_callback ? PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT
                                              : PICOUI_BACKEND_DATA_SOURCE_SETTER;
    if (emit_callback) {
        backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
        backend->dispatch_count++;
        backend->last_native_signal = PICOUI_NATIVE_SIGNAL_CLICKED_ITEM;
        backend->last_native_value = (uint64_t)index;
        picoui_combo_box_apply_open_state(combo_box, 0);
        if (combo_box->cb != 0) {
            combo_box->cb(combo_box, index, combo_box->user_data);
        }
    }
    return 0;
}

static int picoui_combo_box_props_are_valid(const struct picoui_combo_box_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

/**
 * @brief Create combo box widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_combo_box *picoui_combo_box_create(struct picoui_window *parent, const char *id)
{
    struct picoui_combo_box *combo_box;

    if (parent == 0 || id == 0) {
        return 0;
    }

    combo_box = calloc(1, sizeof(*combo_box));
    if (combo_box == 0) {
        return 0;
    }

    combo_box->widget.backend_widget =
        picoui_backend_create_combo_box(parent->widget.backend_widget, id);
    if (combo_box->widget.backend_widget == 0) {
        free(combo_box);
        return 0;
    }

    combo_box->id = id;
    combo_box->item_max = PICOUI_LIST_MAX_ITEMS;
    combo_box->selected_index = -1;
    combo_box->widget.visible = 1;
    combo_box->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(combo_box->widget.backend_widget, &combo_box->widget) != 0) {
        free(combo_box);
        return 0;
    }
    if (picoui_backend_combo_box_bind_host(combo_box->widget.backend_widget) != 0) {
        free(combo_box);
        return 0;
    }
    ((struct picoui_backend_widget *)combo_box->widget.backend_widget)->open = 0;
    return combo_box;
}

/**
 * @brief Create combo box widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_combo_box *picoui_combo_box_create_with_props(struct picoui_window *parent,
                                                            const struct picoui_combo_box_props *props)
{
    struct picoui_combo_box *combo_box;

    if (!picoui_combo_box_props_are_valid(props)) {
        return 0;
    }

    combo_box = picoui_combo_box_create(parent, props->id);
    if (combo_box == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&combo_box->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&combo_box->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&combo_box->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&combo_box->widget, props->border_color) != 0
        || picoui_widget_set_radius(&combo_box->widget, props->radius) != 0
        || picoui_widget_set_padding(&combo_box->widget, props->padding) != 0) {
        free(combo_box);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&combo_box->widget, props->style_class) != 0) {
        free(combo_box);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&combo_box->widget, props->width, props->height) != 0) {
        free(combo_box);
        return 0;
    }

    return combo_box;
}

/**
 * @brief combo box add item
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_add_item(struct picoui_combo_box *combo_box, const char *id, const char *text)
{
    int index;
    int next_count;
    struct picoui_backend_widget *backend;

    if (combo_box == 0 || id == 0 || text == 0 || combo_box->item_count >= combo_box->item_max) {
        return -1;
    }

    index = combo_box->item_count;
    combo_box->items[index].id = id;
    combo_box->items[index].text = text;
    combo_box->backend_item_ids[index] = id;
    combo_box->backend_item_texts[index] = (const unsigned char *)text;
    next_count = index + 1;
    combo_box->item_count = next_count;
    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    if (backend != 0) {
        backend->list_item_count = next_count;
        if (backend->ld_widget != 0) {
            (void)picoui_backend_combo_box_set_items(combo_box->widget.backend_widget,
                                                     combo_box->backend_item_ids,
                                                     combo_box->backend_item_texts,
                                                     next_count);
            if (combo_box->selected_index >= 0 && combo_box->selected_index < next_count) {
                (void)picoui_backend_combo_box_set_selected_index(backend, combo_box->selected_index);
            }
            (void)picoui_backend_combo_box_set_open(backend, backend->open);
        }
    }
    return 0;
}

/**
 * @brief Set static items of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] item_ids item ids
 * @param[in] texts texts
 * @param[in] item_count item count
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_static_items(struct picoui_combo_box *combo_box,
                                      const char *const *item_ids,
                                      const char *const *texts,
                                      int item_count)
{
    int i;

    if (combo_box == 0 || item_ids == 0 || texts == 0 || item_count < 0 || item_count > combo_box->item_max) {
        return -1;
    }

    combo_box->item_count = 0;
    combo_box->selected_index = -1;
    for (i = 0; i < item_count; ++i) {
        if (item_ids[i] == 0 || texts[i] == 0 || picoui_combo_box_add_item(combo_box, item_ids[i], texts[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Set select item of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_select_item(struct picoui_combo_box *combo_box, int index)
{
    return picoui_combo_box_set_selected_index(combo_box, index);
}

/**
 * @brief Set selected index of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_selected_index(struct picoui_combo_box *combo_box, int index)
{
    return picoui_combo_box_apply_selected_index_internal(combo_box, index, 0);
}

/**
 * @brief Get select item of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @return The property value, negative on error
 */

int picoui_combo_box_get_select_item(const struct picoui_combo_box *combo_box)
{
    return picoui_combo_box_get_selected_index(combo_box);
}

/**
 * @brief Get selected index of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @return -1 on failure
 */

int picoui_combo_box_get_selected_index(const struct picoui_combo_box *combo_box)
{
    int selected_index;

    if (combo_box == 0) {
        return -1;
    }

    if (combo_box->widget.backend_widget != 0) {
        if (picoui_backend_combo_box_sync_selected_index((struct picoui_combo_box *)combo_box,
                                                         &selected_index) == 0) {
            return selected_index;
        }
    }

    if (combo_box->selected_index >= 0 && combo_box->selected_index < combo_box->item_count) {
        return combo_box->selected_index;
    }

    return -1;
}

/**
 * @brief Get text of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] index Index
 */

const char *picoui_combo_box_get_text(const struct picoui_combo_box *combo_box, int index)
{
    if (combo_box == 0 || index < 0 || index >= combo_box->item_count) {
        return 0;
    }

    return combo_box->items[index].text;
}

/**
 * @brief combo box is open
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] is_open is open
 * @return -1 on failure
 */

int picoui_combo_box_is_open(const struct picoui_combo_box *combo_box, int *is_open)
{
    struct picoui_backend_widget *backend;
    int backend_open;

    if (combo_box == 0 || is_open == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }

    if (picoui_backend_combo_box_get_open(backend, &backend_open) == 0) {
        backend->open = backend_open != 0 ? 1 : 0;
    }

    *is_open = backend->open ? 1 : 0;
    return 0;
}

/**
 * @brief Set text color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_text_color(struct picoui_combo_box *combo_box, unsigned int rgb)
{
    if (combo_box == 0) {
        return -1;
    }

    if (picoui_backend_combo_box_set_text_color(combo_box->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    combo_box->widget.text_color = rgb;
    return 0;
}

/**
 * @brief Set background color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_background_color(struct picoui_combo_box *combo_box, unsigned int rgb)
{
    return picoui_combo_box_set_bg_color(combo_box, rgb);
}

/**
 * @brief Set bg color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_bg_color(struct picoui_combo_box *combo_box, unsigned int rgb)
{
    if (combo_box == 0) {
        return -1;
    }

    if (picoui_backend_combo_box_set_bg_color(combo_box->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    combo_box->widget.bg_color = rgb;
    return 0;
}

/**
 * @brief Set frame color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_frame_color(struct picoui_combo_box *combo_box, unsigned int rgb)
{
    if (combo_box == 0) {
        return -1;
    }

    if (picoui_backend_combo_box_set_frame_color(combo_box->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    combo_box->widget.border_color = rgb;
    return 0;
}

/**
 * @brief Set select color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_combo_box_set_select_color(struct picoui_combo_box *combo_box, unsigned int rgb)
{
    if (combo_box == 0) {
        return -1;
    }

    return picoui_backend_combo_box_set_select_color(combo_box->widget.backend_widget, rgb);
}

/**
 * @brief Set item max of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] item_max item max
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_item_max(struct picoui_combo_box *combo_box, int item_max)
{
    if (combo_box == 0 || item_max < combo_box->item_count || item_max <= 0 || item_max > PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (picoui_backend_combo_box_set_item_max(combo_box->widget.backend_widget, item_max) != 0) {
        return -1;
    }
    combo_box->item_max = item_max;
    return 0;
}

/**
 * @brief Set dropdown source of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_dropdown_source(struct picoui_combo_box *combo_box,
                                         struct picoui_image_source *source)
{
    if (combo_box == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_combo_box_set_dropdown_source(combo_box->widget.backend_widget, source) != 0) {
        return -1;
    }
    combo_box->dropdown_source = source;
    return 0;
}

/**
 * @brief Set dropdown image of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_dropdown_image(struct picoui_combo_box *combo_box,
                                        struct picoui_image_source *source)
{
    return picoui_combo_box_set_dropdown_source(combo_box, source);
}

/**
 * @brief Set on selected of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void picoui_combo_box_set_on_selected(struct picoui_combo_box *combo_box,
                                      void (*callback)(struct picoui_combo_box *combo_box,
                                                       int index,
                                                       void *user_data),
                                      void *user_data)
{
    if (combo_box == 0) {
        return;
    }

    combo_box->cb = callback;
    combo_box->user_data = user_data;
}
