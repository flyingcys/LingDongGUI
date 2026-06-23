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

#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/gui/ldSlider.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/misc/ldMsg.h"

static int tinyui_widget_accepts_event(const struct tinyui_widget *widget)
{
    return widget != 0 && widget->enabled != 0 && widget->visible != 0;
}

static int tinyui_widget_slider_value_to_percent(struct tinyui_slider *slider, int value)
{
    int range;

    if (slider == NULL) {
        return value;
    }

    range = slider->max_value - slider->min_value;
    if (range <= 0) {
        return 0;
    }

    return ((value - slider->min_value) * 100) / range;
}

static int tinyui_widget_slider_percent_to_value(struct tinyui_slider *slider, int permille)
{
    int range;
    int scaled;

    if (slider == NULL) {
        return permille;
    }

    if (permille < 0) {
        permille = 0;
    }
    if (permille > 1000) {
        permille = 1000;
    }

    range = slider->max_value - slider->min_value;
    if (range <= 0) {
        return slider->min_value;
    }

    scaled = slider->min_value + ((range * permille) / 1000);
    if (scaled < slider->min_value) {
        return slider->min_value;
    }
    if (scaled > slider->max_value) {
        return slider->max_value;
    }
    return scaled;
}

void tinyui_widget_sync_ld_value(struct tinyui_widget *widget,
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
        int percent = tinyui_widget_slider_value_to_percent((struct tinyui_slider *)widget, value);
        ldSliderSetPercent(ld_slider, (float)percent);
        break;
    }
    default:
        break;
    }
}

