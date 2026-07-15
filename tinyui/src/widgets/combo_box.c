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
#include "widgets/combo_box.h"

#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldComboBox.h"
#include "../../../src/misc/ldMsg.h"

#include <string.h>

/* Fire unified event-pool callbacks from widget TUs (event_fire is static in core). */
static void tinyui_selection_fire_value_changed(struct tinyui_widget *widget, int32_t value)
{
    struct tinyui_runtime_state *rt = tinyui_runtime_state_get();
    struct tinyui_event_callback_pool *pool;
    struct tinyui_event_callback_slot *ordered[TINYUI_EVENT_CB_CAPACITY];
    size_t ordered_count = 0U;
    size_t i;
    uint16_t epoch;
    uint32_t mask;
    tinyui_obj_t *target;
    bool was_processing;

    if (rt == 0 || widget == 0 || !rt->initialized) {
        return;
    }

    pool = &rt->event_cb_pool;
    target = (tinyui_obj_t *)(void *)widget;
    mask = TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED);

    epoch = (uint16_t)(pool->dispatch_epoch + 1U);
    if (epoch == 0U) {
        epoch = 1U;
    }
    pool->dispatch_epoch = epoch;
    rt->dispatch_epoch = epoch;

    for (i = 0; i < (size_t)TINYUI_EVENT_CB_CAPACITY; ++i) {
        struct tinyui_event_callback_slot *slot = &pool->slots[i];
        if (slot->allocated == 0U || slot->cb == 0) {
            continue;
        }
        if (slot->object != target) {
            continue;
        }
        if ((slot->event_mask & mask) == 0U) {
            continue;
        }
        if (!(slot->born_epoch < epoch)) {
            continue;
        }
        ordered[ordered_count++] = slot;
    }

    for (i = 1; i < ordered_count; ++i) {
        size_t j = i;
        struct tinyui_event_callback_slot *cur = ordered[i];
        while (j > 0U &&
               ordered[j - 1U]->registration_order > cur->registration_order) {
            ordered[j] = ordered[j - 1U];
            j -= 1U;
        }
        ordered[j] = cur;
    }

    was_processing = rt->processing;
    rt->processing = true;
    for (i = 0; i < ordered_count; ++i) {
        struct tinyui_event_callback_slot *slot = ordered[i];
        uint16_t generation;
        tinyui_event_cb_t cb;
        tinyui_event_t event;

        if (slot->allocated == 0U || slot->cb == 0) {
            continue;
        }
        if (slot->object != target || (slot->event_mask & mask) == 0U) {
            continue;
        }
        if (!(slot->born_epoch < epoch)) {
            continue;
        }
        if (widget->deleting != 0U) {
            break;
        }

        generation = slot->generation;
        cb = slot->cb;
        memset(&event, 0, sizeof(event));
        event.code = TINYUI_EVENT_VALUE_CHANGED;
        event.target = target;
        event.user_data = slot->user_data;
        event.data.value = value;
        cb(&event);

        if (slot->allocated == 0U || slot->generation != generation) {
            continue;
        }
        if (widget->deleting != 0U) {
            break;
        }
    }
    rt->processing = was_processing;
    if (!was_processing && rt->delete_pending != 0 && rt->delete_target != 0) {
        tinyui_obj_t *delete_target = rt->delete_target;
        struct tinyui_widget *delete_widget =
            (struct tinyui_widget *)(void *)delete_target;

        rt->delete_pending = 0;
        rt->delete_target = 0;
        (void)tinyui_runtime_internal_widget_destroy(delete_widget);
    }
}

static struct tinyui_combo_box *tinyui_combo_box_as_combo_box(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_COMBO_BOX)) {
        return 0;
    }
    return (struct tinyui_combo_box *)w;
}

static const struct tinyui_combo_box *tinyui_combo_box_as_combo_box_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_COMBO_BOX)) {
        return 0;
    }
    return (const struct tinyui_combo_box *)w;
}


static void tinyui_combo_box_rollback(struct tinyui_combo_box *combo_box)
{
    if (combo_box == 0) {
        return;
    }
    if (combo_box->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&combo_box->widget);
    } else {
        ldFree(combo_box);
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

    w = tinyui_runtime_internal_widget_from_ld_scene(scene, msg.ptSender);
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
    (void)tinyui_runtime_internal_widget_claim_backend_focus(w);
    if (previous_selected_index == selected_index) {
        return false;
    }
    if (combo_box->cb != 0) {
        combo_box->cb(combo_box, selected_index, combo_box->user_data);
    }
    tinyui_selection_fire_value_changed(w, (int32_t)selected_index);
    return false;
}

static int combo_box_props_valid(const tinyui_combo_box_props_t *props)
{
    return props != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->radius >= 0 &&
           props->padding >= 0;
}

static void *tinyui_runtime_internal_combo_box_ld_init(void *ctx,
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
                           tinyui_resolve_ld_font(0, 12));
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

