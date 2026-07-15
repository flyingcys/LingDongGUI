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

#include "core/event.h"
#include "core/focus.h"
#include "integration/input.h"

#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/gui/ldSlider.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/misc/ldMsg.h"

#include <stdint.h>
#include <string.h>

static int tinyui_runtime_internal_widget_accepts_event(const struct tinyui_widget *widget)
{
    return widget != 0 && widget->enabled != 0 && widget->visible != 0;
}

static int tinyui_runtime_internal_widget_slider_value_to_percent(struct tinyui_slider *slider, int value)
{
    int64_t range;

    if (slider == NULL) {
        return value;
    }

    range = (int64_t)slider->max_value - (int64_t)slider->min_value;
    if (range <= 0) {
        return 0;
    }

    return (int)((((int64_t)value - (int64_t)slider->min_value) * 100) / range);
}

static int tinyui_runtime_internal_widget_slider_percent_to_value(struct tinyui_slider *slider, int permille)
{
    int64_t range;
    int64_t scaled;

    if (slider == NULL) {
        return permille;
    }

    if (permille < 0) {
        permille = 0;
    }
    if (permille > 1000) {
        permille = 1000;
    }

    range = (int64_t)slider->max_value - (int64_t)slider->min_value;
    if (range <= 0) {
        return slider->min_value;
    }

    scaled = (int64_t)slider->min_value + ((range * (int64_t)permille) / 1000);
    if (scaled < (int64_t)slider->min_value) {
        return slider->min_value;
    }
    if (scaled > (int64_t)slider->max_value) {
        return slider->max_value;
    }
    return (int)scaled;
}

void tinyui_runtime_internal_widget_sync_ld_value(struct tinyui_widget *widget,
                                 int value)
{
    if (widget == NULL || widget->ld_widget == NULL) {
        return;
    }

    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_CHECKBOX: {
        ldCheckBox_t *ld_checkbox = (ldCheckBox_t *)widget->ld_widget;
        ld_checkbox->isChecked = value != 0;
        ld_checkbox->use_as__ldBase_t.isDirtyRegionUpdate = true;
        break;
    }
    case TINYUI_BACKEND_WIDGET_SWITCH: {
        ldSwitch_t *ld_switch = (ldSwitch_t *)widget->ld_widget;
        uint16_t progress = value != 0 ? 1000U : 0U;
        ld_switch->isChecked = value != 0;
        ld_switch->animProgress = progress;
        ld_switch->animStartProgress = progress;
        ld_switch->animTargetProgress = progress;
        ld_switch->animElapsedMs = 0U;
        ld_switch->isAnimating = false;
        ld_switch->use_as__ldBase_t.isDirtyRegionUpdate = true;
        break;
    }
    case TINYUI_BACKEND_WIDGET_SLIDER: {
        ldSlider_t *ld_slider = (ldSlider_t *)widget->ld_widget;
        int percent = tinyui_runtime_internal_widget_slider_value_to_percent((struct tinyui_slider *)widget, value);
        ldSliderSetPercent(ld_slider, (float)percent);
        break;
    }
    default:
        break;
    }
}

void tinyui_runtime_internal_widget_emit_ld_event_bridge(struct tinyui_widget *widget,
                                        enum tinyui_backend_signal signal,
                                        int value)
{
    if (widget == 0 || widget->ld_event_bridge_scene == 0 || widget->ld_event_bridge_sender == 0) {
        return;
    }

    if (signal != TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        return;
    }

    if (widget->ld_event_bridge_scene->ptMsgQueue == 0) {
        return;
    }

    ldMsgEmit(widget->ld_event_bridge_scene->ptMsgQueue,
              widget->ld_event_bridge_sender,
              SIGNAL_VALUE_CHANGED,
              (uint64_t)value);
}

static int tinyui_runtime_internal_widget_claim_focus_for_signal(struct tinyui_widget *widget,
                                                enum tinyui_backend_signal signal)
{
    if (widget == 0) {
        return -1;
    }

