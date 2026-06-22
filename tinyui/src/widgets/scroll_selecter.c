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
#include "scroll_selecter.h"
#include "../../../src/gui/ldScrollSelecter.h"
#include "../../../src/gui/ldBase.h"

#include <stdlib.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
int tinyui_runtime_bridge_bind_leaf_widget(struct tinyui_widget *widget, struct tinyui_app *app);

static ld_scene_t *s_scroll_selecter_depose_scene = NULL;

static void tinyui_scroll_selecter_ld_depose_cb(void *ld_widget)
{
    if (s_scroll_selecter_depose_scene != NULL) {
        ldScrollSelecter_depose(s_scroll_selecter_depose_scene, (ldScrollSelecter_t *)ld_widget);
        s_scroll_selecter_depose_scene = NULL;
    }
}

static void tinyui_scroll_selecter_rollback(struct tinyui_scroll_selecter *scroll_selecter)
{
    if (scroll_selecter == 0) {
        return;
    }
    if (scroll_selecter->widget.ld_widget != 0) {
        s_scroll_selecter_depose_scene = scroll_selecter->widget.owner != 0
            ? scroll_selecter->widget.owner->ld_scene
            : NULL;
        tinyui_widget_destroy_common(&scroll_selecter->widget, tinyui_scroll_selecter_ld_depose_cb);
    } else {
        free(scroll_selecter);
    }
}

static int tinyui_scroll_selecter_props_valid(const struct tinyui_scroll_selecter_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->radius >= 0 &&
           props->padding >= 0;
}

static const char *tinyui_scroll_selecter_selected_text_from_public_state(
    const struct tinyui_scroll_selecter *scroll_selecter
)
{
    int selected_index;

    if (scroll_selecter == 0) {
        return 0;
    }

    selected_index = scroll_selecter->selected_index;
    if (selected_index < 0 || selected_index >= scroll_selecter->item_count) {
        return 0;
    }

    return scroll_selecter->items[selected_index].text;
}

