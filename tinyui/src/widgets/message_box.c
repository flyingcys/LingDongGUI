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
#include "backend.h"
#include "picoui/message_box.h"
#include "picoui/widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldMessageBox.h"
#include "../../../src/gui/ldBase.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static int picoui_message_box_props_are_valid(const struct picoui_message_box_props *props)
{
    return props != 0 && props->id != 0;
}

static ldMessageBox_t *picoui_message_box_get_ld(struct picoui_message_box *box)
{
    struct picoui_backend_widget *backend;

    if (box == 0 || box->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_MESSAGE_BOX || backend->ld_widget == 0) {
        return 0;
    }

    return (ldMessageBox_t *)backend->ld_widget;
}

static void picoui_message_box_confirm_bridge(ld_scene_t *scene, ldMessageBox_t *ld_message_box)
{
    struct picoui_backend_widget *backend;
    struct picoui_message_box *box;

    (void)scene;

    if (ld_message_box == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)ld_message_box)->pInfo;
    if (backend == 0 || backend->host_widget == 0) {
        return;
    }

    box = (struct picoui_message_box *)backend->host_widget;
    if (box->widget.enabled == 0 || box->widget.visible == 0) {
        return;
    }

    if (box->on_confirm_indexed != 0) {
        box->on_confirm_indexed(box, ld_message_box->clickNum, box->on_confirm_indexed_user_data);
    }
    if (box->on_confirm != 0) {
        box->on_confirm(box, box->on_confirm_user_data);
    }
}

