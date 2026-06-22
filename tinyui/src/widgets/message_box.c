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
#include "message_box.h"
#include "widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldMessageBox.h"
#include "../../../src/gui/ldBase.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static int tinyui_message_box_props_are_valid(const struct tinyui_message_box_props *props)
{
    return props != 0 && props->id != 0;
}

static ldMessageBox_t *tinyui_message_box_get_ld(struct tinyui_message_box *box)
{
    if (box == 0 || box->widget.ld_widget == 0
        || box->widget.kind != TINYUI_BACKEND_WIDGET_MESSAGE_BOX) {
        return 0;
    }

    return (ldMessageBox_t *)box->widget.ld_widget;
}

static void tinyui_message_box_confirm_bridge(ld_scene_t *scene, ldMessageBox_t *ld_message_box)
{
    /* C1-T7: pInfo now points to tinyui_widget, not tinyui_backend_widget */
    struct tinyui_widget *w;
    struct tinyui_message_box *box;

    (void)scene;

    if (ld_message_box == 0) {
        return;
    }

    w = (struct tinyui_widget *)((ldBase_t *)ld_message_box)->pInfo;
    if (w == 0) {
        return;
    }

    box = (struct tinyui_message_box *)w;
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

struct tinyui_message_box *tinyui_message_box_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_message_box *box;
    struct tinyui_app *app_state;
    ldMessageBox_t *ld_message_box;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }

    app_state = parent->owner;
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    box = calloc(1, sizeof(*box));
    if (box == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;

    ld_message_box = ldMessageBox_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent->ld_name_id,
                                       0,
                                       0,
                                       260,
                                       140,
                                       (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_message_box == 0) {
        free(box);
        return 0;
    }

    box->id = id;
    box->widget.ld_widget  = ld_message_box;
    box->widget.ld_name_id = name_id;
    box->widget.kind       = TINYUI_BACKEND_WIDGET_MESSAGE_BOX;
    box->widget.owner      = app_state;
    box->widget.visible    = 1;
    box->widget.enabled    = 1;
    ((ldBase_t *)ld_message_box)->pInfo = &box->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&box->widget, app_state);
    return box;
}