/**
 * @brief Create scroll selecter widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_scroll_selecter *tinyui_scroll_selecter_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_app *app_state;
    ldScrollSelecter_t *ld_scroll_selecter;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = parent->widget.owner;
    if (parent->widget.ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    scroll_selecter = calloc(1, sizeof(*scroll_selecter));
    if (scroll_selecter == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    if (name_id == 0) {
        free(scroll_selecter);
        return 0;
    }

    ld_scroll_selecter = ldScrollSelecter_init(app_state->ld_scene,
                                               NULL,
                                               name_id,
                                               parent->widget.ld_name_id,
                                               0,
                                               0,
                                               180,
                                               72,
                                               (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_scroll_selecter == 0) {
        free(scroll_selecter);
        return 0;
    }

    scroll_selecter->widget.kind = TINYUI_BACKEND_WIDGET_SCROLL_SELECTER;
    scroll_selecter->widget.ld_widget = ld_scroll_selecter;
    scroll_selecter->widget.ld_name_id = name_id;
    scroll_selecter->widget.owner = app_state;
    scroll_selecter->widget.value = -1;
    ((ldBase_t *)ld_scroll_selecter)->pInfo = &scroll_selecter->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&scroll_selecter->widget, app_state);

    scroll_selecter->id = id;
    scroll_selecter->selected_index = -1;
    scroll_selecter->edit_mode = 1;
    scroll_selecter->transparent = 1;
    scroll_selecter->speed = 1;
    scroll_selecter->widget.visible = 1;
    scroll_selecter->widget.enabled = 1;
    return scroll_selecter;
}

/**
 * @brief Create scroll selecter widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_scroll_selecter *tinyui_scroll_selecter_create_with_props(
    struct tinyui_window *parent,
    const struct tinyui_scroll_selecter_props *props
)
{
    struct tinyui_scroll_selecter *scroll_selecter;

    if (!tinyui_scroll_selecter_props_valid(props)) {
        return 0;
    }

    scroll_selecter = tinyui_scroll_selecter_create(parent, props->id);
    if (scroll_selecter == 0) {
        return 0;
    }

    if (tinyui_widget_set_user_data(&scroll_selecter->widget, props->user_data) != 0 ||
        tinyui_widget_set_bg_color(&scroll_selecter->widget, props->bg_color) != 0 ||
        tinyui_widget_set_text_color(&scroll_selecter->widget, props->text_color) != 0 ||
        tinyui_widget_set_border_color(&scroll_selecter->widget, props->border_color) != 0 ||
        tinyui_widget_set_radius(&scroll_selecter->widget, props->radius) != 0 ||
        tinyui_widget_set_padding(&scroll_selecter->widget, props->padding) != 0) {
        tinyui_scroll_selecter_rollback(scroll_selecter);
        return 0;
    }
    if (props->style_class != 0 &&
        tinyui_widget_set_style_class(&scroll_selecter->widget, props->style_class) != 0) {
        tinyui_scroll_selecter_rollback(scroll_selecter);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        tinyui_widget_set_size(&scroll_selecter->widget, props->width, props->height) != 0) {
        tinyui_scroll_selecter_rollback(scroll_selecter);
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

int tinyui_scroll_selecter_set_items(struct tinyui_scroll_selecter *scroll_selecter,
                                     const char *const *item_ids,
                                     const char *const *texts,
                                     int item_count)
{
    ldScrollSelecter_t *ld_scroll_selecter;
    int i;

    if (scroll_selecter == 0 ||
        scroll_selecter->widget.kind != TINYUI_BACKEND_WIDGET_SCROLL_SELECTER ||
        scroll_selecter->widget.ld_widget == 0 ||
        item_ids == 0 ||
        texts == 0 ||
        item_count < 0 ||
        item_count > TINYUI_LIST_MAX_ITEMS) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetItems(ld_scroll_selecter, (const uint8_t **)texts, (uint8_t)item_count);
    scroll_selecter->widget.list_item_count = (uint16_t)item_count;
    for (i = 0; i < item_count; ++i) {
        (void)item_ids[i];
        scroll_selecter->backend_item_ids[i] = item_ids[i];
        scroll_selecter->backend_item_texts[i] = (const unsigned char *)texts[i];
        scroll_selecter->items[i].id = item_ids[i];
        scroll_selecter->items[i].text = texts[i];
    }
    for (; i < TINYUI_LIST_MAX_ITEMS; ++i) {
        scroll_selecter->backend_item_ids[i] = 0;
        scroll_selecter->backend_item_texts[i] = 0;
        scroll_selecter->items[i].id = 0;
        scroll_selecter->items[i].text = 0;
    }
    scroll_selecter->item_count = item_count;
    scroll_selecter->widget.value = -1;
    if (item_count > 0) {
        ldScrollSelecterSetSelectItemNum(ld_scroll_selecter, 0);
        scroll_selecter->widget.value = 0;
        scroll_selecter->selected_index = 0;
    } else {
        scroll_selecter->selected_index = -1;
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

int tinyui_scroll_selecter_add_item(struct tinyui_scroll_selecter *scroll_selecter,
                                    const char *id,
                                    const char *text)
{
    ldScrollSelecter_t *ld_scroll_selecter;
    int index;
    int next_count;

    if (scroll_selecter == 0 || id == 0 || text == 0 ||
        scroll_selecter->widget.ld_widget == 0 ||
        scroll_selecter->item_count >= TINYUI_LIST_MAX_ITEMS) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;

    index = scroll_selecter->item_count;
    scroll_selecter->backend_item_ids[index] = id;
    scroll_selecter->backend_item_texts[index] = (const unsigned char *)text;
    next_count = index + 1;

    ldScrollSelecterSetItems(ld_scroll_selecter,
                             (const uint8_t **)scroll_selecter->backend_item_texts,
                             (uint8_t)next_count);
    scroll_selecter->widget.list_item_count = (uint16_t)next_count;

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

int tinyui_scroll_selecter_set_select_item_num(struct tinyui_scroll_selecter *scroll_selecter, int index)
{
    return tinyui_scroll_selecter_set_selected_index(scroll_selecter, index);
}

/**
 * @brief Set selected index of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_scroll_selecter_set_selected_index(struct tinyui_scroll_selecter *scroll_selecter, int index)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 ||
        scroll_selecter->widget.kind != TINYUI_BACKEND_WIDGET_SCROLL_SELECTER ||
        scroll_selecter->widget.ld_widget == 0 ||
        index < 0 ||
        index >= (int)scroll_selecter->widget.list_item_count) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetSelectItemNum(ld_scroll_selecter, (int8_t)index);
    scroll_selecter->widget.value = index;
    scroll_selecter->selected_index = index;
    return 0;
}

/**
 * @brief Get select item num of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @return The property value, negative on error
 */

int tinyui_scroll_selecter_get_select_item_num(const struct tinyui_scroll_selecter *scroll_selecter)
{
    return tinyui_scroll_selecter_get_selected_index(scroll_selecter);
}

/**
 * @brief Get selected index of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @return -1 on failure
 */

