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
#include "picoui/scroll_selecter.h"

#include <stdlib.h>
#include <string.h>

static int picoui_scroll_selecter_props_are_valid(const struct picoui_scroll_selecter_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->radius >= 0 &&
           props->padding >= 0;
}

/**
 * @brief Create scroll selecter widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_scroll_selecter *picoui_scroll_selecter_create(struct picoui_window *parent, const char *id)
{
    struct picoui_scroll_selecter *scroll_selecter;

    if (parent == 0 || id == 0) {
        return 0;
    }

    scroll_selecter = calloc(1, sizeof(*scroll_selecter));
    if (scroll_selecter == 0) {
        return 0;
    }

    scroll_selecter->widget.backend_widget =
        picoui_backend_create_scroll_selecter(parent->widget.backend_widget, id);
    if (scroll_selecter->widget.backend_widget == 0) {
        free(scroll_selecter);
        return 0;
    }

    scroll_selecter->id = id;
    scroll_selecter->selected_index = -1;
    scroll_selecter->edit_mode = 1;
    scroll_selecter->transparent = 1;
    scroll_selecter->speed = 1;
    scroll_selecter->widget.visible = 1;
    scroll_selecter->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(scroll_selecter->widget.backend_widget, &scroll_selecter->widget) != 0) {
        free(scroll_selecter);
        return 0;
    }
    return scroll_selecter;
}

/**
 * @brief Create scroll selecter widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_scroll_selecter *picoui_scroll_selecter_create_with_props(
    struct picoui_window *parent,
    const struct picoui_scroll_selecter_props *props
)
{
    struct picoui_scroll_selecter *scroll_selecter;

    if (!picoui_scroll_selecter_props_are_valid(props)) {
        return 0;
    }

    scroll_selecter = picoui_scroll_selecter_create(parent, props->id);
    if (scroll_selecter == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&scroll_selecter->widget, props->user_data) != 0 ||
        picoui_widget_set_bg_color(&scroll_selecter->widget, props->bg_color) != 0 ||
        picoui_widget_set_text_color(&scroll_selecter->widget, props->text_color) != 0 ||
        picoui_widget_set_border_color(&scroll_selecter->widget, props->border_color) != 0 ||
        picoui_widget_set_radius(&scroll_selecter->widget, props->radius) != 0 ||
        picoui_widget_set_padding(&scroll_selecter->widget, props->padding) != 0) {
        free(scroll_selecter);
        return 0;
    }
    if (props->style_class != 0 &&
        picoui_widget_set_style_class(&scroll_selecter->widget, props->style_class) != 0) {
        free(scroll_selecter);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        picoui_widget_set_size(&scroll_selecter->widget, props->width, props->height) != 0) {
        free(scroll_selecter);
        return 0;
    }

    return scroll_selecter;
}

/**
 * @brief Set items of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] item_ids item ids
 * @param[in] texts texts
 * @param[in] item_count item count
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_items(struct picoui_scroll_selecter *scroll_selecter,
                                     const char *const *item_ids,
                                     const char *const *texts,
                                     int item_count)
{
    int i;

    if (scroll_selecter == 0 || item_ids == 0 || texts == 0 || item_count < 0 || item_count > PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    scroll_selecter->item_count = 0;
    scroll_selecter->selected_index = -1;
    for (i = 0; i < item_count; ++i) {
        if (item_ids[i] == 0 || texts[i] == 0 || picoui_scroll_selecter_add_item(scroll_selecter, item_ids[i], texts[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief scroll selecter add item
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_add_item(struct picoui_scroll_selecter *scroll_selecter,
                                    const char *id,
                                    const char *text)
{
    int index;
    int next_count;

    if (scroll_selecter == 0 || id == 0 || text == 0 ||
        scroll_selecter->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    index = scroll_selecter->item_count;
    scroll_selecter->backend_item_ids[index] = id;
    scroll_selecter->backend_item_texts[index] = (const unsigned char *)text;
    next_count = index + 1;

    if (picoui_backend_scroll_selecter_set_items(scroll_selecter->widget.backend_widget,
                                                 scroll_selecter->backend_item_ids,
                                                 scroll_selecter->backend_item_texts,
                                                 next_count) != 0) {
        scroll_selecter->backend_item_ids[index] = 0;
        scroll_selecter->backend_item_texts[index] = 0;
        return -1;
    }

    scroll_selecter->items[index].id = id;
    scroll_selecter->items[index].text = text;
    scroll_selecter->item_count = next_count;
    return 0;
}

/**
 * @brief Set select item num of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_select_item_num(struct picoui_scroll_selecter *scroll_selecter, int index)
{
    return picoui_scroll_selecter_set_selected_index(scroll_selecter, index);
}

/**
 * @brief Set selected index of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_selected_index(struct picoui_scroll_selecter *scroll_selecter, int index)
{
    if (scroll_selecter == 0 || index < 0 || index >= scroll_selecter->item_count) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_selected_index(scroll_selecter->widget.backend_widget, index) != 0) {
        return -1;
    }
    scroll_selecter->selected_index = index;
    return 0;
}

/**
 * @brief Get select item num of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @return The property value, negative on error
 */

int picoui_scroll_selecter_get_select_item_num(const struct picoui_scroll_selecter *scroll_selecter)
{
    return picoui_scroll_selecter_get_selected_index(scroll_selecter);
}

