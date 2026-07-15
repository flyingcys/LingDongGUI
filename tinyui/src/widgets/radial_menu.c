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
#include "widgets/radial_menu.h"
#include "internal/widget_legacy.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldRadialMenu.h"
#include "../../../src/misc/ldMsg.h"

#include <string.h>

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
        if (slot->object != target || (slot->event_mask & mask) == 0U) {
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

static struct tinyui_radial_menu *tinyui_radial_menu_as_radial_menu(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_RADIAL_MENU)) {
        return 0;
    }
    return (struct tinyui_radial_menu *)w;
}

static const struct tinyui_radial_menu *tinyui_radial_menu_as_radial_menu_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_RADIAL_MENU)) {
        return 0;
    }
    return (const struct tinyui_radial_menu *)w;
}


extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

static arm_2d_tile_t *const g_radial_menu_tiles[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
};

static arm_2d_tile_t *const g_radial_menu_masks[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
};

#define TINYUI_RADIAL_MENU_NATIVE_MAX_ITEMS 5


struct tinyui_radial_menu_create_ctx {
    int width;
    int height;
    int x_axis;
    int y_axis;
    int item_max;
};


static void tinyui_radial_menu_rollback(struct tinyui_radial_menu *radial_menu)
{
    if (radial_menu == 0) {
        return;
    }
    if (radial_menu->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&radial_menu->widget);
    } else {
        ldFree(radial_menu);
    }
}

static void *tinyui_runtime_internal_radial_menu_ld_init(void *ctx,
                                        struct ld_scene_t *scene,
                                        uint16_t name_id,
                                        uint16_t parent_name_id)
{
    const struct tinyui_radial_menu_create_ctx *create_ctx =
        (const struct tinyui_radial_menu_create_ctx *)ctx;

    if (create_ctx == 0) {
        return 0;
    }

    return ldRadialMenu_init(scene,
                             NULL,
                             name_id,
                             parent_name_id,
                             0,
                             0,
                             (int16_t)create_ctx->width,
                             (int16_t)create_ctx->height,
                             (uint16_t)create_ctx->x_axis,
                             (uint16_t)create_ctx->y_axis,
                             (uint8_t)create_ctx->item_max);
}

static bool tinyui_radial_menu_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct tinyui_widget *w;
    struct tinyui_radial_menu *radial_menu;
    ldRadialMenu_t *ld_radial_menu;
    int selected_index;
    int previous_selected_index;

    if (msg.ptSender == 0 || msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    w = tinyui_runtime_internal_widget_from_ld_scene(scene, msg.ptSender);
    if (w == 0) {
        return false;
    }

    radial_menu = (struct tinyui_radial_menu *)w;
    ld_radial_menu = (ldRadialMenu_t *)w->ld_widget;
    selected_index = (int)msg.value;
    if (selected_index < 0 || selected_index >= radial_menu->item_count ||
        ld_radial_menu == 0) {
        return false;
    }

    previous_selected_index = radial_menu->selected_index;
    if (radial_menu->widget.visible == 0 || radial_menu->widget.enabled == 0) {
        return false;
    }

    if (tinyui_runtime_internal_widget_claim_backend_focus(w) != 0) {
        return false;
    }

    /* Instant sync to LD selectItem (not animated SetClickItem) so getter
     * readback matches native SIGNAL_CLICKED_ITEM payload. */
    ldRadialMenuSetDefaultItem(ld_radial_menu, (uint8_t)selected_index);
    radial_menu->selected_index = selected_index;
    w->value = selected_index;
    if (previous_selected_index == selected_index) {
        return false;
    }
    if (radial_menu->cb != 0) {
        radial_menu->cb(radial_menu, selected_index, radial_menu->user_data);
    }
    tinyui_selection_fire_value_changed(w, (int32_t)selected_index);
    return false;
}

static int tinyui_radial_menu_props_are_valid(const tinyui_radial_menu_props_t *props)
{
    return props != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->x_axis >= 0 &&
           props->y_axis >= 0 &&
           props->item_max >= 0 &&
           props->default_index >= -1;
}