void tinyui_widget_emit_ld_event_bridge(struct tinyui_widget *widget,
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

static int tinyui_widget_claim_focus_for_signal(struct tinyui_widget *widget,
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

    return tinyui_widget_claim_focus(widget);
}

static void tinyui_widget_restore_rejected_list_selection(struct tinyui_widget *widget)
{
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
}

static struct tinyui_app *tinyui_widget_get_owner_app(struct tinyui_widget *widget)
{
    return tinyui_widget_owner_app(widget);
}

static void tinyui_widget_note_focus_event(struct tinyui_widget *widget,
                                           enum tinyui_focus_event event)
{
    if (widget == 0 || event == TINYUI_FOCUS_EVENT_NONE) {
        return;
    }

    widget->last_focus_event = event;
    widget->focus_change_count++;
    if (event == TINYUI_FOCUS_EVENT_ENTER) {
        widget->has_focus = 1;
        widget->focus_enter_count++;
        return;
    }

    widget->has_focus = 0;
    widget->focus_leave_count++;
}

/**
 * @brief Claim input focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_claim_focus(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;
    struct tinyui_widget *previous;

    if (widget == 0 || widget->visible == 0 || widget->enabled == 0) {
        return -1;
    }

    owner = tinyui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    previous = owner->focus_owner;
    if (previous == widget) {
        if (widget->has_focus == 0) {
            tinyui_widget_note_focus_event(widget, TINYUI_FOCUS_EVENT_ENTER);
        }
        return 0;
    }

    if (previous != 0) {
        tinyui_widget_note_focus_event(previous, TINYUI_FOCUS_EVENT_LEAVE);
    }

    owner->focus_owner = widget;
    tinyui_widget_note_focus_event(widget, TINYUI_FOCUS_EVENT_ENTER);
    return 0;
}

/**
 * @brief Widget: release focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_release_focus(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0) {
        return -1;
    }

    owner = tinyui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    if (owner->focus_owner != widget) {
        if (widget->has_focus != 0) {
            tinyui_widget_note_focus_event(widget, TINYUI_FOCUS_EVENT_LEAVE);
        }
        return 0;
    }

    owner->focus_owner = 0;
    tinyui_widget_note_focus_event(widget, TINYUI_FOCUS_EVENT_LEAVE);
    return 0;
}

/**
 * @brief Widget: is focus owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success
 */

int tinyui_widget_is_focus_owner(const struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0) {
        return 0;
    }

    owner = tinyui_widget_get_owner_app((struct tinyui_widget *)widget);
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

int tinyui_widget_mark_edit_result(struct tinyui_widget *widget, enum tinyui_edit_result result)
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

int tinyui_widget_claim_editing(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0 || widget->visible == 0 || widget->enabled == 0) {
        return -1;
    }

    owner = tinyui_widget_get_owner_app(widget);
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

int tinyui_widget_release_editing(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0) {
        return -1;
    }

    owner = tinyui_widget_get_owner_app(widget);
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

int tinyui_widget_is_editing_owner(const struct tinyui_widget *widget)
{
    struct tinyui_app *owner;

    if (widget == 0) {
        return 0;
    }

    owner = tinyui_widget_get_owner_app((struct tinyui_widget *)widget);
    if (owner == 0) {
        return 0;
    }

    return owner->editing_owner == widget;
}

int tinyui_widget_dispatch_signal(struct tinyui_widget *widget,
                                  enum tinyui_backend_signal signal,
                                  int value,
                                  tinyui_value_changed_cb cb,
                                  void *user_data)
{
    if (widget == 0) {
        return -1;
    }

    if (!tinyui_widget_accepts_event(widget)) {
        return 0;
    }

    if (signal == TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        if (widget->value == value) {
            return 0;
        }

        widget->value = value;
        tinyui_widget_sync_ld_value(widget, value);
        tinyui_widget_emit_ld_event_bridge(widget, signal, value);
        if (cb != 0) {
            tinyui_widget_emit_value_changed(cb, widget, value, user_data);
        }
        return 0;
    }

    return -1;
}

int tinyui_widget_dispatch_event(struct tinyui_widget *widget,
                                 enum tinyui_backend_signal signal,
                                 tinyui_event_cb cb,
                                 void *user_data)
{
    if (widget == 0) {
        return -1;
    }

    if (!tinyui_widget_accepts_event(widget)) {
        (void)tinyui_widget_release_focus(widget);
        return 0;
    }

    if (signal == TINYUI_BACKEND_SIGNAL_PRESSED || signal == TINYUI_BACKEND_SIGNAL_RELEASED) {
        if (tinyui_widget_claim_focus_for_signal(widget, signal) != 0) {
            return -1;
        }
        tinyui_widget_emit_event(cb, widget, user_data);
        return 0;
    }

    return -1;
}

int tinyui_widget_dispatch_native_signal(struct tinyui_widget *widget,
                                         uint32_t native_signal,
                                         uint64_t native_value)
{
    if (widget == 0) {
        return -1;
    }

    if (widget->enabled == 0 || widget->visible == 0) {
        tinyui_widget_restore_rejected_list_selection(widget);
        (void)tinyui_widget_release_focus(widget);
        return 0;
    }

    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_BUTTON: {
        struct tinyui_button *button = (struct tinyui_button *)widget;

        if (native_signal == SIGNAL_PRESS) {
            return tinyui_widget_dispatch_event(widget,
                                                TINYUI_BACKEND_SIGNAL_PRESSED,
                                                button->on_pressed,
                                                button->on_pressed_user_data);
        }
        if (native_signal == SIGNAL_HOLD_DOWN) {
            return 0;
        }
        if (native_signal == SIGNAL_RELEASE) {
            int rc;

            rc = tinyui_widget_dispatch_event(widget,
                                              TINYUI_BACKEND_SIGNAL_RELEASED,
                                              button->on_released,
                                              button->on_released_user_data);
            if (rc != 0) {
                return rc;
            }

            if (button->on_clicked != 0) {
                tinyui_widget_emit_clicked(button->on_clicked,
                                           widget,
                                           button->user_data);
            }
            return 0;
        }
        return -1;
    }
    case TINYUI_BACKEND_WIDGET_KEYBOARD: {
        struct tinyui_keyboard *keyboard = (struct tinyui_keyboard *)widget;
        unsigned int key_code = (unsigned int)tinyui_keyboard_get_selected_key_code(keyboard);

        if (native_signal == SIGNAL_PRESS || native_signal == SIGNAL_RELEASE) {
            if (tinyui_widget_claim_focus_for_signal(widget,
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
        return -1;
    }
    case TINYUI_BACKEND_WIDGET_CHECKBOX: {
        struct tinyui_checkbox *checkbox = (struct tinyui_checkbox *)widget;
        int normalized_value;

        if (native_signal != SIGNAL_VALUE_CHANGED) {
            return -1;
        }

        normalized_value = native_value != 0;
        if (tinyui_widget_claim_focus_for_signal(widget,
                                                 TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) != 0) {
            return -1;
        }
        if (checkbox->checked == normalized_value && widget->value == normalized_value) {
            return 0;
        }
        checkbox->checked = normalized_value;
        widget->value = normalized_value;
        tinyui_widget_sync_ld_value(widget, normalized_value);
        tinyui_widget_emit_value_changed(checkbox->cb,
                                         widget,
                                         normalized_value,
                                         checkbox->user_data);
        return 0;
    }
    case TINYUI_BACKEND_WIDGET_SWITCH: {
        struct tinyui_switch *sw = (struct tinyui_switch *)widget;
        int normalized_value;

        if (native_signal != SIGNAL_VALUE_CHANGED) {
            return -1;
        }

        normalized_value = native_value != 0;
        if (tinyui_widget_claim_focus_for_signal(widget,
                                                 TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) != 0) {
            return -1;
        }
        if (sw->checked == normalized_value && widget->value == normalized_value) {
            return 0;
        }
        sw->checked = normalized_value;
        widget->value = normalized_value;
        tinyui_widget_sync_ld_value(widget, normalized_value);
        tinyui_widget_emit_value_changed(sw->cb,
                                         widget,
                                         normalized_value,
                                         sw->user_data);
        return 0;
    }
    case TINYUI_BACKEND_WIDGET_SLIDER: {
        struct tinyui_slider *slider = (struct tinyui_slider *)widget;
        int widget_value;

        if (native_signal != SIGNAL_VALUE_CHANGED) {
            return -1;
        }

        widget_value = tinyui_widget_slider_percent_to_value(slider, (int)native_value);
        if (tinyui_widget_claim_focus_for_signal(widget,
                                                 TINYUI_BACKEND_SIGNAL_VALUE_CHANGED) != 0) {
            return -1;
        }
        if (slider->value == widget_value && widget->value == widget_value) {
            return 0;
        }
        slider->value = widget_value;
        widget->value = widget_value;
        tinyui_widget_sync_ld_value(widget, widget_value);
        tinyui_widget_emit_value_changed(slider->cb,
                                         widget,
                                         widget_value,
                                         slider->user_data);
        return 0;
    }
    case TINYUI_BACKEND_WIDGET_LIST: {
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
        if (tinyui_widget_claim_focus_for_signal(widget,
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
        return 0;
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