    if (signal != TINYUI_BACKEND_SIGNAL_PRESSED &&
        signal != TINYUI_BACKEND_SIGNAL_RELEASED &&
        signal != TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        return 0;
    }

    return tinyui_runtime_internal_widget_claim_focus(widget);
}

static void tinyui_runtime_internal_widget_restore_rejected_list_selection(struct tinyui_widget *widget)
{
#if TINYUI_ENABLE_LIST
    struct tinyui_list *list;

    if (widget == NULL || widget->kind != TINYUI_BACKEND_WIDGET_LIST) {
        return;
    }

    list = (struct tinyui_list *)widget;
    if (list->selected_index >= 0 && list->selected_index < list->item_count) {
        (void)tinyui_list_set_selected_index_ld(widget, list->selected_index);
        return;
    }

    if (widget->ld_widget != NULL) {
        ldListSetSelectItem((ldList_t *)widget->ld_widget, -1);
    }
    widget->value = -1;
#else
    (void)widget;
#endif
}

static struct tinyui_app *tinyui_runtime_internal_widget_get_owner_app(struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_owner_app(widget);
}

static void tinyui_event_fire(struct tinyui_widget *widget,
                              tinyui_event_code_t code,
                              int32_t value,
                              uint16_t key,
                              bool key_pressed);

static void tinyui_runtime_internal_widget_note_focus_event(struct tinyui_widget *widget,
                                           enum tinyui_focus_event event)
{
    if (widget == 0 || event == TINYUI_FOCUS_EVENT_NONE) {
        return;
    }

    widget->last_focus_event = event;
    if (widget->focus_change_count != UINT16_MAX) {
        widget->focus_change_count++;
    }
    if (event == TINYUI_FOCUS_EVENT_ENTER) {
        widget->has_focus = 1;
        if (widget->focus_enter_count != UINT16_MAX) {
            widget->focus_enter_count++;
        }
        tinyui_event_fire(widget, TINYUI_EVENT_FOCUSED, 0, 0U, false);
        return;
    }

    widget->has_focus = 0;
    if (widget->focus_leave_count != UINT16_MAX) {
        widget->focus_leave_count++;
    }
    tinyui_event_fire(widget, TINYUI_EVENT_DEFOCUSED, 0, 0U, false);
}

/**
 * @brief Claim input focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_runtime_internal_widget_claim_focus(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;
    struct tinyui_widget *previous;

    if (widget == 0 || widget->visible == 0 || widget->enabled == 0) {
        return -1;
    }

    owner = tinyui_runtime_internal_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    previous = owner->focus_owner;
    if (previous == widget) {
        if (widget->has_focus == 0) {
            tinyui_runtime_internal_widget_note_focus_event(widget, TINYUI_FOCUS_EVENT_ENTER);
        }
        return 0;
    }

    if (previous != 0) {
        tinyui_runtime_internal_widget_note_focus_event(previous, TINYUI_FOCUS_EVENT_LEAVE);
    }

    owner->focus_owner = widget;
    tinyui_runtime_internal_widget_note_focus_event(widget, TINYUI_FOCUS_EVENT_ENTER);
    return 0;
}

/**
 * @brief Widget: release focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_runtime_internal_widget_release_focus(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0) {
        return -1;
    }

    owner = tinyui_runtime_internal_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    if (owner->focus_owner != widget) {
        if (widget->has_focus != 0) {
            tinyui_runtime_internal_widget_note_focus_event(widget, TINYUI_FOCUS_EVENT_LEAVE);
        }
        return 0;
    }

    owner->focus_owner = 0;
    tinyui_runtime_internal_widget_note_focus_event(widget, TINYUI_FOCUS_EVENT_LEAVE);
    return 0;
}

/**
 * @brief Widget: is focus owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success
 */

int tinyui_runtime_internal_widget_is_focus_owner(const struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0) {
        return 0;
    }

    owner = tinyui_runtime_internal_widget_get_owner_app((struct tinyui_widget *)widget);
    if (owner == 0) {
        return 0;
    }

    return owner->focus_owner == widget;
}