int tinyui_scroll_selecter_get_selected_index(const struct tinyui_scroll_selecter *scroll_selecter)
{
    ldScrollSelecter_t *ld_scroll_selecter;
    int selected_index;

    if (scroll_selecter == 0 ||
        scroll_selecter->widget.kind != TINYUI_BACKEND_WIDGET_SCROLL_SELECTER ||
        scroll_selecter->widget.ld_widget == 0) {
        return scroll_selecter != 0 ? scroll_selecter->selected_index : -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    if (ld_scroll_selecter->itemCount == 0) {
        return -1;
    }

    selected_index = (int)ldScrollSelecterGetSelectItemNum(ld_scroll_selecter);
    if (selected_index >= 0 && selected_index < scroll_selecter->item_count) {
        ((struct tinyui_scroll_selecter *)scroll_selecter)->selected_index = selected_index;
        return selected_index;
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

int tinyui_scroll_selecter_set_text_color(struct tinyui_scroll_selecter *scroll_selecter, unsigned int rgb)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 || scroll_selecter->widget.ld_widget == 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetTextColor(ld_scroll_selecter, (ldColor)tinyui_rgb_to_ld_color(rgb));
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

int tinyui_scroll_selecter_set_background_color(struct tinyui_scroll_selecter *scroll_selecter, unsigned int rgb)
{
    return tinyui_scroll_selecter_set_bg_color(scroll_selecter, rgb);
}

/**
 * @brief Set bg color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_scroll_selecter_set_bg_color(struct tinyui_scroll_selecter *scroll_selecter, unsigned int rgb)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 || scroll_selecter->widget.ld_widget == 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetBackgroundColor(ld_scroll_selecter, (ldColor)tinyui_rgb_to_ld_color(rgb));
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

int tinyui_scroll_selecter_set_indicator_color(struct tinyui_scroll_selecter *scroll_selecter,
                                               unsigned int rgb)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 || scroll_selecter->widget.ld_widget == 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetIndicatorColor(ld_scroll_selecter, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set background image of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_scroll_selecter_set_background_image(struct tinyui_scroll_selecter *scroll_selecter,
                                                struct tinyui_image_source *source)
{
    return tinyui_scroll_selecter_set_bg_source(scroll_selecter, source);
}

/**
 * @brief Set bg source of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_scroll_selecter_set_bg_source(struct tinyui_scroll_selecter *scroll_selecter,
                                         struct tinyui_image_source *source)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 || scroll_selecter->widget.ld_widget == 0 ||
        source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetBackgroundImage(ld_scroll_selecter, source->img_tile, source->mask_tile);
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

int tinyui_scroll_selecter_set_indicator_image(struct tinyui_scroll_selecter *scroll_selecter,
                                               struct tinyui_image_source *source)
{
    return tinyui_scroll_selecter_set_indicator_source(scroll_selecter, source);
}

/**
 * @brief Set indicator source of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_scroll_selecter_set_indicator_source(struct tinyui_scroll_selecter *scroll_selecter,
                                                struct tinyui_image_source *source)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 || scroll_selecter->widget.ld_widget == 0 ||
        source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetIndicatorImage(ld_scroll_selecter, source->img_tile, source->mask_tile);
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

int tinyui_scroll_selecter_set_transparent(struct tinyui_scroll_selecter *scroll_selecter, int transparent)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 || scroll_selecter->widget.ld_widget == 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetTransparent(ld_scroll_selecter, transparent != 0);
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

int tinyui_scroll_selecter_set_speed(struct tinyui_scroll_selecter *scroll_selecter, int speed)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 || scroll_selecter->widget.ld_widget == 0 || speed <= 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetSpeed(ld_scroll_selecter, (uint8_t)speed);
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

int tinyui_scroll_selecter_set_select_text(struct tinyui_scroll_selecter *scroll_selecter, const char *text)
{
    ldScrollSelecter_t *ld_scroll_selecter;
    int index;

    if (scroll_selecter == 0 || scroll_selecter->widget.ld_widget == 0 || text == 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetSelectText(ld_scroll_selecter, (uint8_t *)text);

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

int tinyui_scroll_selecter_set_edit_mode(struct tinyui_scroll_selecter *scroll_selecter, int is_edit)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 ||
        scroll_selecter->widget.kind != TINYUI_BACKEND_WIDGET_SCROLL_SELECTER ||
        scroll_selecter->widget.ld_widget == 0) {
        return -1;
    }

    ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
    ldScrollSelecterSetEditMode(ld_scroll_selecter, is_edit != 0);
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

int tinyui_scroll_selecter_get_edit_mode(const struct tinyui_scroll_selecter *scroll_selecter, int *is_edit)
{
    ldScrollSelecter_t *ld_scroll_selecter;

    if (scroll_selecter == 0 || is_edit == 0) {
        return -1;
    }

    if (scroll_selecter->widget.kind == TINYUI_BACKEND_WIDGET_SCROLL_SELECTER &&
        scroll_selecter->widget.ld_widget != 0) {
        ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
        *is_edit = ld_scroll_selecter->isEdit ? 1 : 0;
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

const char *tinyui_scroll_selecter_get_selected_text(const struct tinyui_scroll_selecter *scroll_selecter)
{
    ldScrollSelecter_t *ld_scroll_selecter;
    const char *selected_text;

    if (scroll_selecter == 0) {
        return 0;
    }

    if (scroll_selecter->widget.kind == TINYUI_BACKEND_WIDGET_SCROLL_SELECTER &&
        scroll_selecter->widget.ld_widget != 0) {
        ld_scroll_selecter = (ldScrollSelecter_t *)scroll_selecter->widget.ld_widget;
        if (ld_scroll_selecter->itemCount != 0) {
            selected_text = (const char *)ldScrollSelecterGetSelectText(ld_scroll_selecter);
            if (selected_text != 0) {
                return selected_text;
            }
        }
    }

    return tinyui_scroll_selecter_selected_text_from_public_state(scroll_selecter);
}

/**
 * @brief Get select text of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 */

const char *tinyui_scroll_selecter_get_select_text(const struct tinyui_scroll_selecter *scroll_selecter)
{
    return tinyui_scroll_selecter_get_selected_text(scroll_selecter);
}