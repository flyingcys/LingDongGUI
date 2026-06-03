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

#include "backend.h"
#include "internal.h"
#include "ldBase.h"
#include "ldMessageBox.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static struct picoui_backend_app_state *picoui_backend_message_box_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldMessageBox_t *picoui_backend_message_box_get_ld(struct picoui_message_box *box)
{
    struct picoui_backend_widget *backend;

    if (box == NULL || box->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_MESSAGE_BOX || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldMessageBox_t *)backend->ld_widget;
}

static void picoui_backend_message_box_confirm_bridge(ld_scene_t *scene, ldMessageBox_t *ld_message_box)
{
    struct picoui_backend_widget *backend;
    struct picoui_message_box *box;

    (void)scene;

    if (ld_message_box == NULL) {
        return;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)ld_message_box)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
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
 * @brief Create backend for message box
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_message_box(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldMessageBox_t *ld_message_box;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_message_box_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_message_box = ldMessageBox_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent_widget->ld_name_id,
                                       0,
                                       0,
                                       260,
                                       140,
                                       (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_message_box == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_MESSAGE_BOX;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_message_box;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief message: box set title
 *
 * @param[in] box box
 * @param[in] title title
 * @return 0 on success, -1 on failure
 */

int picoui_backend_message_box_set_title(struct picoui_message_box *box, const char *title)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL || title == NULL) {
        return -1;
    }

    ldMessageBoxSetTitle(ld_message_box, (const uint8_t *)title);
    return 0;
}

/**
 * @brief message: box set message
 *
 * @param[in] box box
 * @param[in] message message
 * @return 0 on success, -1 on failure
 */

int picoui_backend_message_box_set_message(struct picoui_message_box *box, const char *message)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL || message == NULL) {
        return -1;
    }

    ldMessageBoxSetMsg(ld_message_box, (const uint8_t *)message);
    return 0;
}

/**
 * @brief message: box set confirm text
 *
 * @param[in] box box
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_message_box_set_confirm_text(struct picoui_message_box *box, const char *text)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);
    struct picoui_backend_widget *backend;

    if (ld_message_box == NULL || text == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    if (backend == NULL) {
        return -1;
    }

    backend->text = text;
    ldMessageBoxSetBtn(ld_message_box, (const uint8_t **)&backend->text, 1);
    return 0;
}

/**
 * @brief message: box set buttons
 *
 * @param[in] box box
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_backend_message_box_set_buttons(struct picoui_message_box *box,
                                           const char *const *buttons,
                                           int count)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL || buttons == NULL || count <= 0) {
        return -1;
    }

    ldMessageBoxSetBtn(ld_message_box, (const uint8_t **)buttons, (uint8_t)count);
    return 0;
}

/**
 * @brief message: box set string colors
 *
 * @param[in] box box
 * @param[in] title_color title color
 * @param[in] message_color message color
 * @param[in] button_color button color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_message_box_set_string_colors(struct picoui_message_box *box,
                                                 unsigned int title_color,
                                                 unsigned int message_color,
                                                 unsigned int button_color)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL) {
        return -1;
    }

    ldMessageBoxSetStringColor(ld_message_box,
                               (ldColor)title_color,
                               (ldColor)message_color,
                               (ldColor)button_color);
    return 0;
}

/**
 * @brief message: box set button colors
 *
 * @param[in] box box
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_message_box_set_button_colors(struct picoui_message_box *box,
                                                 unsigned int release_color,
                                                 unsigned int press_color)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL) {
        return -1;
    }

    ldMessageBoxSetButtonColor(ld_message_box, (ldColor)release_color, (ldColor)press_color);
    return 0;
}

/**
 * @brief message: box set bg color
 *
 * @param[in] box box
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_message_box_set_bg_color(struct picoui_message_box *box, unsigned int bg_color)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL) {
        return -1;
    }

    ldMessageBoxSetBackgroundColor(ld_message_box, (ldColor)bg_color);
    return 0;
}

/**
 * @brief message: box set on confirm
 *
 * @param[in] box box
 * @return 0 on success, -1 on failure
 */

int picoui_backend_message_box_set_on_confirm(struct picoui_message_box *box)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL) {
        return -1;
    }

    ldMessageBoxSetCallback(ld_message_box, picoui_backend_message_box_confirm_bridge);
    return 0;
}