/**
 * @brief Widget: mark edit result
 *
 * @param[in] widget Widget instance
 * @param[in] result result
 * @return 0 on success, -1 on failure
 */

int tinyui_runtime_internal_widget_mark_edit_result(struct tinyui_widget *widget, enum tinyui_edit_result result)
{
    if (widget == 0) {
        return -1;
    }

    if (result != TINYUI_EDIT_RESULT_COMMIT && result != TINYUI_EDIT_RESULT_CANCEL) {
        return -1;
    }

    widget->pending_edit_result = result;
    return 0;
}

/**
 * @brief Widget: claim editing
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_runtime_internal_widget_claim_editing(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0 || widget->visible == 0 || widget->enabled == 0) {
        return -1;
    }

    owner = tinyui_runtime_internal_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    widget->pending_edit_result = TINYUI_EDIT_RESULT_NONE;
    owner->editing_owner = widget;
    return 0;
}

/**
 * @brief Widget: release editing
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_runtime_internal_widget_release_editing(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0) {
        return -1;
    }

    owner = tinyui_runtime_internal_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    if (owner->editing_owner == widget) {
        owner->editing_owner = 0;
    }
    widget->last_edit_result = widget->pending_edit_result;
    widget->pending_edit_result = TINYUI_EDIT_RESULT_NONE;
    return 0;
}

/**
 * @brief Widget: is editing owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success
 */

int tinyui_runtime_internal_widget_is_editing_owner(const struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0) {
        return 0;
    }

    owner = tinyui_runtime_internal_widget_get_owner_app((struct tinyui_widget *)widget);
    if (owner == 0) {
        return 0;
    }

    return owner->editing_owner == widget;
}

int tinyui_runtime_internal_widget_dispatch_signal(struct tinyui_widget *widget,
                                  enum tinyui_backend_signal signal,
                                  int value,
                                  tinyui_value_changed_cb cb,
                                  void *user_data)
{
    if (widget == 0) {
        return -1;
    }

    if (!tinyui_runtime_internal_widget_accepts_event(widget)) {
        return 0;
    }

    if (signal == TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        if (widget->value == value) {
            return 0;
        }

        widget->value = value;
        tinyui_runtime_internal_widget_sync_ld_value(widget, value);
        tinyui_runtime_internal_widget_emit_ld_event_bridge(widget, signal, value);
        if (cb != 0) {
            tinyui_runtime_internal_widget_emit_value_changed(cb, widget, value, user_data);
        }
        return 0;
    }

    return -1;
}

int tinyui_runtime_internal_widget_dispatch_event(struct tinyui_widget *widget,
                                 enum tinyui_backend_signal signal,
                                 tinyui_event_cb cb,
                                 void *user_data)
{
    if (widget == 0) {
        return -1;
    }

    if (!tinyui_runtime_internal_widget_accepts_event(widget)) {
        (void)tinyui_runtime_internal_widget_release_focus(widget);
        return 0;
    }

    if (signal == TINYUI_BACKEND_SIGNAL_PRESSED || signal == TINYUI_BACKEND_SIGNAL_RELEASED) {
        if (tinyui_runtime_internal_widget_claim_focus_for_signal(widget, signal) != 0) {
            return -1;
        }
        tinyui_runtime_internal_widget_emit_event(cb, widget, user_data);
        return 0;
    }

    return -1;
}

