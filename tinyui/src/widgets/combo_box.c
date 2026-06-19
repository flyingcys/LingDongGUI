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
#include "combo_box.h"

#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldComboBox.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static ldColor tinyui_combo_box_rgb_to_ld_color(unsigned int rgb)
{
    unsigned int red = (rgb >> 16) & 0xFFU;
    unsigned int green = (rgb >> 8) & 0xFFU;
    unsigned int blue = rgb & 0xFFU;

    return (ldColor)(((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3));
}

static ldComboBox_t *tinyui_combo_box_get_ld(void *backend_widget)
{
    struct tinyui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldComboBox_t *)widget->ld_widget;
}

static bool tinyui_combo_box_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct tinyui_backend_widget *backend;
    struct tinyui_combo_box *combo_box;
    ldComboBox_t *ld_combo_box;
    int selected_index;
    int previous_selected_index;

    (void)scene;

    if (msg.ptSender == NULL) {
        return false;
    }

    backend = (struct tinyui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return false;
    }

    combo_box = (struct tinyui_combo_box *)backend->host_widget;
    ld_combo_box = tinyui_combo_box_get_ld(backend);
    if (ld_combo_box == NULL) {
        return false;
    }

    if (msg.signal == SIGNAL_PRESS) {
        return false;
    }

    if (msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    selected_index = (int)msg.value;
    if (selected_index < 0 || selected_index >= combo_box->item_count) {
        return false;
    }

    previous_selected_index = combo_box->selected_index;
    if (combo_box->widget.visible == 0 || combo_box->widget.enabled == 0) {
        if (previous_selected_index >= 0 && previous_selected_index < combo_box->item_count) {
            ldComboBoxSetSelectItem(ld_combo_box, (uint8_t)previous_selected_index);
        }
        return false;
    }

    ldComboBoxSetSelectItem(ld_combo_box, (uint8_t)selected_index);
    combo_box->selected_index = selected_index;
    backend->value = selected_index;
    (void)tinyui_widget_claim_backend_focus(backend);
    if (combo_box->cb != 0) {
        combo_box->cb(combo_box, selected_index, combo_box->user_data);
    }
    return false;
}