static struct tinyui_radial_menu *tinyui_radial_menu_create_internal(struct tinyui_widget *parent,
                                                                     const char *id,
                                                                     int width,
                                                                     int height,
                                                                     int x_axis,
                                                                     int y_axis,
                                                                     int item_max)
{
    struct tinyui_radial_menu *radial_menu;
    ldRadialMenu_t *ld_radial_menu;
    struct tinyui_radial_menu_create_ctx create_ctx;

    if (parent == 0 || id == 0) {
        return 0;
    }

    if (width <= 0 || height <= 0) {
        return 0;
    }
    if (x_axis <= 0) {
        x_axis = 1;
    }
    if (y_axis <= 0) {
        y_axis = 1;
    }
    if (item_max <= 0) {
        item_max = 1;
    }

    if (((struct tinyui_widget *)(void *)parent)->ld_widget == 0 || ((struct tinyui_widget *)(void *)parent)->owner == 0 || ((struct tinyui_widget *)(void *)parent)->owner->ld_scene == 0) {
        return 0;
    }

    create_ctx.width = width;
    create_ctx.height = height;
    create_ctx.x_axis = x_axis;
    create_ctx.y_axis = y_axis;
    create_ctx.item_max = item_max;

    radial_menu = (struct tinyui_radial_menu *)tinyui_runtime_internal_widget_create_leaf(
        parent,
        TINYUI_BACKEND_WIDGET_RADIAL_MENU,
        tinyui_runtime_internal_radial_menu_ld_init,
        &create_ctx,
        sizeof(*radial_menu));
    if (radial_menu == 0) {
        return 0;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    if (ld_radial_menu == 0) {
        tinyui_radial_menu_rollback(radial_menu);
        return 0;
    }

    radial_menu->widget.value = -1;
    radial_menu->widget.visible = 1;
    radial_menu->widget.enabled = 1;

    radial_menu->id = id;
    radial_menu->selected_index = -1;
    radial_menu->x_axis = x_axis;
    radial_menu->y_axis = y_axis;
    radial_menu->item_max = item_max;

    if (!ldMsgConnect(ld_radial_menu, SIGNAL_CLICKED_ITEM, tinyui_radial_menu_native_slot)) {
        tinyui_radial_menu_rollback(radial_menu);
        return 0;
    }

    return radial_menu;
}

/**
 * @brief Create radial menu widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

tinyui_obj_t *tinyui_radial_menu_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "radial_menu";
    if (parent_w == 0) { return 0; }

    return tinyui_radial_menu_create_internal(parent_w, id, 194, 96, 68, 46, 5);
}

/**
 * @brief radial menu init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_radial_menu *tinyui_runtime_internal_radial_menu_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_radial_menu_create(parent);
}

/**
 * @brief Create radial menu widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_radial_menu_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_radial_menu_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_radial_menu *radial_menu;

    if (props == 0) {
        return tinyui_radial_menu_create(parent);
    }

    obj = tinyui_radial_menu_create(parent);
    if (obj == 0) {
        return 0;
    }
    radial_menu = (struct tinyui_radial_menu *)(void *)obj;

    if ((props->fields & TINYUI_RADIAL_MENU_FIELD_ID) != 0) {
        (void)props->id;
    }
    if ((props->fields & TINYUI_RADIAL_MENU_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&radial_menu->widget, props->user_data) != 0) {
        tinyui_radial_menu_rollback(radial_menu);
        return 0;
    }
    }
    if ((props->fields & TINYUI_RADIAL_MENU_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&radial_menu->widget, props->style_class) != 0) {
        tinyui_radial_menu_rollback(radial_menu);
        return 0;
    }
    }
    if ((props->fields & TINYUI_RADIAL_MENU_FIELD_WIDTH) != 0 ||
        (props->fields & TINYUI_RADIAL_MENU_FIELD_HEIGHT) != 0 ||
        (props->fields & TINYUI_RADIAL_MENU_FIELD_X_AXIS) != 0 ||
        (props->fields & TINYUI_RADIAL_MENU_FIELD_Y_AXIS) != 0 ||
        (props->fields & TINYUI_RADIAL_MENU_FIELD_ITEM_MAX) != 0) {
        int w = ((props->fields & TINYUI_RADIAL_MENU_FIELD_WIDTH) != 0) ? props->width : tinyui_runtime_internal_widget_get_width(&radial_menu->widget);
        int h = ((props->fields & TINYUI_RADIAL_MENU_FIELD_HEIGHT) != 0) ? props->height : tinyui_runtime_internal_widget_get_height(&radial_menu->widget);
        int xa = ((props->fields & TINYUI_RADIAL_MENU_FIELD_X_AXIS) != 0) ? props->x_axis : radial_menu->x_axis;
        int ya = ((props->fields & TINYUI_RADIAL_MENU_FIELD_Y_AXIS) != 0) ? props->y_axis : radial_menu->y_axis;
        int im = ((props->fields & TINYUI_RADIAL_MENU_FIELD_ITEM_MAX) != 0) ? props->item_max : radial_menu->item_max;
        if (w < 0) { w = 0; }
        if (h < 0) { h = 0; }
            if (tinyui_radial_menu_set_geometry((tinyui_obj_t *)radial_menu, w, h, xa, ya, im) != 0) {
                tinyui_radial_menu_rollback(radial_menu);
                return 0;
            }
    }
    if ((props->fields & TINYUI_RADIAL_MENU_FIELD_DEFAULT_INDEX) != 0) {
    if (tinyui_radial_menu_set_default_item((tinyui_obj_t *)radial_menu, props->default_index) != 0) {
        tinyui_radial_menu_rollback(radial_menu);
        return 0;
    }
    }

    return obj;
}



/**
 * @brief radial menu add item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_add_item(tinyui_obj_t *radial_menu_obj, const char *id)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    int index;
    int before_count;
    ldRadialMenu_t *ld_radial_menu;

    if (radial_menu == 0 || id == 0 ||
        radial_menu->widget.ld_widget == 0 ||
        radial_menu->widget.kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return -1;
    }
    if (radial_menu->item_count >= TINYUI_LIST_MAX_ITEMS ||
        (int)radial_menu->widget.list_item_count >= TINYUI_RADIAL_MENU_NATIVE_MAX_ITEMS ||
        (radial_menu->item_max > 0 &&
         (int)radial_menu->widget.list_item_count >= radial_menu->item_max)) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    if ((int)ld_radial_menu->use_as__ldBase_t.itemCount >= (int)ld_radial_menu->itemMax) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }

    index = (int)radial_menu->widget.list_item_count;
    before_count = (int)ld_radial_menu->use_as__ldBase_t.itemCount;
    ldRadialMenuAddItem(ld_radial_menu,
                        g_radial_menu_tiles[index % 5],
                        g_radial_menu_masks[index % 5]);
    if ((int)ld_radial_menu->use_as__ldBase_t.itemCount != before_count + 1) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }
    radial_menu->widget.list_item_count++;
    if (radial_menu->widget.value < 0) {
        radial_menu->widget.value = 0;
    }

    index = radial_menu->item_count++;
    radial_menu->items[index].id = id;
    radial_menu->items[index].text = id;
    if (radial_menu->selected_index < 0) {
        radial_menu->selected_index = 0;
    }
    if (radial_menu->selected_index >= 0 &&
        radial_menu->selected_index < radial_menu->item_count) {
        ldRadialMenuSetDefaultItem(ld_radial_menu,
                                   (uint8_t)radial_menu->selected_index);
        radial_menu->widget.value = radial_menu->selected_index;
    }
    tinyui_runtime_set_last_result(TINYUI_OK);
    return 0;
}

/**
 * @brief radial menu add item with source
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_add_item_with_source(tinyui_obj_t *radial_menu_obj, const char *id, struct tinyui_image_source *source)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    int index;
    int before_count;
    ldRadialMenu_t *ld_radial_menu;

    if (radial_menu == 0
        || id == 0
        || source == 0
        || tinyui_image_source_get_image_tile(source) == 0
        || tinyui_image_source_get_mask_tile(source) == 0
        || radial_menu->widget.ld_widget == 0
        || radial_menu->widget.kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return -1;
    }
    if (radial_menu->item_count >= TINYUI_LIST_MAX_ITEMS ||
        (int)radial_menu->widget.list_item_count >= TINYUI_RADIAL_MENU_NATIVE_MAX_ITEMS ||
        (radial_menu->item_max > 0 &&
         (int)radial_menu->widget.list_item_count >= radial_menu->item_max)) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    if ((int)ld_radial_menu->use_as__ldBase_t.itemCount >= (int)ld_radial_menu->itemMax) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }

    index = (int)radial_menu->widget.list_item_count;
    before_count = (int)ld_radial_menu->use_as__ldBase_t.itemCount;
    ldRadialMenuAddItem(ld_radial_menu,
                        tinyui_image_source_get_image_tile(source),
                        tinyui_image_source_get_mask_tile(source));
    if ((int)ld_radial_menu->use_as__ldBase_t.itemCount != before_count + 1) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }
    radial_menu->widget.list_item_count++;
    if (radial_menu->widget.value < 0) {
        radial_menu->widget.value = 0;
    }

    index = radial_menu->item_count++;
    radial_menu->items[index].id = id;
    radial_menu->items[index].text = id;
    radial_menu->item_sources[index] = source;
    if (radial_menu->selected_index < 0) {
        radial_menu->selected_index = 0;
    }
    if (radial_menu->selected_index >= 0 &&
        radial_menu->selected_index < radial_menu->item_count) {
        ldRadialMenuSetDefaultItem(ld_radial_menu,
                                   (uint8_t)radial_menu->selected_index);
        radial_menu->widget.value = radial_menu->selected_index;
    }
    tinyui_runtime_set_last_result(TINYUI_OK);
    return 0;
}

/**
 * @brief radial menu add item with image
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_add_item_with_image(tinyui_obj_t *radial_menu_obj, const char *id, struct tinyui_image_source *source)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    return tinyui_radial_menu_add_item_with_source((tinyui_obj_t *)radial_menu, id, source);
}

int tinyui_radial_menu_set_geometry(tinyui_obj_t *radial_menu_obj, int width, int height, int x_axis, int y_axis, int item_max)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    ldRadialMenu_t *ld_radial_menu;

    if (radial_menu == 0 || width <= 0 || height <= 0 || x_axis <= 0 || y_axis <= 0
        || item_max <= 0 || item_max > TINYUI_RADIAL_MENU_NATIVE_MAX_ITEMS
        || radial_menu->widget.ld_widget == 0
        || radial_menu->widget.kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU
        || radial_menu->widget.list_item_count != 0) {
        return -1;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    ldBaseSetWidth((ldBase_t *)ld_radial_menu, (int16_t)width);
    ldBaseSetHeight((ldBase_t *)ld_radial_menu, (int16_t)height);
    ld_radial_menu->originPos.iX = (int16_t)(width >> 1);
    ld_radial_menu->originPos.iY = (int16_t)(height >> 1);
    ld_radial_menu->xAxis = (uint16_t)x_axis;
    ld_radial_menu->yAxis = (uint16_t)y_axis;
    ld_radial_menu->itemMax = (uint8_t)item_max;
    radial_menu->x_axis = x_axis;
    radial_menu->y_axis = y_axis;
    radial_menu->item_max = item_max;
    return 0;
}

/**
 * @brief Set selected index of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_set_selected_index(tinyui_obj_t *radial_menu_obj, int index)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    ldRadialMenu_t *ld_radial_menu;

    if (radial_menu == 0 || index < 0 || index >= radial_menu->item_count) {
        return -1;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    if (ld_radial_menu == 0 ||
        radial_menu->widget.kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        index >= (int)radial_menu->widget.list_item_count) {
        return -1;
    }

    ldRadialMenuSetDefaultItem(ld_radial_menu, (uint8_t)index);
    radial_menu->widget.value = index;
    radial_menu->selected_index = index;
    return 0;
}

/**
 * @brief Get selected index of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @return -1 on failure
 */

int tinyui_radial_menu_get_selected_index(const tinyui_obj_t *radial_menu_obj)
{
    const struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu_const(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    ldRadialMenu_t *ld_radial_menu;

    if (radial_menu == 0) {
        return -1;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    if (ld_radial_menu != 0) {
        int ld_index = (int)ld_radial_menu->selectItem;

        if (ld_index >= 0 && ld_index < radial_menu->item_count) {
            ((struct tinyui_radial_menu *)radial_menu)->selected_index = ld_index;
            return ld_index;
        }
    }

    return radial_menu->selected_index;
}

/**
 * @brief radial menu offset selection
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_offset_selection(tinyui_obj_t *radial_menu_obj, int offset)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    ldRadialMenu_t *ld_radial_menu;
    int item_count;
    int next_index;

    if (radial_menu == 0 || radial_menu->item_count <= 0) {
        return -1;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    if (ld_radial_menu == 0 ||
        radial_menu->widget.kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        (int)radial_menu->widget.list_item_count <= 0) {
        return -1;
    }

    item_count = (int)radial_menu->widget.list_item_count;
    next_index = (int)ld_radial_menu->selectItem;
    if (next_index < 0) {
        next_index = 0;
    }
    next_index = (next_index + offset) % item_count;
    if (next_index < 0) {
        next_index += item_count;
    }

    ldRadialMenuSetDefaultItem(ld_radial_menu, (uint8_t)next_index);
    radial_menu->widget.value = next_index;
    if (next_index >= 0 && next_index < radial_menu->item_count) {
        radial_menu->selected_index = next_index;
    }
    return 0;
}

/**
 * @brief Set default item of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_set_default_item(tinyui_obj_t *radial_menu_obj, int index)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    return tinyui_radial_menu_set_selected_index((tinyui_obj_t *)radial_menu, index);
}

/**
 * @brief radial menu click item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_click_item(tinyui_obj_t *radial_menu_obj, int index)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    ldRadialMenu_t *ld_radial_menu;

    if (radial_menu == 0 || index < 0 || index >= radial_menu->item_count) {
        return -1;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    if (ld_radial_menu == 0 ||
        radial_menu->widget.kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        index >= (int)radial_menu->widget.list_item_count) {
        return -1;
    }

    ldRadialMenuSetClickItem(ld_radial_menu, (uint8_t)index);
    radial_menu->widget.value = index;
    radial_menu->selected_index = index;
    return 0;
}

/**
 * @brief Set click item of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_set_click_item(tinyui_obj_t *radial_menu_obj, int index)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    return tinyui_radial_menu_click_item((tinyui_obj_t *)radial_menu, index);
}

/**
 * @brief radial menu offset item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_offset_item(tinyui_obj_t *radial_menu_obj, int offset)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return -1; }

    ldRadialMenu_t *ld_radial_menu;
    int item_count;
    int next_index;

    if (radial_menu == 0 || radial_menu->item_count <= 0) {
        return -1;
    }

    ld_radial_menu = (ldRadialMenu_t *)radial_menu->widget.ld_widget;
    if (ld_radial_menu == 0 ||
        radial_menu->widget.kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        (int)radial_menu->widget.list_item_count <= 0) {
        return -1;
    }

    item_count = (int)radial_menu->widget.list_item_count;
    next_index = (int)ld_radial_menu->selectItem;
    if (next_index < 0) {
        next_index = 0;
    }
    next_index = (next_index + offset) % item_count;
    if (next_index < 0) {
        next_index += item_count;
    }

    ldRadialMenuSetOffsetItem(ld_radial_menu, (int8_t)offset);
    if (next_index >= 0 && next_index < radial_menu->item_count) {
        radial_menu->selected_index = next_index;
    }
    return 0;
}

/**
 * @brief Set on selected of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void tinyui_radial_menu_set_on_selected(tinyui_obj_t *radial_menu_obj, void (*callback)(tinyui_obj_t *radial_menu,
                                                         int index,
                                                         void *user_data), void *user_data)
{
    struct tinyui_radial_menu *radial_menu = tinyui_radial_menu_as_radial_menu(radial_menu_obj);
    if (radial_menu == 0) { return; }

    if (radial_menu == 0) {
        return;
    }

    radial_menu->cb = (void (*)(struct tinyui_radial_menu *, int, void *))callback;
    radial_menu->user_data = user_data;
}