int tinyui_runtime_internal_widget_dispatch_native_signal(struct tinyui_widget *widget,
                                         uint32_t native_signal,
                                         uint64_t native_value)
{
    if (widget == 0) {
        return -1;
    }

    if (widget->enabled == 0 || widget->visible == 0) {
        tinyui_runtime_internal_widget_restore_rejected_list_selection(widget);
        (void)tinyui_runtime_internal_widget_release_focus(widget);
        return 0;
    }

    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_BUTTON: {
        /* Task 6: only unified event pool; no legacy on_* fields. */
        if (native_signal == SIGNAL_PRESS) {
            if (!tinyui_runtime_internal_widget_accepts_event(widget)) {
                (void)tinyui_runtime_internal_widget_release_focus(widget);
                return 0;
            }
            if (tinyui_runtime_internal_widget_claim_focus_for_signal(
                    widget, TINYUI_BACKEND_SIGNAL_PRESSED) != 0) {
                return -1;
            }
            tinyui_event_fire(widget, TINYUI_EVENT_PRESSED, 0, 0U, false);
            return 0;
        }
        if (native_signal == SIGNAL_HOLD_DOWN) {
            return 0;
        }
        if (native_signal == SIGNAL_RELEASE) {
            if (!tinyui_runtime_internal_widget_accepts_event(widget)) {
                (void)tinyui_runtime_internal_widget_release_focus(widget);
                return 0;
            }
            if (tinyui_runtime_internal_widget_claim_focus_for_signal(
                    widget, TINYUI_BACKEND_SIGNAL_RELEASED) != 0) {
                return -1;
            }
            tinyui_event_fire(widget, TINYUI_EVENT_RELEASED, 0, 0U, false);
            if (widget->deleting == 0U) {
                tinyui_event_fire(widget, TINYUI_EVENT_CLICKED, 0, 0U, false);
            }
            return 0;
        }
        return -1;
    }
    case TINYUI_BACKEND_WIDGET_KEYBOARD: {
#if TINYUI_ENABLE_KEYBOARD
        struct tinyui_keyboard *keyboard = (struct tinyui_keyboard *)widget;
        unsigned int key_code = (unsigned int)tinyui_keyboard_get_selected_key_code(keyboard);

        if (native_signal == SIGNAL_PRESS || native_signal == SIGNAL_RELEASE) {
            if (tinyui_runtime_internal_widget_claim_focus_for_signal(widget,
                                                     native_signal == SIGNAL_PRESS
                                                         ? TINYUI_BACKEND_SIGNAL_PRESSED
                                                         : TINYUI_BACKEND_SIGNAL_RELEASED) != 0) {
                return -1;
            }
            if (keyboard->event_cb != 0) {
                keyboard->event_cb(keyboard, key_code, native_signal, keyboard->event_user_data);
            }
            return 0;
        }
#endif
        return -1;
    }
    case TINYUI_BACKEND_WIDGET_CHECKBOX: {
        struct tinyui_checkbox *checkbox = (struct tinyui_checkbox *)widget;
        int normalized_value;

        if (native_signal != SIGNAL_VALUE_CHANGED) {
            return -1;
        }

        normalized_value = native_value != 0;
        if (tinyui_runtime_internal_widget_claim_focus_for_signal(widget,
                                                 TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) != 0) {
            return -1;
        }
        if (checkbox->checked == normalized_value && widget->value == normalized_value) {
            return 0;
        }
        checkbox->checked = normalized_value;
        widget->value = normalized_value;
        tinyui_runtime_internal_widget_sync_ld_value(widget, normalized_value);
        /* Task 7: only unified event pool; no dedicated cb field. */
        tinyui_event_fire(widget,
                          TINYUI_EVENT_VALUE_CHANGED,
                          (int32_t)normalized_value,
                          0U,
                          false);
        return 0;
    }
    case TINYUI_BACKEND_WIDGET_SWITCH: {
        struct tinyui_switch *sw = (struct tinyui_switch *)widget;
        int normalized_value;

        if (native_signal != SIGNAL_VALUE_CHANGED) {
            return -1;
        }

        normalized_value = native_value != 0;
        if (tinyui_runtime_internal_widget_claim_focus_for_signal(widget,
                                                 TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) != 0) {
            return -1;
        }
        if (sw->checked == normalized_value && widget->value == normalized_value) {
            return 0;
        }
        sw->checked = normalized_value;
        widget->value = normalized_value;
        tinyui_runtime_internal_widget_sync_ld_value(widget, normalized_value);
        /* M3 Task 2: only unified event pool; no dedicated cb field. */
        tinyui_event_fire(widget,
                          TINYUI_EVENT_VALUE_CHANGED,
                          (int32_t)normalized_value,
                          0U,
                          false);
        return 0;
    }
    case TINYUI_BACKEND_WIDGET_SLIDER: {
        struct tinyui_slider *slider = (struct tinyui_slider *)widget;
        int widget_value;

        if (native_signal != SIGNAL_VALUE_CHANGED) {
            return -1;
        }

        widget_value = tinyui_runtime_internal_widget_slider_percent_to_value(slider, (int)native_value);
        if (tinyui_runtime_internal_widget_claim_focus_for_signal(widget,
                                                 TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) != 0) {
            return -1;
        }
        if (slider->value == widget_value && widget->value == widget_value) {
            return 0;
        }
        slider->value = widget_value;
        widget->value = widget_value;
        tinyui_runtime_internal_widget_sync_ld_value(widget, widget_value);
        /* Task 7: only unified event pool; no dedicated cb field. */
        tinyui_event_fire(widget,
                          TINYUI_EVENT_VALUE_CHANGED,
                          (int32_t)widget_value,
                          0U,
                          false);
        return 0;
    }
    case TINYUI_BACKEND_WIDGET_LIST: {
#if !TINYUI_ENABLE_LIST
        return -1;
#else
        struct tinyui_list *list = (struct tinyui_list *)widget;
        int selected_index;
        int was_selected_index;
        int was_widget_value;

        if (native_signal != SIGNAL_CLICKED_ITEM) {
            return -1;
        }

        selected_index = (int)native_value;
        if (selected_index < 0 || selected_index >= list->item_count) {
            return 0;
        }
        was_selected_index = list->selected_index;
        was_widget_value = widget->value;
        if (tinyui_runtime_internal_widget_claim_focus_for_signal(widget,
                                                 TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) != 0) {
            return -1;
        }

        if (tinyui_list_set_selected_index_ld(widget, selected_index) != 0) {
            return -1;
        }
        if (tinyui_list_sync_selected_index(list, &selected_index) != 0) {
            return -1;
        }
        if (was_selected_index == selected_index && was_widget_value == selected_index) {
            return 0;
        }
        if (list->cb != 0) {
            list->cb(list, selected_index, list->user_data);
        }
        tinyui_event_fire(widget,
                          TINYUI_EVENT_VALUE_CHANGED,
                          (int32_t)selected_index,
                          0U,
                          false);
        return 0;
#endif
    }
    default:
        break;
    }

    return -1;
}

