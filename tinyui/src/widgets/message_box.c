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

/* ── C2 depose / rollback ──────────────────────────────────────────────── */

static ld_scene_t *s_message_box_depose_scene = NULL;

static void s_message_box_depose_cb(void *ld_widget)
{
    if (s_message_box_depose_scene != NULL) {
        ldMessageBox_depose(s_message_box_depose_scene, (ldMessageBox_t *)ld_widget);
        s_message_box_depose_scene = NULL;
    }
}

static void tinyui_message_box_rollback(struct tinyui_message_box *box)
{
    if (box == 0) {
        return;
    }
    if (box->widget.ld_widget != 0) {
        s_message_box_depose_scene = box->widget.owner != 0
            ? box->widget.owner->ld_scene
            : NULL;
        tinyui_widget_destroy_common(&box->widget, s_message_box_depose_cb);
    } else {
        free(box);
    }
}

/* ── props validation ─────────────────────────────────────────────────── */

static int tinyui_message_box_props_are_valid(const struct tinyui_message_box_props *props)
{
    return props != 0 && props->id != 0;
}

/* ── confirm bridge (native event slot) ────────────────────────────────── */

static void tinyui_message_box_confirm_bridge(ld_scene_t *scene, ldMessageBox_t *ld_message_box)
{
    struct tinyui_widget *w;
    struct tinyui_message_box *box;

    if (ld_message_box == 0) {
        return;
    }

    w = tinyui_widget_from_ld_scene(scene, ld_message_box);
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

struct tinyui_message_box_create_ctx {
    arm_2d_font_t *font;
};

static void *tinyui_message_box_ld_init(void *ctx,
                                        ld_scene_t *scene,
                                        uint16_t name_id,
                                        uint16_t parent_name_id)
{
    struct tinyui_message_box_create_ctx *create_ctx;

    if (scene == 0 || ctx == 0) {
        return 0;
    }

    create_ctx = (struct tinyui_message_box_create_ctx *)ctx;
    return ldMessageBox_init(scene,
                             NULL,
                             name_id,
                             parent_name_id,
                             0,
                             0,
                             260,
                             140,
                             create_ctx->font);
}

/* ── create ────────────────────────────────────────────────────────────── */

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
    struct tinyui_message_box_create_ctx ctx;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }

    app_state = parent->owner;
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    ctx.font = (arm_2d_font_t *)&ARM_2D_FONT_6x8;

    box = (struct tinyui_message_box *)tinyui_widget_create_leaf(
        parent,
        TINYUI_BACKEND_WIDGET_MESSAGE_BOX,
        tinyui_message_box_ld_init,
        &ctx,
        sizeof(*box));
    if (box == 0) {
        return 0;
    }

    box->id = id;
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
        tinyui_message_box_rollback(box);
        return 0;
    }
    if (tinyui_widget_set_user_data(&box->widget, props->user_data) != 0) {
        tinyui_message_box_rollback(box);
        return 0;
    }
    if ((props->title != 0 && tinyui_message_box_set_title(box, props->title) != 0)
        || (props->message != 0 && tinyui_message_box_set_message(box, props->message) != 0)
        || (props->confirm_text != 0
            && tinyui_message_box_set_confirm_text(box, props->confirm_text) != 0)) {
        tinyui_message_box_rollback(box);
        return 0;
    }

    return box;
}

/* ── setters: direct (ldMessageBox_t *)widget->ld_widget cast ──────────── */

/**
 * @brief Set title of message box widget
 *
 * @param[in] box box
 * @param[in] title title
 * @return 0 on success, -1 on failure
 */

int tinyui_message_box_set_title(struct tinyui_message_box *box, const char *title)
{
    if (box == 0 || title == 0 || box->widget.ld_widget == 0) {
        return -1;
    }

    ldMessageBoxSetTitle((ldMessageBox_t *)box->widget.ld_widget, (const uint8_t *)title);
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
    if (box == 0 || message == 0 || box->widget.ld_widget == 0) {
        return -1;
    }

    ldMessageBoxSetMsg((ldMessageBox_t *)box->widget.ld_widget, (const uint8_t *)message);
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
    if (box == 0 || text == 0 || box->widget.ld_widget == 0) {
        return -1;
    }

    box->confirm_text = text;
    ldMessageBoxSetBtn((ldMessageBox_t *)box->widget.ld_widget,
                       (const uint8_t **)&box->confirm_text, 1);
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

    if (box == 0 || buttons == 0 || count <= 0 || count > TINYUI_LIST_MAX_ITEMS
        || box->widget.ld_widget == 0) {
        return -1;
    }
    for (i = 0; i < count; ++i) {
        if (buttons[i] == 0) {
            return -1;
        }
    }

    ldMessageBoxSetBtn((ldMessageBox_t *)box->widget.ld_widget,
                       (const uint8_t **)buttons, (uint8_t)count);
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
    if (box == 0 || box->widget.ld_widget == 0) {
        return -1;
    }

    ldMessageBoxSetStringColor((ldMessageBox_t *)box->widget.ld_widget,
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
    if (box == 0 || box->widget.ld_widget == 0) {
        return -1;
    }

    ldMessageBoxSetButtonColor((ldMessageBox_t *)box->widget.ld_widget,
                               (ldColor)release_color, (ldColor)press_color);
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
    if (box == 0 || box->widget.ld_widget == 0) {
        return -1;
    }

    ldMessageBoxSetBackgroundColor((ldMessageBox_t *)box->widget.ld_widget, (ldColor)bg_color);
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
    if (box == 0) {
        return;
    }

    box->on_confirm = callback;
    box->on_confirm_user_data = user_data;
    if (callback != 0 && box->widget.ld_widget != 0) {
        ldMessageBoxSetCallback((ldMessageBox_t *)box->widget.ld_widget,
                                tinyui_message_box_confirm_bridge);
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
    if (box == 0) {
        return;
    }

    box->on_confirm_indexed = callback;
    box->on_confirm_indexed_user_data = user_data;
    if (callback != 0 && box->widget.ld_widget != 0) {
        ldMessageBoxSetCallback((ldMessageBox_t *)box->widget.ld_widget,
                                tinyui_message_box_confirm_bridge);
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
