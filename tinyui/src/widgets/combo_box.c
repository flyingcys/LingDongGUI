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

static ld_scene_t *s_combo_box_depose_scene = NULL;

static void tinyui_combo_box_ld_depose_cb(void *ld_widget)
{
    if (s_combo_box_depose_scene != NULL) {
        ldComboBox_depose(s_combo_box_depose_scene, (ldComboBox_t *)ld_widget);
        s_combo_box_depose_scene = NULL;
    }
}

static void tinyui_combo_box_rollback(struct tinyui_combo_box *combo_box)
{
    if (combo_box == 0) {
        return;
    }
    if (combo_box->widget.ld_widget != 0) {
        s_combo_box_depose_scene = combo_box->widget.owner != 0
            ? combo_box->widget.owner->ld_scene
            : NULL;
        tinyui_widget_destroy_common(&combo_box->widget, tinyui_combo_box_ld_depose_cb);
    } else {
        free(combo_box);
    }
}

static bool tinyui_combo_box_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct tinyui_widget *w;
    struct tinyui_combo_box *combo_box;
    ldComboBox_t *ld_combo_box;
    int selected_index;
    int previous_selected_index;

    if (msg.ptSender == NULL) {
        return false;
    }

    w = tinyui_widget_from_ld_scene(scene, msg.ptSender);
    if (w == NULL) {
        return false;
    }

    combo_box = (struct tinyui_combo_box *)w;
    ld_combo_box = (ldComboBox_t *)w->ld_widget;
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
    w->value = selected_index;
    (void)tinyui_widget_claim_backend_focus(w);
    if (previous_selected_index == selected_index) {
        return false;
    }
    if (combo_box->cb != 0) {
        combo_box->cb(combo_box, selected_index, combo_box->user_data);
    }
    return false;
}

static int combo_box_props_valid(const struct tinyui_combo_box_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->radius >= 0 &&
           props->padding >= 0;
}

static void *tinyui_combo_box_ld_init(void *ctx,
                                      struct ld_scene_t *scene,
                                      uint16_t name_id,
                                      uint16_t parent_name_id)
{
    (void)ctx;
    return ldComboBox_init(scene,
                           NULL,
                           name_id,
                           parent_name_id,
                           0,
                           0,
                           220,
                           32,
                           (arm_2d_font_t *)&ARM_2D_FONT_6x8);
}

int tinyui_combo_box_set_items(struct tinyui_combo_box *combo_box,
                               const char *const *item_ids,
                               const unsigned char *const *items,
                               int item_count)
{
    ldComboBox_t *ld_combo_box;
    int i;

    if (combo_box == NULL ||
        combo_box->widget.kind != TINYUI_BACKEND_WIDGET_COMBO_BOX ||
        combo_box->widget.ld_widget == NULL ||
        item_ids == NULL ||
        items == NULL ||
        item_count < 0 ||
        item_count > TINYUI_BACKEND_LIST_MAX_ITEMS) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;

    ldComboBoxSetStaticItems(ld_combo_box, (uint8_t **)items, (uint8_t)item_count);
    for (i = 0; i < item_count; ++i) {
        combo_box->backend_item_ids[i] = item_ids[i];
    }
    combo_box->widget.list_item_count = (uint16_t)item_count;
    return 0;
}

int tinyui_combo_box_sync_selected_index(struct tinyui_combo_box *combo_box,
                                         int *selected_index_out)
{
    ldComboBox_t *ld_combo_box;
    int selected_index;

    if (combo_box == NULL || combo_box->widget.ld_widget == NULL) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    if (ld_combo_box->itemCount == 0) {
        return -1;
    }

    selected_index = (int)ldComboBoxGetSelectItem(ld_combo_box);
    if (selected_index < 0 || selected_index >= combo_box->item_count) {
        return -1;
    }

    combo_box->selected_index = selected_index;
    combo_box->widget.value = selected_index;
    if (selected_index_out != NULL) {
        *selected_index_out = selected_index;
    }
    return 0;
}

int tinyui_combo_box_bind_host(struct tinyui_combo_box *combo_box)
{
    ldComboBox_t *ld_combo_box;

    if (combo_box == NULL || combo_box->widget.ld_widget == NULL) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;

    if (!ldMsgConnect(ld_combo_box, SIGNAL_PRESS, tinyui_combo_box_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_combo_box, SIGNAL_CLICKED_ITEM, tinyui_combo_box_native_slot)) {
        return -1;
    }
    return 0;
}

