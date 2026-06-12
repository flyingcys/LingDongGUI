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

#include "picoui/keyboard.h"
#include "internal.h"
#include "../core/runtime_bridge.h"
#include "ldLineEdit.h"
#include "ldKeyboard.h"

#include <string.h>
#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
void ldKeyboardInputAscii(ldKeyboard_t *ptWidget, uint8_t ascii);

static void picoui_keyboard_free_layout(struct picoui_keyboard *keyboard)
{
    int i;
    void *native_layout;

    if (keyboard == 0) {
        return;
    }

    native_layout = keyboard->native_layout;

    if (keyboard->layout_entries == 0) {
        if (native_layout != 0) {
            free(native_layout);
        }
        keyboard->native_layout = 0;
        keyboard->layout_count = 0;
        return;
    }

    for (i = 0; i < keyboard->layout_count; ++i) {
        free(keyboard->layout_entries[i].text);
    }
    free(keyboard->layout_entries);
    if (native_layout != 0 && native_layout != (void *)keyboard->layout_entries) {
        free(native_layout);
    }
    keyboard->layout_entries = 0;
    keyboard->native_layout = 0;
    keyboard->layout_count = 0;
}

static int picoui_keyboard_props_are_valid(const struct picoui_keyboard_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static int picoui_keyboard_get_selected_key_code_internal(const struct picoui_keyboard *keyboard,
                                                          unsigned int *key_code)
{
    struct picoui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0 || key_code == 0 || keyboard->widget.backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    if (ld_keyboard == 0) {
        return -1;
    }

    *key_code = ld_keyboard->keyCode;
    return 0;
}

static ldKeyboard_t *picoui_keyboard_get_ld_widget(const struct picoui_keyboard *keyboard)
{
    struct picoui_backend_widget *backend;

    if (keyboard == 0 || keyboard->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_KEYBOARD || backend->ld_widget == 0) {
        return 0;
    }

    return (ldKeyboard_t *)backend->ld_widget;
}

static struct picoui_line_edit *picoui_keyboard_get_target_line_edit_local(
    struct picoui_backend_widget *backend)
{
    struct picoui_app *app;
    struct picoui_widget *target;
    struct picoui_backend_widget *target_backend;
    ldBase_t *ld_base;

    if (backend == 0 || backend->owner == 0) {
        return 0;
    }

    app = backend->owner;
    target = app->editing_owner != 0 ? app->editing_owner : app->focus_owner;
    if (target == 0 || target->backend_widget == 0) {
        return 0;
    }

    target_backend = (struct picoui_backend_widget *)target->backend_widget;
    if (target_backend->kind != PICOUI_BACKEND_WIDGET_TEXT || target_backend->ld_widget == 0) {
        return 0;
    }

    ld_base = (ldBase_t *)target_backend->ld_widget;
    if (ld_base->widgetType != widgetTypeLineEdit) {
        return 0;
    }
    return (struct picoui_line_edit *)target;
}

static const kbBtnInfo_t *picoui_keyboard_get_custom_button_list_local(struct picoui_backend_widget *backend)
{
    struct picoui_keyboard *keyboard;
    kbBtnInfo_t *native_buttons;
    int i;

    if (backend == 0 || backend->host_widget == 0) {
        return 0;
    }

    keyboard = (struct picoui_keyboard *)backend->host_widget;
    if (keyboard->layout_entries == 0 || keyboard->layout_count <= 0) {
        return 0;
    }

    if (keyboard->native_layout != 0 &&
        keyboard->native_layout != (void *)keyboard->layout_entries) {
        return (const kbBtnInfo_t *)keyboard->native_layout;
    }

    native_buttons = calloc((size_t)keyboard->layout_count + 1U, sizeof(*native_buttons));
    if (native_buttons == 0) {
        return 0;
    }

    for (i = 0; i < keyboard->layout_count; ++i) {
        struct picoui_keyboard_button button_info;

        native_buttons[i].region.tLocation.iX = (int16_t)keyboard->layout_entries[i].x;
        native_buttons[i].region.tLocation.iY = (int16_t)keyboard->layout_entries[i].y;
        native_buttons[i].region.tSize.iWidth = (int16_t)keyboard->layout_entries[i].width;
        native_buttons[i].region.tSize.iHeight = (int16_t)keyboard->layout_entries[i].height;
        native_buttons[i].pText = (uint8_t *)keyboard->layout_entries[i].text;
        native_buttons[i].keyCode = (uint8_t)keyboard->layout_entries[i].key_code;
        native_buttons[i].pressColor = __RGB((keyboard->layout_entries[i].press_color >> 16) & 0xFFU,
                                             (keyboard->layout_entries[i].press_color >> 8) & 0xFFU,
                                             keyboard->layout_entries[i].press_color & 0xFFU);
        native_buttons[i].releaseColor = __RGB((keyboard->layout_entries[i].release_color >> 16) & 0xFFU,
                                               (keyboard->layout_entries[i].release_color >> 8) & 0xFFU,
                                               keyboard->layout_entries[i].release_color & 0xFFU);

        button_info.x = keyboard->layout_entries[i].x;
        button_info.y = keyboard->layout_entries[i].y;
        button_info.width = keyboard->layout_entries[i].width;
        button_info.height = keyboard->layout_entries[i].height;
        button_info.text = keyboard->layout_entries[i].text;
        button_info.key_code = keyboard->layout_entries[i].key_code;
        button_info.press_color = keyboard->layout_entries[i].press_color;
        button_info.release_color = keyboard->layout_entries[i].release_color;
        if (keyboard->draw_cb != 0) {
            keyboard->draw_invocation_count++;
            keyboard->last_draw_key_code = button_info.key_code;
            keyboard->draw_cb(keyboard, &button_info, keyboard->draw_user_data);
        }
    }

    keyboard->native_layout = native_buttons;
    return native_buttons;
}

static void picoui_keyboard_prepare_local(ldKeyboard_t *ld_keyboard,
                                          struct picoui_line_edit *line_edit)
{
    struct picoui_backend_widget *backend;
    const kbBtnInfo_t *custom_buttons;

    if (ld_keyboard == 0) {
        return;
    }

    if (line_edit != 0) {
        ld_keyboard->editType = (ldEditType_t)line_edit->type;
    }
    backend = (struct picoui_backend_widget *)((ldBase_t *)ld_keyboard)->pInfo;
    custom_buttons = picoui_keyboard_get_custom_button_list_local(backend);
    if (ld_keyboard->pBtnList == 0 || ld_keyboard->isWaitInit) {
        ld_keyboard->pBtnList = custom_buttons != 0
                              ? custom_buttons
                              : ldKeyboardGetTargetBtnList(ld_keyboard);
        ld_keyboard->isWaitInit = false;
    } else if (custom_buttons != 0) {
        ld_keyboard->pBtnList = custom_buttons;
    } else {
        ld_keyboard->pBtnList = ldKeyboardGetTargetBtnList(ld_keyboard);
    }
    ldBaseSetHidden((ldBase_t *)ld_keyboard, false);
}

/**
 * @brief Create keyboard widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_keyboard *picoui_keyboard_create(struct picoui_window *parent, const char *id)
{
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldKeyboard_t *ld_keyboard;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->widget.backend_widget;
    app_state = parent_backend != 0
        ? tinyui_runtime_bridge_backend_state_from_parent(parent_backend)
        : 0;
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    keyboard = calloc(1, sizeof(*keyboard));
    if (keyboard == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(keyboard);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(keyboard);
        return 0;
    }

    ld_keyboard = ldKeyboard_init(app_state->ld_scene,
                                  NULL,
                                  name_id,
                                  parent_backend->ld_name_id,
                                  (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_keyboard == 0) {
        free(backend);
        free(keyboard);
        return 0;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_KEYBOARD,
                                         id,
                                         parent_backend->theme) != 0) {
        ldKeyboard_depose(app_state->ld_scene, ld_keyboard);
        free(backend);
        free(keyboard);
        return 0;
    }
    backend->ld_widget = ld_keyboard;
    backend->ld_name_id = name_id;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldKeyboard_depose(app_state->ld_scene, ld_keyboard);
        free(backend);
        free(keyboard);
        return 0;
    }

    keyboard->widget.backend_widget = backend;
    keyboard->id = id;
    keyboard->widget.visible = 1;
    keyboard->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(keyboard->widget.backend_widget, &keyboard->widget) != 0) {
        (void)tinyui_runtime_bridge_detach_from_parent(keyboard->widget.backend_widget);
        ldKeyboard_depose(app_state->ld_scene, ld_keyboard);
        free(backend);
        free(keyboard);
        return 0;
    }
    return keyboard;
}

/**
 * @brief Create keyboard widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_keyboard *picoui_keyboard_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_keyboard_props *props)
{
    struct picoui_keyboard *keyboard;

    if (!picoui_keyboard_props_are_valid(props)) {
        return 0;
    }

    keyboard = picoui_keyboard_create(parent, props->id);
    if (keyboard == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&keyboard->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&keyboard->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&keyboard->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&keyboard->widget, props->border_color) != 0
        || picoui_widget_set_radius(&keyboard->widget, props->radius) != 0
        || picoui_widget_set_padding(&keyboard->widget, props->padding) != 0) {
        free(keyboard);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&keyboard->widget, props->style_class) != 0) {
        free(keyboard);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&keyboard->widget, props->width, props->height) != 0) {
        free(keyboard);
        return 0;
    }

    return keyboard;
}

/**
 * @brief keyboard input ascii
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] ascii ascii
 * @return -1 on failure
 */

int picoui_keyboard_input_ascii(struct picoui_keyboard *keyboard, unsigned int ascii)
{
    struct picoui_backend_widget *backend;
    struct picoui_line_edit *line_edit;
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit;

    if (keyboard == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    line_edit = picoui_keyboard_get_target_line_edit_local(backend);
    ld_keyboard = picoui_keyboard_get_ld_widget(keyboard);
    if (line_edit == 0 || ld_keyboard == 0) {
        return -1;
    }
    ld_line_edit = (ldLineEdit_t *)((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_widget;
    if (ld_line_edit == 0) {
        return -1;
    }

    picoui_keyboard_prepare_local(ld_keyboard, line_edit);
    ld_keyboard->ppStr = &ld_line_edit->pText;
    ld_keyboard->strMax = ld_line_edit->textMax;
    ld_keyboard->editorId = ((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_name_id;
    ldKeyboardInputAscii(ld_keyboard, (uint8_t)ascii);
    line_edit->widget.text = picoui_backend_line_edit_get_text(line_edit->widget.backend_widget);
    return 0;
}

/**
 * @brief keyboard navigate
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] direction direction
 * @return -1 on failure
 */

int picoui_keyboard_navigate(struct picoui_keyboard *keyboard, int direction)
{
    struct picoui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    if (backend == 0 || backend->host_widget == 0 || !picoui_widget_is_focus_owner(backend->host_widget)) {
        return -1;
    }

    ld_keyboard = picoui_keyboard_get_ld_widget(keyboard);
    if (ld_keyboard == 0) {
        return -1;
    }

    picoui_keyboard_prepare_local(ld_keyboard, picoui_keyboard_get_target_line_edit_local(backend));
    ldKeyboardNavigate(ld_keyboard, (ldNavDir_t)direction);
    return 0;
}

/**
 * @brief keyboard update
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int picoui_keyboard_update(struct picoui_keyboard *keyboard)
{
    struct picoui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    ld_keyboard = picoui_keyboard_get_ld_widget(keyboard);
    if (backend == 0 || ld_keyboard == 0) {
        return -1;
    }

    picoui_keyboard_prepare_local(ld_keyboard, picoui_keyboard_get_target_line_edit_local(backend));
    ldKeyboardUpdate(ld_keyboard);
    return 0;
}

/**
 * @brief keyboard button update
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] key_code key code
 * @return -1 on failure
 */

int picoui_keyboard_button_update(struct picoui_keyboard *keyboard, unsigned int key_code)
{
    struct picoui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0 || key_code > 0xFFU) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    ld_keyboard = picoui_keyboard_get_ld_widget(keyboard);
    if (backend == 0 || ld_keyboard == 0) {
        return -1;
    }

    picoui_keyboard_prepare_local(ld_keyboard, picoui_keyboard_get_target_line_edit_local(backend));
    ld_keyboard->keyCode = (uint8_t)key_code;
    ld_keyboard->isKeySelect = true;
    ldKeyboardBtnUpdate(ld_keyboard, (uint8_t)key_code);
    if (keyboard->event_cb != 0) {
        keyboard->event_cb(keyboard,
                           (unsigned int)key_code,
                           PICOUI_NATIVE_SIGNAL_VALUE_CHANGED,
                           keyboard->event_user_data);
    }
    return 0;
}

/**
 * @brief keyboard click
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int picoui_keyboard_click(struct picoui_keyboard *keyboard)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    if (backend == 0 || backend->host_widget == 0 || !picoui_widget_is_focus_owner(backend->host_widget)) {
        return -1;
    }

    app_state = tinyui_runtime_bridge_backend_state_from_parent(backend);
    ld_keyboard = picoui_keyboard_get_ld_widget(keyboard);
    if (app_state == 0 || app_state->ld_scene == 0 || ld_keyboard == 0) {
        return -1;
    }

    ldKeyboardClick(app_state->ld_scene, ld_keyboard, SIGNAL_PRESS);
    if (keyboard->event_cb != 0) {
        keyboard->event_cb(keyboard, ld_keyboard->keyCode, PICOUI_NATIVE_SIGNAL_PRESS, keyboard->event_user_data);
    }
    return 0;
}

/**
 * @brief keyboard exit
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int picoui_keyboard_exit(struct picoui_keyboard *keyboard)
{
    struct picoui_backend_widget *backend;
    struct picoui_line_edit *line_edit;
    struct picoui_widget *editing_owner_widget = 0;
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit = 0;

    if (keyboard == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }

    ld_keyboard = picoui_keyboard_get_ld_widget(keyboard);
    if (ld_keyboard == 0) {
        return -1;
    }

    if (backend->owner != 0) {
        editing_owner_widget = backend->owner->editing_owner;
    }

    line_edit = picoui_keyboard_get_target_line_edit_local(backend);
    ldKeyboardExit(ld_keyboard);
    if (line_edit != 0) {
        ld_line_edit = (ldLineEdit_t *)((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_widget;
        line_edit->editing = 0;
        if (ld_line_edit != 0) {
            ld_line_edit->isEditing = false;
            ((ldBase_t *)ld_line_edit)->isDirtyRegionUpdate = true;
        }
        ((struct picoui_backend_widget *)line_edit->widget.backend_widget)->edit_result_on_finish =
            PICOUI_EDIT_RESULT_CANCEL;
        (void)picoui_widget_mark_edit_result(&line_edit->widget, PICOUI_EDIT_RESULT_CANCEL);
        (void)picoui_widget_release_editing(&line_edit->widget);
    } else if (editing_owner_widget != 0) {
        (void)picoui_widget_mark_edit_result(editing_owner_widget, PICOUI_EDIT_RESULT_CANCEL);
        (void)picoui_widget_release_editing(editing_owner_widget);
    }
    if (backend->host_widget != 0 && picoui_widget_is_focus_owner(backend->host_widget)) {
        return picoui_widget_release_focus(backend->host_widget);
    }
    return 0;
}

/**
 * @brief Set buttons of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_buttons(struct picoui_keyboard *keyboard,
                                const struct picoui_keyboard_button *buttons,
                                int count)
{
    struct picoui_keyboard_layout_entry *entries;
    int i;

    if (keyboard == 0) {
        return -1;
    }

    if (buttons == 0 || count <= 0) {
        picoui_keyboard_free_layout(keyboard);
        keyboard->buttons = 0;
        return 0;
    }

    entries = calloc((size_t)count, sizeof(*entries));
    if (entries == 0) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        if (buttons[i].text == 0 || buttons[i].key_code > 0xFFU ||
            buttons[i].width < 0 || buttons[i].height < 0) {
            while (--i >= 0) {
                free(entries[i].text);
            }
            free(entries);
            return -1;
        }
        entries[i].text = strdup(buttons[i].text);
        if (entries[i].text == 0) {
            while (--i >= 0) {
                free(entries[i].text);
            }
            free(entries);
            return -1;
        }
        entries[i].key_code = buttons[i].key_code;
        entries[i].press_color = buttons[i].press_color;
        entries[i].release_color = buttons[i].release_color;
        entries[i].x = buttons[i].x;
        entries[i].y = buttons[i].y;
        entries[i].width = buttons[i].width;
        entries[i].height = buttons[i].height;
    }

    picoui_keyboard_free_layout(keyboard);
    keyboard->buttons = buttons;
    keyboard->layout_entries = entries;
    keyboard->layout_count = count;
    keyboard->native_layout = entries;
    return 0;
}

/**
 * @brief Get buttons of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_get_buttons(const struct picoui_keyboard *keyboard,
                                const struct picoui_keyboard_button **buttons,
                                int *count)
{
    if (keyboard == 0 || buttons == 0 || count == 0) {
        return -1;
    }

    *buttons = keyboard->buttons;
    *count = keyboard->layout_count;
    return 0;
}

/**
 * @brief Set on key event of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_on_key_event(struct picoui_keyboard *keyboard,
                                     picoui_keyboard_event_cb cb,
                                     void *user_data)
{
    if (keyboard == 0) {
        return -1;
    }

    keyboard->event_cb = cb;
    keyboard->event_user_data = user_data;
    return 0;
}

/**
 * @brief Get selected key code of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int picoui_keyboard_get_selected_key_code(const struct picoui_keyboard *keyboard)
{
    unsigned int key_code = 0;

    if (picoui_keyboard_get_selected_key_code_internal(keyboard, &key_code) != 0) {
        return -1;
    }

    return (int)key_code;
}

/**
 * @brief Set layout of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_layout(struct picoui_keyboard *keyboard,
                               const struct picoui_keyboard_button *buttons,
                               int count)
{
    return picoui_keyboard_set_buttons(keyboard, buttons, count);
}

/**
 * @brief Set event callback of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_event_callback(struct picoui_keyboard *keyboard,
                                       picoui_keyboard_event_cb cb,
                                       void *user_data)
{
    return picoui_keyboard_set_on_key_event(keyboard, cb, user_data);
}

/**
 * @brief Set draw callback of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_draw_callback(struct picoui_keyboard *keyboard,
                                      picoui_keyboard_draw_cb cb,
                                      void *user_data)
{
    if (keyboard == 0) {
        return -1;
    }

    keyboard->draw_cb = cb;
    keyboard->draw_user_data = user_data;
    return 0;
}