static void *tinyui_combo_box_create_backend_local(void *parent, const char *id)
{
    struct tinyui_backend_widget *widget;
    struct tinyui_backend_widget *parent_widget = parent;
    struct tinyui_backend_app_state *app_state;
    ldComboBox_t *ld_combo_box;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(widget);
        return 0;
    }
    ld_combo_box = ldComboBox_init(app_state->ld_scene,
                                   NULL,
                                   name_id,
                                   parent_widget->ld_name_id,
                                   0,
                                   0,
                                   220,
                                   32,
                                   (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_combo_box == NULL) {
        free(widget);
        return 0;
    }

    if (tinyui_widget_init_child(widget,
                                         parent,
                                         TINYUI_BACKEND_WIDGET_COMBO_BOX,
                                         id,
                                         parent_widget->theme) != 0) {
        ldComboBox_depose(app_state->ld_scene, ld_combo_box);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_combo_box;
    widget->ld_name_id = name_id;
    widget->value = -1;
    if (tinyui_widget_attach_child(parent, widget) != 0) {
        ldComboBox_depose(app_state->ld_scene, ld_combo_box);
        free(widget);
        return 0;
    }
    return widget;
}

static int tinyui_combo_box_props_are_valid(const struct tinyui_combo_box_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->radius >= 0 &&
           props->padding >= 0;
}

int tinyui_combo_box_set_items(void *backend_widget,
                               const char *const *item_ids,
                               const unsigned char *const *items,
                               int item_count)
{
    struct tinyui_backend_widget *widget = backend_widget;
    ldComboBox_t *ld_combo_box;
    int i;

    if (widget == NULL ||
        widget->kind != TINYUI_BACKEND_WIDGET_COMBO_BOX ||
        widget->ld_widget == NULL ||
        item_ids == NULL ||
        items == NULL ||
        item_count < 0 ||
        item_count > TINYUI_BACKEND_LIST_MAX_ITEMS) {
        return -1;
    }

    ld_combo_box = tinyui_combo_box_get_ld(backend_widget);
    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetStaticItems(ld_combo_box, (uint8_t **)items, (uint8_t)item_count);
    for (i = 0; i < item_count; ++i) {
        widget->list_item_ids[i] = item_ids[i];
    }
    widget->list_item_count = item_count;
    return 0;
}

static int tinyui_combo_box_set_text_color_ld(void *backend_widget, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box = tinyui_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetTextColor(ld_combo_box, tinyui_combo_box_rgb_to_ld_color(rgb));
    return 0;
}

static int tinyui_combo_box_set_bg_color_ld(void *backend_widget, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box = tinyui_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetBackgroundColor(ld_combo_box, tinyui_combo_box_rgb_to_ld_color(rgb));
    return 0;
}

static int tinyui_combo_box_set_frame_color_ld(void *backend_widget, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box = tinyui_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetFrameColor(ld_combo_box, tinyui_combo_box_rgb_to_ld_color(rgb));
    return 0;
}

static int tinyui_combo_box_set_select_color_ld(void *backend_widget, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box = tinyui_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetSelectColor(ld_combo_box, tinyui_combo_box_rgb_to_ld_color(rgb));
    return 0;
}

static int tinyui_combo_box_set_item_max_ld(void *backend_widget, int item_max)
{
    struct tinyui_backend_widget *widget = backend_widget;
    ldComboBox_t *ld_combo_box;

    if (widget == NULL ||
        widget->kind != TINYUI_BACKEND_WIDGET_COMBO_BOX ||
        widget->ld_widget == NULL ||
        item_max <= 0 ||
        item_max > TINYUI_BACKEND_LIST_MAX_ITEMS ||
        item_max < widget->list_item_count) {
        return -1;
    }

    ld_combo_box = tinyui_combo_box_get_ld(backend_widget);
    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetItemMax(ld_combo_box, (uint8_t)item_max);
    return 0;
}

static int tinyui_combo_box_set_dropdown_source_ld(void *backend_widget,
                                                    struct tinyui_image_source *source)
{
    ldComboBox_t *ld_combo_box = tinyui_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    ldComboBoxSetDropdownImage(ld_combo_box, source->img_tile, source->mask_tile);
    return 0;
}

static int tinyui_combo_box_set_selected_index_ld(void *backend_widget, int index)
{
    struct tinyui_backend_widget *widget = backend_widget;
    ldComboBox_t *ld_combo_box;

    if (widget == NULL ||
        widget->kind != TINYUI_BACKEND_WIDGET_COMBO_BOX ||
        widget->ld_widget == NULL ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_combo_box = tinyui_combo_box_get_ld(backend_widget);
    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetSelectItem(ld_combo_box, (uint8_t)index);
    widget->value = index;
    return 0;
}

static int tinyui_combo_box_get_selected_index_ld(void *backend_widget)
{
    ldComboBox_t *ld_combo_box = tinyui_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    if (ld_combo_box->itemCount == 0) {
        return -1;
    }
    return (int)ldComboBoxGetSelectItem(ld_combo_box);
}

static const char *tinyui_combo_box_get_text_ld(void *backend_widget, int index)
{
    ldComboBox_t *ld_combo_box = tinyui_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL || index < 0) {
        return NULL;
    }

    return (const char *)ldComboBoxGetText(ld_combo_box, (uint8_t)index);
}

int tinyui_combo_box_sync_selected_index(struct tinyui_combo_box *combo_box,
                                         int *selected_index_out)
{
    struct tinyui_backend_widget *backend;
    int selected_index;

    if (combo_box == NULL || combo_box->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    selected_index = tinyui_combo_box_get_selected_index_ld(backend);
    if (selected_index < 0 || selected_index >= combo_box->item_count) {
        return -1;
    }

    combo_box->selected_index = selected_index;
    backend->value = selected_index;
    if (selected_index_out != NULL) {
        *selected_index_out = selected_index;
    }
    return 0;
}

int tinyui_combo_box_bind_host(void *backend_widget)
{
    struct tinyui_backend_widget *backend = backend_widget;
    ldComboBox_t *ld_combo_box;

    if (backend == NULL) {
        return -1;
    }

    ld_combo_box = tinyui_combo_box_get_ld(backend_widget);
    if (ld_combo_box == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_combo_box, SIGNAL_PRESS, tinyui_combo_box_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_combo_box, SIGNAL_CLICKED_ITEM, tinyui_combo_box_native_slot)) {
        return -1;
    }
    return 0;
}

int tinyui_combo_box_get_open(void *backend_widget, int *is_open)
{
    ldComboBox_t *ld_combo_box = tinyui_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL || is_open == NULL) {
        return -1;
    }

    *is_open = ld_combo_box->isExpand ? 1 : 0;
    return 0;
}