/**
 * @brief Event: stub
 *
 * @return 0 on success
 */

int tinyui_event_stub(void)
{
    return 0;
}

/* ── M2 Task 4: fixed event-callback pool ───────────────────────────────── */

static tinyui_event_handle_t tinyui_event_handle_encode(uint16_t slot_index,
                                                       uint16_t generation)
{
    return ((tinyui_event_handle_t)generation << 16) |
           (tinyui_event_handle_t)((uint16_t)(slot_index + 1U));
}

static int tinyui_event_handle_decode(tinyui_event_handle_t handle,
                                      uint16_t *slot_index_out,
                                      uint16_t *generation_out)
{
    uint16_t slot_plus_one;
    uint16_t generation;

    if (handle == 0U || slot_index_out == 0 || generation_out == 0) {
        return -1;
    }
    slot_plus_one = (uint16_t)(handle & UINT32_C(0xffff));
    generation = (uint16_t)(handle >> 16);
    if (slot_plus_one == 0U || generation == 0U) {
        return -1;
    }
    if ((uint32_t)slot_plus_one - 1U >= (uint32_t)TINYUI_EVENT_CB_CAPACITY) {
        return -1;
    }
    *slot_index_out = (uint16_t)(slot_plus_one - 1U);
    *generation_out = generation;
    return 0;
}

static void tinyui_event_slot_release(struct tinyui_event_callback_pool *pool,
                                      struct tinyui_event_callback_slot *slot)
{
    if (pool == 0 || slot == 0 || slot->allocated == 0U) {
        return;
    }
    slot->allocated = 0U;
    slot->object = 0;
    slot->event_mask = 0U;
    slot->cb = 0;
    slot->user_data = 0;
    slot->registration_order = 0U;
    slot->born_epoch = 0U;
    /* keep generation so next encode differs after reuse */
    if (pool->active_count > 0U) {
        pool->active_count = (uint16_t)(pool->active_count - 1U);
    }
}

