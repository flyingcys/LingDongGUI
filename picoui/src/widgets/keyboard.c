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

#include <string.h>
#include <stdlib.h>

int picoui_native_keyboard_input_ascii(struct picoui_keyboard *keyboard, unsigned int ascii);

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
    if (keyboard == 0 || key_code == 0 || keyboard->widget.backend_widget == 0) {
        return -1;
    }

    *key_code = keyboard->selected_key_code;
    return 0;
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

    if (parent == 0 || id == 0) {
        return 0;
    }

    keyboard = calloc(1, sizeof(*keyboard));
    if (keyboard == 0) {
        return 0;
    }

    keyboard->widget.backend_widget = picoui_backend_create_keyboard(parent->widget.backend_widget, id);
    if (keyboard->widget.backend_widget == 0) {
        free(keyboard);
        return 0;
    }

    keyboard->id = id;
    keyboard->widget.visible = 1;
    keyboard->widget.enabled = 1;
    keyboard->widget.selectable = 1;
    if (picoui_backend_widget_bind_host(keyboard->widget.backend_widget, &keyboard->widget) != 0) {
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
    if (keyboard == 0) {
        return -1;
    }

    return picoui_native_keyboard_input_ascii(keyboard, ascii);
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
    if (keyboard == 0) {
        return -1;
    }

    return picoui_backend_keyboard_navigate(keyboard->widget.backend_widget, direction);
}

/**
 * @brief keyboard update
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int picoui_keyboard_update(struct picoui_keyboard *keyboard)
{
    if (keyboard == 0) {
        return -1;
    }

    return picoui_backend_keyboard_update(keyboard->widget.backend_widget);
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
    if (keyboard == 0 || key_code > 0xFFU) {
        return -1;
    }

    keyboard->selected_key_code = key_code;
    return picoui_backend_keyboard_button_update(keyboard->widget.backend_widget, (unsigned char)key_code);
}

/**
 * @brief keyboard click
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int picoui_keyboard_click(struct picoui_keyboard *keyboard)
{
    if (keyboard == 0) {
        return -1;
    }

    keyboard->selected_key_code = (unsigned int)picoui_keyboard_get_selected_key_code(keyboard);
    return picoui_backend_keyboard_click(keyboard->widget.backend_widget);
}

/**
 * @brief keyboard exit
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int picoui_keyboard_exit(struct picoui_keyboard *keyboard)
{
    struct picoui_app *app;
    struct picoui_line_edit *line_edit;
    struct picoui_backend_widget *backend;

    if (keyboard == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    if (backend != 0 && backend->owner != 0) {
        app = backend->owner;
        if (app->editing_owner != 0 && app->editing_owner->accepts_text_input != 0) {
            line_edit = (struct picoui_line_edit *)app->editing_owner;
            line_edit->editing = 0;
            (void)picoui_widget_mark_edit_result(&line_edit->widget, PICOUI_EDIT_RESULT_CANCEL);
            (void)picoui_widget_release_editing(&line_edit->widget);
        }
    }

    return picoui_backend_keyboard_exit(keyboard->widget.backend_widget);
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