/**
 * @brief Create message box widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_message_box *picoui_message_box_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_message_box *box;
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldMessageBox_t *ld_message_box;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->backend_widget;
    app_state = picoui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    box = calloc(1, sizeof(*box));
    if (box == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(box);
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(box);
        return 0;
    }

    ld_message_box = ldMessageBox_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent_backend->ld_name_id,
                                       0,
                                       0,
                                       260,
                                       140,
                                       (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_message_box == 0) {
        free(backend);
        free(box);
        return 0;
    }

    if (picoui_backend_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_MESSAGE_BOX,
                                         id,
                                         parent_backend->theme) != 0) {
        ldMessageBox_depose(app_state->ld_scene, ld_message_box);
        free(backend);
        free(box);
        return 0;
    }
    backend->ld_widget = ld_message_box;
    backend->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent_backend, backend) != 0) {
        ldMessageBox_depose(app_state->ld_scene, ld_message_box);
        free(backend);
        free(box);
        return 0;
    }

    box->widget.backend_widget = backend;
    box->id = id;
    box->widget.visible = 1;
    box->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(box->widget.backend_widget, &box->widget) != 0) {
        (void)picoui_backend_widget_detach_from_parent(box->widget.backend_widget);
        ldMessageBox_depose(app_state->ld_scene, ld_message_box);
        free(backend);
        free(box);
        return 0;
    }
    return box;
}

/**
 * @brief message box init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_message_box *picoui_message_box_init(struct picoui_widget *parent, const char *id)
{
    return picoui_message_box_create(parent, id);
}

/**
 * @brief Create message box widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_message_box *picoui_message_box_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_message_box_props *props)
{
    struct picoui_message_box *box;

    if (!picoui_message_box_props_are_valid(props)) {
        return 0;
    }

    box = picoui_message_box_create(parent, props->id);
    if (box == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&box->widget, props->style_class) != 0) {
        free(box);
        return 0;
    }
    if (picoui_widget_set_user_data(&box->widget, props->user_data) != 0) {
        free(box);
        return 0;
    }
    if ((props->title != 0 && picoui_message_box_set_title(box, props->title) != 0)
        || (props->message != 0 && picoui_message_box_set_message(box, props->message) != 0)
        || (props->confirm_text != 0
            && picoui_message_box_set_confirm_text(box, props->confirm_text) != 0)) {
        free(box);
        return 0;
    }

    return box;
}

/**
 * @brief Set title of message box widget
 *
 * @param[in] box box
 * @param[in] title title
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_title(struct picoui_message_box *box, const char *title)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0 || title == 0) {
        return -1;
    }

    ld_message_box = picoui_message_box_get_ld(box);
    if (ld_message_box == 0) {
        return -1;
    }

    ldMessageBoxSetTitle(ld_message_box, (const uint8_t *)title);
    box->title = title;
    return 0;
}

/**
 * @brief Set message of message box widget
 *
 * @param[in] box box
 * @param[in] message message
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_message(struct picoui_message_box *box, const char *message)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0 || message == 0) {
        return -1;
    }

    ld_message_box = picoui_message_box_get_ld(box);
    if (ld_message_box == 0) {
        return -1;
    }

    ldMessageBoxSetMsg(ld_message_box, (const uint8_t *)message);
    box->message = message;
    return 0;
}

/**
 * @brief Set msg of message box widget
 *
 * @param[in] box box
 * @param[in] message message
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_msg(struct picoui_message_box *box, const char *message)
{
    return picoui_message_box_set_message(box, message);
}

/**
 * @brief Set confirm text of message box widget
 *
 * @param[in] box box
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_confirm_text(struct picoui_message_box *box, const char *text)
{
    ldMessageBox_t *ld_message_box;
    struct picoui_backend_widget *backend;

    if (box == 0 || text == 0) {
        return -1;
    }

    ld_message_box = picoui_message_box_get_ld(box);
    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    if (ld_message_box == 0 || backend == 0) {
        return -1;
    }

    backend->text = text;
    ldMessageBoxSetBtn(ld_message_box, (const uint8_t **)&backend->text, 1);
    box->confirm_text = text;
    return 0;
}

/**
 * @brief Set buttons of message box widget
 *
 * @param[in] box box
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_buttons(struct picoui_message_box *box, const char *const *buttons, int count)
{
    int i;
    ldMessageBox_t *ld_message_box;

    if (box == 0 || buttons == 0 || count <= 0 || count > PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }
    for (i = 0; i < count; ++i) {
        if (buttons[i] == 0) {
            return -1;
        }
    }

    ld_message_box = picoui_message_box_get_ld(box);
    if (ld_message_box == 0) {
        return -1;
    }

    ldMessageBoxSetBtn(ld_message_box, (const uint8_t **)buttons, (uint8_t)count);
    for (i = 0; i < count; ++i) {
        box->buttons[i] = buttons[i];
    }
    box->button_count = count;
    if (count > 0) {
        box->confirm_text = buttons[count - 1];
    }
    return 0;
}

/**
 * @brief Set btn of message box widget
 *
 * @param[in] box box
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_btn(struct picoui_message_box *box, const char *const *buttons, int count)
{
    return picoui_message_box_set_buttons(box, buttons, count);
}

/**
 * @brief Set string colors of message box widget
 *
 * @param[in] box box
 * @param[in] title_color title color
 * @param[in] message_color message color
 * @param[in] button_color button color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_string_colors(struct picoui_message_box *box,
                                         unsigned int title_color,
                                         unsigned int message_color,
                                         unsigned int button_color)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return -1;
    }

    ld_message_box = picoui_message_box_get_ld(box);
    if (ld_message_box == 0) {
        return -1;
    }

    ldMessageBoxSetStringColor(ld_message_box,
                               (ldColor)title_color,
                               (ldColor)message_color,
                               (ldColor)button_color);
    box->title_color = title_color;
    box->message_color = message_color;
    box->button_color = button_color;
    return 0;
}

/**
 * @brief Set string color of message box widget
 *
 * @param[in] box box
 * @param[in] title_color title color
 * @param[in] message_color message color
 * @param[in] button_color button color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_string_color(struct picoui_message_box *box,
                                        unsigned int title_color,
                                        unsigned int message_color,
                                        unsigned int button_color)
{
    return picoui_message_box_set_string_colors(box, title_color, message_color, button_color);
}

/**
 * @brief Set button colors of message box widget
 *
 * @param[in] box box
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_button_colors(struct picoui_message_box *box,
                                         unsigned int release_color,
                                         unsigned int press_color)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return -1;
    }

    ld_message_box = picoui_message_box_get_ld(box);
    if (ld_message_box == 0) {
        return -1;
    }

    ldMessageBoxSetButtonColor(ld_message_box, (ldColor)release_color, (ldColor)press_color);
    box->release_color = release_color;
    box->press_color = press_color;
    return 0;
}

/**
 * @brief Set button color of message box widget
 *
 * @param[in] box box
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_button_color(struct picoui_message_box *box,
                                        unsigned int release_color,
                                        unsigned int press_color)
{
    return picoui_message_box_set_button_colors(box, release_color, press_color);
}

/**
 * @brief Set bg color of message box widget
 *
 * @param[in] box box
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_bg_color(struct picoui_message_box *box, unsigned int bg_color)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return -1;
    }

    ld_message_box = picoui_message_box_get_ld(box);
    if (ld_message_box == 0) {
        return -1;
    }

    ldMessageBoxSetBackgroundColor(ld_message_box, (ldColor)bg_color);
    box->bg_color = bg_color;
    return 0;
}

/**
 * @brief Set background color of message box widget
 *
 * @param[in] box box
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_background_color(struct picoui_message_box *box, unsigned int bg_color)
{
    return picoui_message_box_set_bg_color(box, bg_color);
}

/**
 * @brief Set on confirm of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void picoui_message_box_set_on_confirm(
    struct picoui_message_box *box,
    picoui_message_box_callback_t callback,
    void *user_data)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return;
    }

    box->on_confirm = callback;
    box->on_confirm_user_data = user_data;
    if (callback != 0) {
        ld_message_box = picoui_message_box_get_ld(box);
        if (ld_message_box != 0) {
            ldMessageBoxSetCallback(ld_message_box, picoui_message_box_confirm_bridge);
        }
    }
}

/**
 * @brief Set callback of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void picoui_message_box_set_callback(
    struct picoui_message_box *box,
    picoui_message_box_callback_t callback,
    void *user_data)
{
    picoui_message_box_set_on_confirm(box, callback, user_data);
}

/**
 * @brief Set on confirm indexed of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void picoui_message_box_set_on_confirm_indexed(
    struct picoui_message_box *box,
    picoui_message_box_indexed_callback_t callback,
    void *user_data)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return;
    }

    box->on_confirm_indexed = callback;
    box->on_confirm_indexed_user_data = user_data;
    if (callback != 0) {
        ld_message_box = picoui_message_box_get_ld(box);
        if (ld_message_box != 0) {
            ldMessageBoxSetCallback(ld_message_box, picoui_message_box_confirm_bridge);
        }
    }
}

/**
 * @brief Get title of message box widget
 *
 * @param[in] box box
 */

const char *picoui_message_box_get_title(const struct picoui_message_box *box)
{
    if (box == 0) {
        return 0;
    }
    return box->title;
}

/**
 * @brief Get message of message box widget
 *
 * @param[in] box box
 */

const char *picoui_message_box_get_message(const struct picoui_message_box *box)
{
    if (box == 0) {
        return 0;
    }
    return box->message;
}

/**
 * @brief Get confirm text of message box widget
 *
 * @param[in] box box
 */

const char *picoui_message_box_get_confirm_text(const struct picoui_message_box *box)
{
    if (box == 0) {
        return 0;
    }
    return box->confirm_text;
}