static void tinyui_event_clear_object_slots(struct tinyui_widget *widget)
{
    struct tinyui_runtime_state *rt = tinyui_runtime_state_get();
    struct tinyui_event_callback_pool *pool;
    size_t i;

    if (rt == 0 || widget == 0) {
        return;
    }
    pool = &rt->event_cb_pool;
    for (i = 0; i < (size_t)TINYUI_EVENT_CB_CAPACITY; ++i) {
        struct tinyui_event_callback_slot *slot = &pool->slots[i];
        if (slot->allocated != 0U && slot->object == (tinyui_obj_t *)(void *)widget) {
            tinyui_event_slot_release(pool, slot);
        }
    }
}

static int tinyui_event_slot_cmp_order(const void *a, const void *b)
{
    const struct tinyui_event_callback_slot *const *sa =
        (const struct tinyui_event_callback_slot *const *)a;
    const struct tinyui_event_callback_slot *const *sb =
        (const struct tinyui_event_callback_slot *const *)b;

    if ((*sa)->registration_order < (*sb)->registration_order) {
        return -1;
    }
    if ((*sa)->registration_order > (*sb)->registration_order) {
        return 1;
    }
    return 0;
}

static void tinyui_event_fire(struct tinyui_widget *widget,
                              tinyui_event_code_t code,
                              int32_t value,
                              uint16_t key,
                              bool key_pressed)
{
    struct tinyui_runtime_state *rt = tinyui_runtime_state_get();
    struct tinyui_event_callback_pool *pool;
    struct tinyui_event_callback_slot *ordered[TINYUI_EVENT_CB_CAPACITY];
    size_t ordered_count = 0U;
    size_t i;
    uint16_t epoch;
    uint32_t mask;
    tinyui_obj_t *target;

    if (rt == 0 || widget == 0 || !rt->initialized) {
        return;
    }
    if (code < TINYUI_EVENT_PRESSED || code > TINYUI_EVENT_DELETE) {
        return;
    }

    pool = &rt->event_cb_pool;
    target = (tinyui_obj_t *)(void *)widget;
    mask = TINYUI_EVENT_MASK(code);

    epoch = (uint16_t)(pool->dispatch_epoch + 1U);
    if (epoch == 0U) {
        epoch = 1U;
    }
    pool->dispatch_epoch = epoch;
    /* Align timer-style global epoch so create-in-callback is next-round. */
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

    /* Sort by registration_order without heap (insertion sort on fixed array). */
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
    (void)tinyui_event_slot_cmp_order;

    {
        bool was_processing = rt->processing;
        /* Defer obj_delete during callbacks to avoid free mid-iteration. */
        rt->processing = true;

        for (i = 0; i < ordered_count; ++i) {
            struct tinyui_event_callback_slot *slot = ordered[i];
            uint16_t generation;
            tinyui_event_cb_t cb;
            tinyui_event_t event;

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
            /* Target deleted mid-dispatch: stop subsequent callbacks. */
            if (widget->deleting != 0U && code != TINYUI_EVENT_DELETE) {
                break;
            }

            generation = slot->generation;
            cb = slot->cb;

            memset(&event, 0, sizeof(event));
            event.code = code;
            event.target = target;
            event.user_data = slot->user_data;
            if (code == TINYUI_EVENT_VALUE_CHANGED) {
                event.data.value = value;
            } else if (code == TINYUI_EVENT_KEY) {
                event.data.key.key = key;
                event.data.key.pressed = key_pressed;
            }

            cb(&event);

            if (slot->allocated == 0U || slot->generation != generation) {
                continue;
            }
            if (widget->deleting != 0U && code != TINYUI_EVENT_DELETE) {
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
}

void tinyui_event_emit_delete(struct tinyui_widget *widget)
{
    if (widget == 0) {
        return;
    }
    /* deleting must already be set by destroy path */
    widget->deleting = 1;
    tinyui_event_fire(widget, TINYUI_EVENT_DELETE, 0, 0U, false);
    tinyui_event_clear_object_slots(widget);
}

tinyui_result_t tinyui_obj_add_event_cb(tinyui_obj_t *obj,
                                        uint32_t event_mask,
                                        tinyui_event_cb_t cb,
                                        void *user_data,
                                        tinyui_event_handle_t *handle)
{
    struct tinyui_runtime_state *rt = tinyui_runtime_state_get();
    struct tinyui_event_callback_pool *pool;
    struct tinyui_widget *widget;
    struct tinyui_event_callback_slot *slot = 0;
    size_t i;
    uint16_t generation;
    uint16_t reg_order;

    if (handle != 0) {
        *handle = 0U;
    }
    if (rt == 0 || !rt->initialized) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }
    widget = (struct tinyui_widget *)(void *)obj;
    if (widget == 0 || cb == 0 || event_mask == 0U) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return TINYUI_ERROR_INVALID_ARG;
    }
    if (widget->deleting != 0U) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }

    pool = &rt->event_cb_pool;
    for (i = 0; i < (size_t)TINYUI_EVENT_CB_CAPACITY; ++i) {
        if (pool->slots[i].allocated == 0U) {
            slot = &pool->slots[i];
            break;
        }
    }
    if (slot == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return TINYUI_ERROR_CAPACITY;
    }

    generation = (uint16_t)(pool->next_generation + 1U);
    if (generation == 0U) {
        generation = 1U;
    }
    pool->next_generation = generation;

    reg_order = (uint16_t)(pool->next_registration_order + 1U);
    if (reg_order == 0U) {
        reg_order = 1U;
    }
    pool->next_registration_order = reg_order;

    memset(slot, 0, sizeof(*slot));
    slot->object = obj;
    slot->event_mask = event_mask;
    slot->cb = cb;
    slot->user_data = user_data;
    slot->generation = generation;
    slot->registration_order = reg_order;
    slot->born_epoch = pool->dispatch_epoch;
    slot->allocated = 1U;
    pool->active_count = (uint16_t)(pool->active_count + 1U);

    if (handle != 0) {
        *handle = tinyui_event_handle_encode((uint16_t)(slot - pool->slots), generation);
    }
    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

tinyui_result_t tinyui_obj_remove_event_cb(tinyui_obj_t *obj,
                                           tinyui_event_handle_t handle)
{
    struct tinyui_runtime_state *rt = tinyui_runtime_state_get();
    struct tinyui_event_callback_pool *pool;
    struct tinyui_event_callback_slot *slot;
    uint16_t slot_index = 0U;
    uint16_t generation = 0U;

    if (rt == 0 || !rt->initialized) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }
    if (obj == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_OBJECT);
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (tinyui_event_handle_decode(handle, &slot_index, &generation) != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return TINYUI_ERROR_INVALID_ARG;
    }

    pool = &rt->event_cb_pool;
    slot = &pool->slots[slot_index];
    if (slot->allocated == 0U ||
        slot->generation != generation ||
        slot->object != obj) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return TINYUI_ERROR_INVALID_ARG;
    }

    tinyui_event_slot_release(pool, slot);
    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

