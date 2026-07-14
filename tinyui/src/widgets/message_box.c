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
#include "widgets/message_box.h"
#include "internal/widget_legacy.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldMessageBox.h"
#include "../../../src/gui/ldBase.h"


static struct tinyui_message_box *tinyui_message_box_as_message_box(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_MESSAGE_BOX)) {
        return 0;
    }
    return (struct tinyui_message_box *)w;
}

static const struct tinyui_message_box *tinyui_message_box_as_message_box_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_MESSAGE_BOX)) {
        return 0;
    }
    return (const struct tinyui_message_box *)w;
}


/* ── C2 depose / rollback ──────────────────────────────────────────────── */



static void tinyui_message_box_rollback(struct tinyui_message_box *box)
{
    if (box == 0) {
        return;
    }
    if (box->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&box->widget);
    } else {
        ldFree(box);
    }
}

/* ── props validation ─────────────────────────────────────────────────── */

static int tinyui_message_box_props_are_valid(const tinyui_message_box_props_t *props)
{
    return props != 0;
}

/* ── confirm bridge (native event slot) ────────────────────────────────── */

static void tinyui_message_box_confirm_bridge(ld_scene_t *scene, ldMessageBox_t *ld_message_box)
{
    struct tinyui_widget *w;
    struct tinyui_message_box *box;

    if (ld_message_box == 0) {
        return;
    }

    w = tinyui_runtime_internal_widget_from_ld_scene(scene, ld_message_box);
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

static void *tinyui_runtime_internal_message_box_ld_init(void *ctx,
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

tinyui_obj_t *tinyui_message_box_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "message_box";
    if (parent_w == 0) { return 0; }

    struct tinyui_message_box *box;
    struct tinyui_app *app_state;
    struct tinyui_message_box_create_ctx ctx;

    if (parent_w == 0 || id == 0 || parent_w->ld_widget == 0) {
        return 0;
    }

    app_state = parent_w->owner;
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    ctx.font = tinyui_resolve_ld_font(0, 12);
    if (ctx.font == 0) {
        return 0;
    }

    box = (struct tinyui_message_box *)tinyui_runtime_internal_widget_create_leaf(
        parent_w,
        TINYUI_BACKEND_WIDGET_MESSAGE_BOX,
        tinyui_runtime_internal_message_box_ld_init,
        &ctx,
        sizeof(*box));
    if (box == 0) {
        return 0;
    }

    box->id = id;
    return (tinyui_obj_t *)box;
}

/**
 * @brief message box init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_message_box *tinyui_runtime_internal_message_box_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_message_box_create(parent);
}

/**
 * @brief Create message box widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_message_box_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_message_box_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_message_box *box;

    if (props == 0) {
        return tinyui_message_box_create(parent);
    }

    obj = tinyui_message_box_create(parent);
    if (obj == 0) {
        return 0;
    }
    box = (struct tinyui_message_box *)(void *)obj;

    if ((props->fields & TINYUI_MESSAGE_BOX_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_MESSAGE_BOX_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&box->widget, props->user_data) != 0) {
        tinyui_message_box_rollback(box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_MESSAGE_BOX_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&box->widget, props->style_class) != 0) {
        tinyui_message_box_rollback(box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_MESSAGE_BOX_FIELD_TITLE) != 0) {
    if (tinyui_message_box_set_title((tinyui_obj_t *)box, props->title) != 0) {
        tinyui_message_box_rollback(box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_MESSAGE_BOX_FIELD_MESSAGE) != 0) {
    if (tinyui_message_box_set_message((tinyui_obj_t *)box, props->message) != 0) {
        tinyui_message_box_rollback(box);
        return 0;
    }
    }
    if ((props->fields & TINYUI_MESSAGE_BOX_FIELD_CONFIRM_TEXT) != 0) {
    if (tinyui_message_box_set_confirm_text((tinyui_obj_t *)box, props->confirm_text) != 0) {
        tinyui_message_box_rollback(box);
        return 0;
    }
    }

    return obj;
}


/* ── setters: direct (ldMessageBox_t *)widget->ld_widget cast ──────────── */

/**
 * @brief Set title of message box widget
 *
 * @param[in] box box
 * @param[in] title title
 * @return 0 on success, -1 on failure
 */

int tinyui_message_box_set_title(tinyui_obj_t *box_obj, const char *title)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

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

int tinyui_message_box_set_message(tinyui_obj_t *box_obj, const char *message)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

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

int tinyui_message_box_set_msg(tinyui_obj_t *box_obj, const char *message)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

    return tinyui_message_box_set_message((tinyui_obj_t *)box, message);
}