tinyui_obj_t *tinyui_combo_box_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "combo_box";
    if (parent_w == 0) { return 0; }

    struct tinyui_combo_box *combo_box;

    if (parent_w == 0 || id == 0) {
        return 0;
    }
    if (parent_w->owner == NULL || parent_w->ld_widget == NULL) {
        return 0;
    }

    combo_box = (struct tinyui_combo_box *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                                     TINYUI_BACKEND_WIDGET_COMBO_BOX,
                                                                     tinyui_runtime_internal_combo_box_ld_init,
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
    return (tinyui_obj_t *)combo_box;
}

tinyui_obj_t *tinyui_combo_box_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_combo_box_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_combo_box *combo_box;

    if (props == 0) {
        return tinyui_combo_box_create(parent);
    }

    obj = tinyui_combo_box_create(parent);
    if (obj == 0) {
        return 0;
    }
    combo_box = (struct tinyui_combo_box *)(void *)obj;

    if ((props->fields & TINYUI_COMBO_BOX_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_COMBO_BOX_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&combo_box->widget, props->user_data) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_COMBO_BOX_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&combo_box->widget, props->style_class) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    }
        if ((props->fields & TINYUI_COMBO_BOX_FIELD_WIDTH) != 0 || (props->fields & TINYUI_COMBO_BOX_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&combo_box->widget);
        int h = tinyui_runtime_internal_widget_get_height(&combo_box->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_COMBO_BOX_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_COMBO_BOX_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&combo_box->widget, w, h) != 0) {
            tinyui_combo_box_rollback(combo_box);
            return 0;
        }
    }
    if ((props->fields & TINYUI_COMBO_BOX_FIELD_BG_COLOR) != 0) {
    if (tinyui_combo_box_set_bg_color((tinyui_obj_t *)combo_box, props->bg_color) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_COMBO_BOX_FIELD_TEXT_COLOR) != 0) {
    if (tinyui_combo_box_set_text_color((tinyui_obj_t *)combo_box, props->text_color) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_COMBO_BOX_FIELD_BORDER_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_border_color(&combo_box->widget, props->border_color) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_COMBO_BOX_FIELD_RADIUS) != 0) {
    if (tinyui_runtime_internal_widget_set_radius(&combo_box->widget, props->radius) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_COMBO_BOX_FIELD_PADDING) != 0) {
    if (tinyui_runtime_internal_widget_set_padding(&combo_box->widget, props->padding) != 0) {
        tinyui_combo_box_rollback(combo_box);
        return 0;
    }
    }

    return obj;
}


int tinyui_combo_box_add_item(tinyui_obj_t *combo_box_obj, const char *id, const char *text)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    ldComboBox_t *ld_combo_box;
    int index;
    int next_count;

    if (combo_box == 0 || id == 0 || text == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return -1;
    }
    if (combo_box->widget.ld_widget == 0 ||
        combo_box->widget.kind != TINYUI_BACKEND_WIDGET_COMBO_BOX) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return -1;
    }
    if (combo_box->item_count >= combo_box->item_max ||
        combo_box->item_count >= TINYUI_LIST_MAX_ITEMS) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    index = combo_box->item_count;
    next_count = index + 1;

    /* Prefer dynamic LD path when itemMax has been configured. */
    if (ld_combo_box->itemMax > 0 && ld_combo_box->isStatic == false) {
        if (ld_combo_box->itemCount >= ld_combo_box->itemMax) {
            tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
            return -1;
        }
        ldComboBoxAddItem(ld_combo_box, (uint8_t *)text);
        if ((int)ld_combo_box->itemCount != next_count) {
            tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
            return -1;
        }
        combo_box->backend_item_ids[index] = id;
        combo_box->backend_item_texts[index] = (const unsigned char *)text;
        combo_box->widget.list_item_count = (uint16_t)next_count;
    } else if (ld_combo_box->itemMax > 0 && ld_combo_box->isStatic == true &&
               ld_combo_box->itemCount == 0 && combo_box->item_count == 0) {
        /* After set_item_max, first dynamic add uses AddItem which flips isStatic. */
        ldComboBoxAddItem(ld_combo_box, (uint8_t *)text);
        if ((int)ld_combo_box->itemCount != 1) {
            tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
            return -1;
        }
        combo_box->backend_item_ids[index] = id;
        combo_box->backend_item_texts[index] = (const unsigned char *)text;
        combo_box->widget.list_item_count = 1;
    } else {
        combo_box->backend_item_ids[index] = id;
        combo_box->backend_item_texts[index] = (const unsigned char *)text;
        if (tinyui_combo_box_set_items(combo_box,
                                       combo_box->backend_item_ids,
                                       combo_box->backend_item_texts,
                                       next_count) != 0) {
            combo_box->backend_item_ids[index] = 0;
            combo_box->backend_item_texts[index] = 0;
            tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
            return -1;
        }
    }

    combo_box->items[index].id = id;
    combo_box->items[index].text = text;
    combo_box->item_count = next_count;
    tinyui_runtime_set_last_result(TINYUI_OK);
    return 0;
}