static void tinyui_combo_box_dispose_partial(struct tinyui_combo_box *combo_box)
{
    struct tinyui_backend_widget *backend;
    struct tinyui_backend_app_state *app_state;

    if (combo_box == 0) {
        return;
    }

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    if (backend != 0) {
        app_state = tinyui_runtime_bridge_backend_state(backend->owner);
        if (backend->parent != 0) {
            (void)tinyui_runtime_bridge_detach_from_parent(backend);
        }
        (void)tinyui_runtime_bridge_unbind_host(backend);
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldComboBox_depose(app_state->ld_scene, (ldComboBox_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(combo_box);
}

struct tinyui_combo_box *tinyui_combo_box_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_combo_box *combo_box;

    if (parent == 0 || id == 0) {
        return 0;
    }

    combo_box = calloc(1, sizeof(*combo_box));
    if (combo_box == 0) {
        return 0;
    }

    combo_box->widget.backend_widget = tinyui_combo_box_create_backend_local(parent->widget.backend_widget, id);
    if (combo_box->widget.backend_widget == 0) {
        free(combo_box);
        return 0;
    }

    combo_box->id = id;
    combo_box->item_max = TINYUI_LIST_MAX_ITEMS;
    combo_box->selected_index = -1;
    combo_box->widget.visible = 1;
    combo_box->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(combo_box->widget.backend_widget, &combo_box->widget) != 0) {
        tinyui_combo_box_dispose_partial(combo_box);
        return 0;
    }
    if (tinyui_combo_box_bind_host(combo_box->widget.backend_widget) != 0) {
        tinyui_combo_box_dispose_partial(combo_box);
        return 0;
    }
    return combo_box;
}

struct tinyui_combo_box *tinyui_combo_box_create_with_props(struct tinyui_window *parent,
                                                            const struct tinyui_combo_box_props *props)
{
    struct tinyui_combo_box *combo_box;

    if (!tinyui_combo_box_props_are_valid(props)) {
        return 0;
    }

    combo_box = tinyui_combo_box_create(parent, props->id);
    if (combo_box == 0) {
        return 0;
    }

    if (tinyui_widget_set_user_data(&combo_box->widget, props->user_data) != 0 ||
        tinyui_widget_set_bg_color(&combo_box->widget, props->bg_color) != 0 ||
        tinyui_widget_set_text_color(&combo_box->widget, props->text_color) != 0 ||
        tinyui_widget_set_border_color(&combo_box->widget, props->border_color) != 0 ||
        tinyui_widget_set_radius(&combo_box->widget, props->radius) != 0 ||
        tinyui_widget_set_padding(&combo_box->widget, props->padding) != 0) {
        tinyui_combo_box_dispose_partial(combo_box);
        return 0;
    }
    if (props->style_class != 0 &&
        tinyui_widget_set_style_class(&combo_box->widget, props->style_class) != 0) {
        tinyui_combo_box_dispose_partial(combo_box);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        tinyui_widget_set_size(&combo_box->widget, props->width, props->height) != 0) {
        tinyui_combo_box_dispose_partial(combo_box);
        return 0;
    }

    return combo_box;
}

int tinyui_combo_box_add_item(struct tinyui_combo_box *combo_box, const char *id, const char *text)
{
    int index;
    int next_count;

    if (combo_box == 0 || id == 0 || text == 0 || combo_box->item_count >= combo_box->item_max) {
        return -1;
    }

    index = combo_box->item_count;
    combo_box->backend_item_ids[index] = id;
    combo_box->backend_item_texts[index] = (const unsigned char *)text;
    next_count = index + 1;
    if (tinyui_combo_box_set_items(combo_box->widget.backend_widget,
                                   combo_box->backend_item_ids,
                                   combo_box->backend_item_texts,
                                   next_count) != 0) {
        combo_box->backend_item_ids[index] = 0;
        combo_box->backend_item_texts[index] = 0;
        return -1;
    }

    combo_box->items[index].id = id;
    combo_box->items[index].text = text;
    combo_box->item_count = next_count;
    return 0;
}

int tinyui_combo_box_set_static_items(struct tinyui_combo_box *combo_box,
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
        if (item_ids[i] == 0 || texts[i] == 0 || tinyui_combo_box_add_item(combo_box, item_ids[i], texts[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

int tinyui_combo_box_set_select_item(struct tinyui_combo_box *combo_box, int index)
{
    return tinyui_combo_box_set_selected_index(combo_box, index);
}

int tinyui_combo_box_set_selected_index(struct tinyui_combo_box *combo_box, int index)
{
    if (combo_box == 0 || index < 0 || index >= combo_box->item_count) {
        return -1;
    }

    if (tinyui_combo_box_set_selected_index_ld(combo_box->widget.backend_widget, index) != 0) {
        return -1;
    }
    combo_box->selected_index = index;
    return 0;
}

int tinyui_combo_box_get_select_item(const struct tinyui_combo_box *combo_box)
{
    return tinyui_combo_box_get_selected_index(combo_box);
}

int tinyui_combo_box_get_selected_index(const struct tinyui_combo_box *combo_box)
{
    int backend_selected_index;

    if (combo_box == 0) {
        return -1;
    }

    if (tinyui_combo_box_sync_selected_index((struct tinyui_combo_box *)combo_box,
                                             &backend_selected_index) == 0) {
        return backend_selected_index;
    }

    return combo_box->selected_index;
}

const char *tinyui_combo_box_get_text(const struct tinyui_combo_box *combo_box, int index)
{
    if (combo_box == 0 || index < 0) {
        return 0;
    }

    return tinyui_combo_box_get_text_ld((void *)combo_box->widget.backend_widget, index);
}

int tinyui_combo_box_is_open(const struct tinyui_combo_box *combo_box, int *is_open)
{
    if (combo_box == 0 || is_open == 0) {
        return -1;
    }

    return tinyui_combo_box_get_open((void *)combo_box->widget.backend_widget, is_open);
}

int tinyui_combo_box_set_text_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    if (combo_box == 0) {
        return -1;
    }

    if (tinyui_combo_box_set_text_color_ld(combo_box->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    combo_box->widget.text_color = rgb;
    return 0;
}

int tinyui_combo_box_set_background_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    return tinyui_combo_box_set_bg_color(combo_box, rgb);
}

int tinyui_combo_box_set_bg_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    if (combo_box == 0) {
        return -1;
    }

    if (tinyui_combo_box_set_bg_color_ld(combo_box->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    combo_box->widget.bg_color = rgb;
    return 0;
}

int tinyui_combo_box_set_frame_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    if (combo_box == 0) {
        return -1;
    }

    if (tinyui_combo_box_set_frame_color_ld(combo_box->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    combo_box->widget.border_color = rgb;
    return 0;
}

int tinyui_combo_box_set_select_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    if (combo_box == 0) {
        return -1;
    }

    return tinyui_combo_box_set_select_color_ld(combo_box->widget.backend_widget, rgb);
}

int tinyui_combo_box_set_item_max(struct tinyui_combo_box *combo_box, int item_max)
{
    if (combo_box == 0 || item_max < combo_box->item_count || item_max <= 0 || item_max > TINYUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (tinyui_combo_box_set_item_max_ld(combo_box->widget.backend_widget, item_max) != 0) {
        return -1;
    }
    combo_box->item_max = item_max;
    return 0;
}

int tinyui_combo_box_set_dropdown_source(struct tinyui_combo_box *combo_box,
                                         struct tinyui_image_source *source)
{
    if (combo_box == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (tinyui_combo_box_set_dropdown_source_ld(combo_box->widget.backend_widget, source) != 0) {
        return -1;
    }
    combo_box->dropdown_source = source;
    return 0;
}

int tinyui_combo_box_set_dropdown_image(struct tinyui_combo_box *combo_box,
                                        struct tinyui_image_source *source)
{
    return tinyui_combo_box_set_dropdown_source(combo_box, source);
}

void tinyui_combo_box_set_on_selected(struct tinyui_combo_box *combo_box,
                                      void (*callback)(struct tinyui_combo_box *combo_box,
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