/**
 * @brief Set confirm text of message box widget
 *
 * @param[in] box box
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_message_box_set_confirm_text(tinyui_obj_t *box_obj, const char *text)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

    if (box == 0 || text == 0 || box->widget.ld_widget == 0) {
        return -1;
    }

    box->confirm_text = text;
    ldMessageBoxSetBtn((ldMessageBox_t *)box->widget.ld_widget,
                       (const uint8_t **)&box->confirm_text, 1);
    return 0;
}

int tinyui_message_box_set_layout(tinyui_obj_t *box_obj, int width, int height)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

    ldMessageBox_t *ld_message_box;
    ldBase_t *ld_base;
    const uint8_t **button_group;
    uint8_t button_count;

    if (box == 0 || width <= 0 || height <= 0 || box->widget.ld_widget == 0) {
        return -1;
    }

    ld_message_box = (ldMessageBox_t *)box->widget.ld_widget;
    ld_base = (ldBase_t *)ld_message_box;
    ldBaseSetWidth(ld_base, (int16_t)width);
    ldBaseSetHeight(ld_base, (int16_t)height);
    ld_message_box->titleHeight = (uint8_t)((height
                                             - ld_message_box->padding.top
                                             - ld_message_box->padding.bottom) / 5);
    ld_message_box->msgHeight = (uint8_t)(ld_message_box->titleHeight * 3);
    button_group = ld_message_box->ppBtnStrGroup;
    button_count = ld_message_box->btnCount;
    if (button_group != 0 && button_count != 0) {
        ldMessageBoxSetBtn(ld_message_box, button_group, button_count);
    }
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

int tinyui_message_box_set_buttons(tinyui_obj_t *box_obj, const char *const *buttons, int count)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

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

int tinyui_message_box_set_btn(tinyui_obj_t *box_obj, const char *const *buttons, int count)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

    return tinyui_message_box_set_buttons((tinyui_obj_t *)box, buttons, count);
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

int tinyui_message_box_set_string_colors(tinyui_obj_t *box_obj, unsigned int title_color, unsigned int message_color, unsigned int button_color)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

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

int tinyui_message_box_set_string_color(tinyui_obj_t *box_obj, unsigned int title_color, unsigned int message_color, unsigned int button_color)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

    return tinyui_message_box_set_string_colors((tinyui_obj_t *)box, title_color, message_color, button_color);
}

/**
 * @brief Set button colors of message box widget
 *
 * @param[in] box box
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int tinyui_message_box_set_button_colors(tinyui_obj_t *box_obj, unsigned int release_color, unsigned int press_color)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

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

int tinyui_message_box_set_button_color(tinyui_obj_t *box_obj, unsigned int release_color, unsigned int press_color)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

    return tinyui_message_box_set_button_colors((tinyui_obj_t *)box, release_color, press_color);
}

/**
 * @brief Set bg color of message box widget
 *
 * @param[in] box box
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int tinyui_message_box_set_bg_color(tinyui_obj_t *box_obj, unsigned int bg_color)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

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

int tinyui_message_box_set_background_color(tinyui_obj_t *box_obj, unsigned int bg_color)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return -1; }

    return tinyui_message_box_set_bg_color((tinyui_obj_t *)box, bg_color);
}

/**
 * @brief Set on confirm of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void tinyui_message_box_set_on_confirm(tinyui_obj_t *box_obj, tinyui_message_box_callback_t callback, void *user_data)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return; }

    if (box == 0) {
        return;
    }

    box->on_confirm = (void *)callback;
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

void tinyui_message_box_set_callback(tinyui_obj_t *box_obj, tinyui_message_box_callback_t callback, void *user_data)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return; }

    tinyui_message_box_set_on_confirm((tinyui_obj_t *)box, callback, user_data);
}

/**
 * @brief Set on confirm indexed of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void tinyui_message_box_set_on_confirm_indexed(tinyui_obj_t *box_obj, tinyui_message_box_indexed_callback_t callback, void *user_data)
{
    struct tinyui_message_box *box = tinyui_message_box_as_message_box(box_obj);
    if (box == 0) { return; }

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

const char * tinyui_message_box_get_title(const tinyui_obj_t *box_obj)
{
    const struct tinyui_message_box *box = tinyui_message_box_as_message_box_const(box_obj);
    if (box == 0) { return 0; }

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

const char * tinyui_message_box_get_message(const tinyui_obj_t *box_obj)
{
    const struct tinyui_message_box *box = tinyui_message_box_as_message_box_const(box_obj);
    if (box == 0) { return 0; }

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

const char * tinyui_message_box_get_confirm_text(const tinyui_obj_t *box_obj)
{
    const struct tinyui_message_box *box = tinyui_message_box_as_message_box_const(box_obj);
    if (box == 0) { return 0; }

    if (box == 0) {
        return 0;
    }
    return box->confirm_text;
}