int tinyui_combo_box_set_static_items(tinyui_obj_t *combo_box_obj, const char *const *item_ids, const char *const *texts, int item_count)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    int i;

    if (combo_box == 0 || item_ids == 0 || texts == 0 || item_count < 0 || item_count > combo_box->item_max) {
        return -1;
    }

    combo_box->item_count = 0;
    combo_box->selected_index = -1;
    for (i = 0; i < item_count; ++i) {
        if (item_ids[i] == 0 || texts[i] == 0 || tinyui_combo_box_add_item((tinyui_obj_t *)combo_box, item_ids[i], texts[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

int tinyui_combo_box_set_select_item(tinyui_obj_t *combo_box_obj, int index)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    return tinyui_combo_box_set_selected_index((tinyui_obj_t *)combo_box, index);
}

int tinyui_combo_box_set_selected_index(tinyui_obj_t *combo_box_obj, int index)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

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

int tinyui_combo_box_get_select_item(const tinyui_obj_t *combo_box_obj)
{
    const struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box_const(combo_box_obj);
    if (combo_box == 0) { return -1; }

    return tinyui_combo_box_get_selected_index((tinyui_obj_t *)combo_box);
}

int tinyui_combo_box_get_selected_index(const tinyui_obj_t *combo_box_obj)
{
    const struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box_const(combo_box_obj);
    if (combo_box == 0) { return -1; }

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

const char * tinyui_combo_box_get_text(const tinyui_obj_t *combo_box_obj, int index)
{
    const struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box_const(combo_box_obj);
    if (combo_box == 0) { return 0; }

    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || index < 0 || combo_box->widget.ld_widget == 0) {
        return 0;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    return (const char *)ldComboBoxGetText(ld_combo_box, (uint8_t)index);
}

int tinyui_combo_box_is_open(const tinyui_obj_t *combo_box_obj, int *is_open)
{
    const struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box_const(combo_box_obj);
    if (combo_box == 0) { return -1; }

    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || is_open == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    *is_open = ld_combo_box->isExpand ? 1 : 0;
    return 0;
}

int tinyui_combo_box_set_text_color(tinyui_obj_t *combo_box_obj, unsigned int rgb)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetTextColor(ld_combo_box, (ldColor)tinyui_rgb_to_ld_color(rgb));
    combo_box->widget.text_color = rgb;
    return 0;
}

int tinyui_combo_box_set_background_color(tinyui_obj_t *combo_box_obj, unsigned int rgb)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    return tinyui_combo_box_set_bg_color((tinyui_obj_t *)combo_box, rgb);
}

int tinyui_combo_box_set_bg_color(tinyui_obj_t *combo_box_obj, unsigned int rgb)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetBackgroundColor(ld_combo_box, (ldColor)tinyui_rgb_to_ld_color(rgb));
    combo_box->widget.bg_color = rgb;
    return 0;
}

int tinyui_combo_box_set_frame_color(tinyui_obj_t *combo_box_obj, unsigned int rgb)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetFrameColor(ld_combo_box, (ldColor)tinyui_rgb_to_ld_color(rgb));
    combo_box->widget.border_color = rgb;
    return 0;
}

int tinyui_combo_box_set_select_color(tinyui_obj_t *combo_box_obj, unsigned int rgb)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetSelectColor(ld_combo_box, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}

int tinyui_combo_box_set_item_max(tinyui_obj_t *combo_box_obj, int item_max)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

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
    tinyui_runtime_set_last_result(TINYUI_OK);
    return 0;
}

int tinyui_combo_box_set_dropdown_source(tinyui_obj_t *combo_box_obj, struct tinyui_image_source *source)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    ldComboBox_t *ld_combo_box;

    if (combo_box == 0 || combo_box->widget.ld_widget == 0 ||
        source == 0 || tinyui_image_source_get_image_tile(source) == 0) {
        return -1;
    }

    ld_combo_box = (ldComboBox_t *)combo_box->widget.ld_widget;
    ldComboBoxSetDropdownImage(ld_combo_box, tinyui_image_source_get_image_tile(source), tinyui_image_source_get_mask_tile(source));
    combo_box->dropdown_source = source;
    return 0;
}

int tinyui_combo_box_set_dropdown_image(tinyui_obj_t *combo_box_obj, struct tinyui_image_source *source)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) { return -1; }

    return tinyui_combo_box_set_dropdown_source((tinyui_obj_t *)combo_box, source);
}

void tinyui_combo_box_set_on_selected(tinyui_obj_t *combo_box_obj,
                                      void (*callback)(tinyui_obj_t *combo_box,
                                                       int index,
                                                       void *user_data),
                                      void *user_data)
{
    struct tinyui_combo_box *combo_box = tinyui_combo_box_as_combo_box(combo_box_obj);
    if (combo_box == 0) {
        return;
    }

    combo_box->cb = (void (*)(struct tinyui_combo_box *, int, void *))callback;
    combo_box->user_data = user_data;
}
