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

#include "../../../../src/gui/ldCheckBox.h"
#include "../../../../src/gui/ldSlider.h"
#include "../../../../src/gui/ldSwitch.h"
#include "../../../../src/misc/ldMsg.h"

static int picoui_widget_accepts_event(const struct picoui_widget *widget)
{
    return widget != 0 && widget->enabled != 0 && widget->visible != 0;
}

static int picoui_widget_slider_value_to_percent(struct picoui_slider *slider, int value)
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

void picoui_widget_sync_ld_value(struct picoui_backend_widget *backend,
                                 struct picoui_widget *widget,
                                 int value)
{
    if (backend == NULL || backend->ld_widget == NULL) {
        return;
    }

    switch (backend->kind) {
    case PICOUI_BACKEND_WIDGET_CHECKBOX: {
        ldCheckBox_t *ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
        ld_checkbox->isChecked = value != 0;
        ld_checkbox->use_as__ldBase_t.isDirtyRegionUpdate = true;
        break;
    }
    case PICOUI_BACKEND_WIDGET_SWITCH: {
        ldSwitch_t *ld_switch = (ldSwitch_t *)backend->ld_widget;
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
    case PICOUI_BACKEND_WIDGET_SLIDER: {
        ldSlider_t *ld_slider = (ldSlider_t *)backend->ld_widget;
        int percent = picoui_widget_slider_value_to_percent((struct picoui_slider *)widget, value);
        ldSliderSetPercent(ld_slider, (float)percent);
        break;
    }
    default:
        break;
    }
}

void picoui_widget_emit_ld_event_bridge(struct picoui_backend_widget *backend,
                                        enum picoui_backend_signal signal,
                                        int value)
{
    if (backend == 0 || backend->ld_event_bridge_scene == 0 || backend->ld_event_bridge_sender == 0) {
        return;
    }

    if (signal != PICOUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        return;
    }

    if (backend->ld_event_bridge_scene->ptMsgQueue == 0) {
        return;
    }

    ldMsgEmit(backend->ld_event_bridge_scene->ptMsgQueue,
              backend->ld_event_bridge_sender,
              SIGNAL_VALUE_CHANGED,
              (uint64_t)value);
}

static int picoui_widget_claim_focus_for_signal(struct picoui_backend_widget *backend,
                                                enum picoui_backend_signal signal)
{
    if (backend == 0) {
        return -1;
    }

    if (signal != PICOUI_BACKEND_SIGNAL_PRESSED &&
        signal != PICOUI_BACKEND_SIGNAL_RELEASED &&
        signal != PICOUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        return 0;
    }

    return picoui_backend_widget_claim_focus(backend);
}
static struct picoui_app *picoui_widget_get_owner_app(struct picoui_widget *widget)
{
    return picoui_widget_owner_app(widget);
}

static void picoui_widget_note_focus_event(struct picoui_widget *widget,
                                           enum picoui_focus_event event)
{
    if (widget == 0 || event == PICOUI_FOCUS_EVENT_NONE) {
        return;
    }

    widget->last_focus_event = event;
    widget->focus_change_count++;
    if (event == PICOUI_FOCUS_EVENT_ENTER) {
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

int picoui_widget_claim_focus(struct picoui_widget *widget)
{
    struct picoui_app *owner;
    struct picoui_widget *previous;

    if (widget == 0 || widget->visible == 0 || widget->enabled == 0) {
        return -1;
    }

    owner = picoui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    previous = owner->focus_owner;
    if (previous == widget) {
        if (widget->has_focus == 0) {
            picoui_widget_note_focus_event(widget, PICOUI_FOCUS_EVENT_ENTER);
        }
        return 0;
    }

    if (previous != 0) {
        picoui_widget_note_focus_event(previous, PICOUI_FOCUS_EVENT_LEAVE);
    }

    owner->focus_owner = widget;
    picoui_widget_note_focus_event(widget, PICOUI_FOCUS_EVENT_ENTER);
    return 0;
}

/**
 * @brief Widget: release focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_release_focus(struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0) {
        return -1;
    }

    owner = picoui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    if (owner->focus_owner != widget) {
        if (widget->has_focus != 0) {
            picoui_widget_note_focus_event(widget, PICOUI_FOCUS_EVENT_LEAVE);
        }
        return 0;
    }

    owner->focus_owner = 0;
    picoui_widget_note_focus_event(widget, PICOUI_FOCUS_EVENT_LEAVE);
    return 0;
}

/**
 * @brief Widget: is focus owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success
 */

int picoui_widget_is_focus_owner(const struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0) {
        return 0;
    }

    owner = picoui_widget_get_owner_app((struct picoui_widget *)widget);
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

int picoui_widget_mark_edit_result(struct picoui_widget *widget, enum picoui_edit_result result)
{
    if (widget == 0) {
        return -1;
    }

    if (result != PICOUI_EDIT_RESULT_COMMIT && result != PICOUI_EDIT_RESULT_CANCEL) {
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

int picoui_widget_claim_editing(struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0 || widget->visible == 0 || widget->enabled == 0) {
        return -1;
    }

    owner = picoui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    widget->pending_edit_result = PICOUI_EDIT_RESULT_NONE;
    owner->editing_owner = widget;
    return 0;
}

/**
 * @brief Widget: release editing
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_release_editing(struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0) {
        return -1;
    }

    owner = picoui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    if (owner->editing_owner == widget) {
        owner->editing_owner = 0;
    }
    widget->last_edit_result = widget->pending_edit_result;
    widget->pending_edit_result = PICOUI_EDIT_RESULT_NONE;
    return 0;
}

/**
 * @brief Widget: is editing owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success
 */

int picoui_widget_is_editing_owner(const struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0) {
        return 0;
    }

    owner = picoui_widget_get_owner_app((struct picoui_widget *)widget);
    if (owner == 0) {
        return 0;
    }

    return owner->editing_owner == widget;
}

int picoui_widget_dispatch_signal(void *backend_widget,
                                  enum picoui_backend_signal signal,
                                  int value,
                                  picoui_value_changed_cb cb,
                                  struct picoui_widget *widget,
                                  void *user_data)
{
    struct picoui_backend_widget *backend = backend_widget;

    if (backend == 0) {
        return -1;
    }

    if (!picoui_widget_accepts_event(widget)) {
        return 0;
    }

    if (signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        if (backend->value == value) {
            return 0;
        }

        backend->value = value;
        backend->data_model_epoch++;
        backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
        picoui_widget_sync_ld_value(backend, widget, value);
        backend->last_signal = signal;
        backend->dispatch_count++;
        picoui_widget_emit_ld_event_bridge(backend, signal, value);
        if (cb != 0) {
            picoui_backend_emit_value_changed(cb, widget, value, user_data);
        }
        return 0;
    }

    return -1;
}

int picoui_widget_dispatch_event(void *backend_widget,
                                 enum picoui_backend_signal signal,
                                 picoui_event_cb cb,
                                 struct picoui_widget *widget,
                                 void *user_data)
{
    struct picoui_backend_widget *backend = backend_widget;

    if (backend == 0) {
        return -1;
    }

    if (!picoui_widget_accepts_event(widget)) {
        (void)picoui_backend_widget_release_focus(backend_widget);
        return 0;
    }

    if (signal == PICOUI_BACKEND_SIGNAL_PRESSED || signal == PICOUI_BACKEND_SIGNAL_RELEASED) {
        if (picoui_widget_claim_focus_for_signal(backend, signal) != 0) {
            return -1;
        }
        backend->last_signal = signal;
        backend->dispatch_count++;
        picoui_backend_emit_event(cb, widget, user_data);
        return 0;
    }

    return -1;
}

/**
 * @brief Event: stub
 *
 * @return 0 on success
 */

int picoui_event_stub(void)
{
    return 0;
}