tinyui_result_t tinyui_focus_set(tinyui_obj_t *obj)
{
    int result;

    if (obj == NULL) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_OBJECT);
        return TINYUI_ERROR_INVALID_OBJECT;
    }

    result = tinyui_runtime_internal_widget_claim_focus((struct tinyui_widget *)obj);
    if (result != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return TINYUI_ERROR_BACKEND;
    }

    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

tinyui_result_t tinyui_focus_clear(void)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_current();

    if (app == NULL) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }
    if (app->focus_owner != NULL && tinyui_runtime_internal_widget_release_focus(app->focus_owner) != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return TINYUI_ERROR_BACKEND;
    }
    if (tinyui_focus_reset(app) != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return TINYUI_ERROR_BACKEND;
    }

    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

tinyui_result_t tinyui_focus_move(tinyui_focus_direction_t direction)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_current();
    enum tinyui_native_nav_dir native_direction;
    int result;

    if (app == NULL) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }

    switch (direction) {
    case TINYUI_FOCUS_LEFT:
        native_direction = TINYUI_NATIVE_NAV_LEFT;
        break;
    case TINYUI_FOCUS_RIGHT:
        native_direction = TINYUI_NATIVE_NAV_RIGHT;
        break;
    case TINYUI_FOCUS_UP:
        native_direction = TINYUI_NATIVE_NAV_UP;
        break;
    case TINYUI_FOCUS_DOWN:
        native_direction = TINYUI_NATIVE_NAV_DOWN;
        break;
    case TINYUI_FOCUS_NEXT:
    case TINYUI_FOCUS_PREVIOUS:
        tinyui_runtime_set_last_result(TINYUI_ERROR_NOT_SUPPORTED);
        return TINYUI_ERROR_NOT_SUPPORTED;
    default:
        tinyui_runtime_set_last_result(TINYUI_ERROR_OUT_OF_RANGE);
        return TINYUI_ERROR_OUT_OF_RANGE;
    }

    result = tinyui_focus_navigate(app, native_direction);
    tinyui_runtime_set_last_result(result == 0 ? TINYUI_OK : TINYUI_ERROR_BACKEND);
    return result == 0 ? TINYUI_OK : TINYUI_ERROR_BACKEND;
}