/**
 * @brief message box init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_message_box *tinyui_message_box_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_message_box_create(parent, id);
}

/**
 * @brief Create message box widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_message_box *tinyui_message_box_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_message_box_props *props)
{
    struct tinyui_message_box *box;

    if (!tinyui_message_box_props_are_valid(props)) {
        return 0;
    }

    box = tinyui_message_box_create(parent, props->id);
    if (box == 0) {
        return 0;
    }

    if (props->style_class != 0
        && tinyui_widget_set_style_class(&box->widget, props->style_class) != 0) {
        free(box);
        return 0;
    }
    if (tinyui_widget_set_user_data(&box->widget, props->user_data) != 0) {
        free(box);
        return 0;
    }
    if ((props->title != 0 && tinyui_message_box_set_title(box, props->title) != 0)
        || (props->message != 0 && tinyui_message_box_set_message(box, props->message) != 0)
        || (props->confirm_text != 0
            && tinyui_message_box_set_confirm_text(box, props->confirm_text) != 0)) {
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

int tinyui_message_box_set_title(struct tinyui_message_box *box, const char *title)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0 || title == 0) {
        return -1;
    }

    ld_message_box = tinyui_message_box_get_ld(box);
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

int tinyui_message_box_set_message(struct tinyui_message_box *box, const char *message)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0 || message == 0) {
        return -1;
    }

    ld_message_box = tinyui_message_box_get_ld(box);
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

int tinyui_message_box_set_msg(struct tinyui_message_box *box, const char *message)
{
    return tinyui_message_box_set_message(box, message);
}

/**
 * @brief Set confirm text of message box widget
 *
 * @param[in] box box
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_message_box_set_confirm_text(struct tinyui_message_box *box, const char *text)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0 || text == 0) {
        return -1;
    }

    ld_message_box = tinyui_message_box_get_ld(box);
    if (ld_message_box == 0) {
        return -1;
    }

    box->confirm_text = text;
    ldMessageBoxSetBtn(ld_message_box, (const uint8_t **)&box->confirm_text, 1);
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

int tinyui_message_box_set_buttons(struct tinyui_message_box *box, const char *const *buttons, int count)
{
    int i;
    ldMessageBox_t *ld_message_box;

    if (box == 0 || buttons == 0 || count <= 0 || count > TINYUI_LIST_MAX_ITEMS) {
        return -1;
    }
    for (i = 0; i < count; ++i) {
        if (buttons[i] == 0) {
            return -1;
        }
    }

    ld_message_box = tinyui_message_box_get_ld(box);
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

int tinyui_message_box_set_btn(struct tinyui_message_box *box, const char *const *buttons, int count)
{
    return tinyui_message_box_set_buttons(box, buttons, count);
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

int tinyui_message_box_set_string_colors(struct tinyui_message_box *box,
                                         unsigned int title_color,
                                         unsigned int message_color,
                                         unsigned int button_color)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return -1;
    }

    ld_message_box = tinyui_message_box_get_ld(box);
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

int tinyui_message_box_set_string_color(struct tinyui_message_box *box,
                                        unsigned int title_color,
                                        unsigned int message_color,
                                        unsigned int button_color)
{
    return tinyui_message_box_set_string_colors(box, title_color, message_color, button_color);
}

/**
 * @brief Set button colors of message box widget
 *
 * @param[in] box box
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int tinyui_message_box_set_button_colors(struct tinyui_message_box *box,
                                         unsigned int release_color,
                                         unsigned int press_color)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return -1;
    }

    ld_message_box = tinyui_message_box_get_ld(box);
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

int tinyui_message_box_set_button_color(struct tinyui_message_box *box,
                                        unsigned int release_color,
                                        unsigned int press_color)
{
    return tinyui_message_box_set_button_colors(box, release_color, press_color);
}

/**
 * @brief Set bg color of message box widget
 *
 * @param[in] box box
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int tinyui_message_box_set_bg_color(struct tinyui_message_box *box, unsigned int bg_color)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return -1;
    }

    ld_message_box = tinyui_message_box_get_ld(box);
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

int tinyui_message_box_set_background_color(struct tinyui_message_box *box, unsigned int bg_color)
{
    return tinyui_message_box_set_bg_color(box, bg_color);
}

/**
 * @brief Set on confirm of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void tinyui_message_box_set_on_confirm(
    struct tinyui_message_box *box,
    tinyui_message_box_callback_t callback,
    void *user_data)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return;
    }

    box->on_confirm = callback;
    box->on_confirm_user_data = user_data;
    if (callback != 0) {
        ld_message_box = tinyui_message_box_get_ld(box);
        if (ld_message_box != 0) {
            ldMessageBoxSetCallback(ld_message_box, tinyui_message_box_confirm_bridge);
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

void tinyui_message_box_set_callback(
    struct tinyui_message_box *box,
    tinyui_message_box_callback_t callback,
    void *user_data)
{
    tinyui_message_box_set_on_confirm(box, callback, user_data);
}

/**
 * @brief Set on confirm indexed of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void tinyui_message_box_set_on_confirm_indexed(
    struct tinyui_message_box *box,
    tinyui_message_box_indexed_callback_t callback,
    void *user_data)
{
    ldMessageBox_t *ld_message_box;

    if (box == 0) {
        return;
    }

    box->on_confirm_indexed = callback;
    box->on_confirm_indexed_user_data = user_data;
    if (callback != 0) {
        ld_message_box = tinyui_message_box_get_ld(box);
        if (ld_message_box != 0) {
            ldMessageBoxSetCallback(ld_message_box, tinyui_message_box_confirm_bridge);
        }
    }
}

/**
 * @brief Get title of message box widget
 *
 * @param[in] box box
 */

const char *tinyui_message_box_get_title(const struct tinyui_message_box *box)
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

const char *tinyui_message_box_get_message(const struct tinyui_message_box *box)
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

const char *tinyui_message_box_get_confirm_text(const struct tinyui_message_box *box)
{
    if (box == 0) {
        return 0;
    }
    return box->confirm_text;
}