/**
 * @brief Get selected index of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @return -1 on failure
 */

int picoui_scroll_selecter_get_selected_index(const struct picoui_scroll_selecter *scroll_selecter)
{
    int backend_selected_index;

    if (scroll_selecter == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_sync_selected_index((struct picoui_scroll_selecter *)scroll_selecter,
                                                           &backend_selected_index) == 0) {
        return backend_selected_index;
    }

    return scroll_selecter->selected_index;
}

/**
 * @brief Set text color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_text_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb)
{
    if (scroll_selecter == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_text_color(scroll_selecter->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    scroll_selecter->widget.text_color = rgb;
    return 0;
}

/**
 * @brief Set background color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_background_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb)
{
    return picoui_scroll_selecter_set_bg_color(scroll_selecter, rgb);
}

/**
 * @brief Set bg color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_bg_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb)
{
    if (scroll_selecter == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_bg_color(scroll_selecter->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    scroll_selecter->widget.bg_color = rgb;
    return 0;
}

/**
 * @brief Set indicator color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_scroll_selecter_set_indicator_color(struct picoui_scroll_selecter *scroll_selecter,
                                               unsigned int rgb)
{
    if (scroll_selecter == 0) {
        return -1;
    }

    return picoui_backend_scroll_selecter_set_indicator_color(scroll_selecter->widget.backend_widget, rgb);
}

/**
 * @brief Set background image of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_background_image(struct picoui_scroll_selecter *scroll_selecter,
                                                struct picoui_image_source *source)
{
    return picoui_scroll_selecter_set_bg_source(scroll_selecter, source);
}

/**
 * @brief Set bg source of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_bg_source(struct picoui_scroll_selecter *scroll_selecter,
                                         struct picoui_image_source *source)
{
    if (scroll_selecter == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_bg_source(scroll_selecter->widget.backend_widget, source) != 0) {
        return -1;
    }
    scroll_selecter->bg_source = source;
    scroll_selecter->transparent = 0;
    return 0;
}

/**
 * @brief Set indicator image of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_indicator_image(struct picoui_scroll_selecter *scroll_selecter,
                                               struct picoui_image_source *source)
{
    return picoui_scroll_selecter_set_indicator_source(scroll_selecter, source);
}

/**
 * @brief Set indicator source of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_indicator_source(struct picoui_scroll_selecter *scroll_selecter,
                                                struct picoui_image_source *source)
{
    if (scroll_selecter == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_indicator_source(scroll_selecter->widget.backend_widget, source) != 0) {
        return -1;
    }
    scroll_selecter->indicator_source = source;
    scroll_selecter->transparent = 0;
    return 0;
}

/**
 * @brief Set transparent of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_transparent(struct picoui_scroll_selecter *scroll_selecter, int transparent)
{
    if (scroll_selecter == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_transparent(scroll_selecter->widget.backend_widget,
                                                       transparent != 0) != 0) {
        return -1;
    }
    scroll_selecter->transparent = transparent != 0;
    return 0;
}

/**
 * @brief Set speed of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] speed speed
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_speed(struct picoui_scroll_selecter *scroll_selecter, int speed)
{
    if (scroll_selecter == 0 || speed <= 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_speed(scroll_selecter->widget.backend_widget, speed) != 0) {
        return -1;
    }
    scroll_selecter->speed = speed;
    return 0;
}

/**
 * @brief Set select text of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_select_text(struct picoui_scroll_selecter *scroll_selecter, const char *text)
{
    int index;

    if (scroll_selecter == 0 || text == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_select_text(scroll_selecter->widget.backend_widget, text) != 0) {
        return -1;
    }

    for (index = 0; index < scroll_selecter->item_count; ++index) {
        if (scroll_selecter->items[index].text != 0 &&
            strcmp(scroll_selecter->items[index].text, text) == 0) {
            scroll_selecter->selected_index = index;
            return 0;
        }
    }

    return -1;
}

/**
 * @brief Set edit mode of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] is_edit is edit
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_edit_mode(struct picoui_scroll_selecter *scroll_selecter, int is_edit)
{
    if (scroll_selecter == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_set_edit_mode(scroll_selecter->widget.backend_widget, is_edit != 0) != 0) {
        return -1;
    }
    scroll_selecter->edit_mode = is_edit != 0;
    return 0;
}

/**
 * @brief Get edit mode of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] is_edit is edit
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_get_edit_mode(const struct picoui_scroll_selecter *scroll_selecter, int *is_edit)
{
    if (scroll_selecter == 0 || is_edit == 0) {
        return -1;
    }

    if (picoui_backend_scroll_selecter_get_edit_mode((void *)scroll_selecter->widget.backend_widget, is_edit) == 0) {
        return 0;
    }

    *is_edit = scroll_selecter->edit_mode;
    return 0;
}

/**
 * @brief Get selected text of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 */

const char *picoui_scroll_selecter_get_selected_text(const struct picoui_scroll_selecter *scroll_selecter)
{
    if (scroll_selecter == 0) {
        return 0;
    }

    return picoui_backend_scroll_selecter_get_selected_text((void *)scroll_selecter->widget.backend_widget);
}

/**
 * @brief Get select text of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 */

const char *picoui_scroll_selecter_get_select_text(const struct picoui_scroll_selecter *scroll_selecter)
{
    return picoui_scroll_selecter_get_selected_text(scroll_selecter);
}