tinyui_obj_t *tinyui_focus_current(void)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_current();

    return app == NULL ? NULL : (tinyui_obj_t *)app->focus_owner;
}

tinyui_result_t tinyui_input_send_key(tinyui_key_t key, bool pressed)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_current();
    enum tinyui_input_key input_key;
    enum tinyui_native_nav_dir navigation_direction;

    if (app == NULL) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }

    switch (key) {
    case TINYUI_KEY_ENTER:
        input_key = TINYUI_INPUT_KEY_ENTER;
        navigation_direction = TINYUI_NATIVE_NAV_ENTER;
        break;
    case TINYUI_KEY_BACK:
        input_key = TINYUI_INPUT_KEY_BACK;
        navigation_direction = TINYUI_NATIVE_NAV_BACK;
        break;
    case TINYUI_KEY_LEFT:
        input_key = TINYUI_INPUT_KEY_LEFT;
        navigation_direction = TINYUI_NATIVE_NAV_LEFT;
        break;
    case TINYUI_KEY_RIGHT:
        input_key = TINYUI_INPUT_KEY_RIGHT;
        navigation_direction = TINYUI_NATIVE_NAV_RIGHT;
        break;
    case TINYUI_KEY_UP:
        input_key = TINYUI_INPUT_KEY_UP;
        navigation_direction = TINYUI_NATIVE_NAV_UP;
        break;
    case TINYUI_KEY_DOWN:
        input_key = TINYUI_INPUT_KEY_DOWN;
        navigation_direction = TINYUI_NATIVE_NAV_DOWN;
        break;
    case TINYUI_KEY_NEXT:
    case TINYUI_KEY_PREVIOUS:
        tinyui_runtime_set_last_result(TINYUI_ERROR_NOT_SUPPORTED);
        return TINYUI_ERROR_NOT_SUPPORTED;
    default:
        tinyui_runtime_set_last_result(TINYUI_ERROR_OUT_OF_RANGE);
        return TINYUI_ERROR_OUT_OF_RANGE;
    }

    if (tinyui_input_push_key(app, input_key, pressed) != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return TINYUI_ERROR_BACKEND;
    }

    /* Unified KEY event to current focus (no second queue). */
    if (app->focus_owner != 0) {
        tinyui_event_fire(app->focus_owner,
                          TINYUI_EVENT_KEY,
                          0,
                          (uint16_t)key,
                          pressed);
    }

    if (!pressed) {
        tinyui_runtime_set_last_result(TINYUI_OK);
        return TINYUI_OK;
    }

    if (tinyui_focus_navigate(app, navigation_direction) != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return TINYUI_ERROR_BACKEND;
    }

    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}