struct tinyui_combo_box *tinyui_combo_box_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_combo_box *combo_box;

    if (parent == 0 || id == 0) {
        return 0;
    }
    if (parent->widget.owner == NULL || parent->widget.ld_widget == NULL) {
        return 0;
    }

    combo_box = (struct tinyui_combo_box *)tinyui_widget_create_leaf(&parent->widget,
                                                                     TINYUI_BACKEND_WIDGET_COMBO_BOX,
                                                                     tinyui_combo_box_ld_init,
                                                                     0,
                                                                     sizeof(*combo_box));
    if (combo_box == 0) {
        return 0;
    }
    combo_box->widget.value = -1;

    combo_box->id = id;
    combo_box->item_max = TINYUI_LIST_MAX_ITEMS;
    combo_box->selected_index = -1;
    combo_box->widget.visible = 1;
    combo_box->widget.enabled = 1;

    if (tinyui_combo_box_bind_host(combo_box) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    return combo_box;
}

struct tinyui_combo_box *tinyui_combo_box_create_with_props(struct tinyui_window *parent,
                                                            const struct tinyui_combo_box_props *props)
{
    struct tinyui_combo_box *combo_box;

    if (!combo_box_props_valid(props)) {
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
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    if (props->style_class != 0 &&
        tinyui_widget_set_style_class(&combo_box->widget, props->style_class) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        tinyui_widget_set_size(&combo_box->widget, props->width, props->height) != 0) {
        tinyui_combo_box_rollback(combo_box);
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
    if (tinyui_combo_box_set_items(combo_box,
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
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 ||
        combo_box->widget.kind != TINYUI_BACKEND_WIDGET_COMBO_BOX ||
        combo_box->widget.ld_widget == 0 ||
        index < 0 ||
        index >= (int)combo_box->widget.list_item_count) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetSelectItem(ld_combo_box, (uint8_t)index);
    combo_box->widget.value = index;
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
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || index < 0 || combo_box->widget.ld_widget == 0) {
        return 0;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    return (const char *)ldComboBoxGetText(ld_combo_box, (uint8_t)index);
}

int tinyui_combo_box_is_open(const struct tinyui_combo_box *combo_box, int *is_open)
{
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || is_open == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    *is_open = ld_combo_box->isExpand ? 1 : 0;
    return 0;
}

int tinyui_combo_box_set_text_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetTextColor(ld_combo_box, (ldColor)tinyui_rgb_to_ld_color(rgb));
    combo_box->widget.text_color = rgb;
    return 0;
}

int tinyui_combo_box_set_background_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    return tinyui_combo_box_set_bg_color(combo_box, rgb);
}

int tinyui_combo_box_set_bg_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetBackgroundColor(ld_combo_box, (ldColor)tinyui_rgb_to_ld_color(rgb));
    combo_box->widget.bg_color = rgb;
    return 0;
}

int tinyui_combo_box_set_frame_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetFrameColor(ld_combo_box, (ldColor)tinyui_rgb_to_ld_color(rgb));
    combo_box->widget.border_color = rgb;
    return 0;
}

int tinyui_combo_box_set_select_color(struct tinyui_combo_box *combo_box, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetSelectColor(ld_combo_box, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}

int tinyui_combo_box_set_item_max(struct tinyui_combo_box *combo_box, int item_max)
{
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 ||
        combo_box->widget.kind != TINYUI_BACKEND_WIDGET_COMBO_BOX ||
        combo_box->widget.ld_widget == 0 ||
        item_max <= 0 ||
        item_max > TINYUI_BACKEND_LIST_MAX_ITEMS ||
        item_max < combo_box->item_count ||
        item_max < (int)combo_box->widget.list_item_count) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetItemMax(ld_combo_box, (uint8_t)item_max);
    combo_box->item_max = item_max;
    return 0;
}

int tinyui_combo_box_set_dropdown_source(struct tinyui_combo_box *combo_box,
                                         struct tinyui_image_source *source)
{
    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0 ||
        source == 0 || source->img_tile == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetDropdownImage(ld_combo_box, source->img_tile, source->mask_tile);
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
