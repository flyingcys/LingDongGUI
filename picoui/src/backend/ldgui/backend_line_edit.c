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
#include "runtime_bridge.h"
#include "ldBase.h"
#include "ldLineEdit.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

#define PICOUI_BACKEND_LINE_EDIT_TEXT_MAX 255

static ldLineEdit_t *picoui_backend_line_edit_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldLineEdit_t *)widget->ld_widget;
}

static arm_2d_align_t picoui_backend_line_edit_align_to_ld(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_CENTER:
        return ARM_2D_ALIGN_CENTRE;
    case PICOUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case PICOUI_ALIGN_START:
    default:
        return ARM_2D_ALIGN_LEFT;
    }
}

static ldColor picoui_backend_line_edit_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static bool picoui_backend_line_edit_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend;
    struct picoui_line_edit *line_edit;

    (void)scene;

    if (msg.ptSender == NULL) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return false;
    }

    line_edit = (struct picoui_line_edit *)backend->host_widget;
    backend->last_native_signal = msg.signal;
    backend->last_native_value = msg.value;
    if (msg.signal == SIGNAL_PRESS) {
        line_edit->editing = 1;
        backend->edit_result_on_finish = PICOUI_EDIT_RESULT_COMMIT;
        (void)picoui_backend_widget_claim_focus(backend);
        (void)picoui_widget_claim_editing(&line_edit->widget);
        return false;
    }

    if (msg.signal == SIGNAL_FINISHED) {
        line_edit->editing = 0;
        line_edit->widget.text = picoui_backend_line_edit_get_text(backend);
        (void)picoui_widget_mark_edit_result(&line_edit->widget, backend->edit_result_on_finish);
        (void)picoui_widget_release_editing(&line_edit->widget);
        backend->edit_result_on_finish = PICOUI_EDIT_RESULT_NONE;
        if (line_edit->on_edit_finished != 0) {
            line_edit->on_edit_finished(line_edit, line_edit->on_edit_finished_user_data);
        }
    }

    return false;
}

/**
 * @brief Create backend for line edit
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_line_edit(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldLineEdit_t *ld_line_edit;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_runtime_bridge_backend_state_from_parent(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(widget);
        return 0;
    }
    ld_line_edit = ldLineEdit_init(app_state->ld_scene,
                                   NULL,
                                   name_id,
                                   parent_widget->ld_name_id,
                                   0,
                                   0,
                                   220,
                                   32,
                                   (arm_2d_font_t *)&ARM_2D_FONT_6x8,
                                   PICOUI_BACKEND_LINE_EDIT_TEXT_MAX);
    if (ld_line_edit == NULL) {
        free(widget);
        return 0;
    }

    if (picoui_backend_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_TEXT,
                                         id,
                                         parent_widget->theme) != 0) {
        ldLineEdit_depose(app_state->ld_scene, ld_line_edit);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_line_edit;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldLineEdit_depose(app_state->ld_scene, ld_line_edit);
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief line: edit set text
 *
 * @param[in] backend_widget backend widget
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_text(void *backend_widget, const char *text)
{
    ldLineEdit_t *ld_line_edit;

    if (backend_widget == NULL || text == NULL) {
        return -1;
    }

    ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);
    if (ld_line_edit == NULL) {
        return -1;
    }

    ldLineEditSetText(ld_line_edit, (uint8_t *)text);
    ((struct picoui_backend_widget *)backend_widget)->text = text;
    return 0;
}

/**
 * @brief line: edit set align
 *
 * @param[in] backend_widget backend widget
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_align(void *backend_widget, enum picoui_align align)
{
    ldLineEdit_t *ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL) {
        return -1;
    }

    ldLineEditSetAlign(ld_line_edit, picoui_backend_line_edit_align_to_ld(align));
    return 0;
}

/**
 * @brief line: edit set color
 *
 * @param[in] backend_widget backend widget
 * @param[in] text_color Text color
 * @param[in] background_color background color
 * @param[in] frame_color frame color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_color(void *backend_widget,
                                       unsigned int text_color,
                                       unsigned int background_color,
                                       unsigned int frame_color)
{
    ldLineEdit_t *ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL) {
        return -1;
    }

    ldLineEditSetColor(ld_line_edit,
                       picoui_backend_line_edit_rgb_to_ld_color(text_color),
                       picoui_backend_line_edit_rgb_to_ld_color(background_color),
                       picoui_backend_line_edit_rgb_to_ld_color(frame_color));
    return 0;
}

/**
 * @brief line: edit get text
 *
 * @param[in] backend_widget backend widget
 */

const char *picoui_backend_line_edit_get_text(void *backend_widget)
{
    ldLineEdit_t *ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL) {
        return NULL;
    }

    return (const char *)ldLineEditGetText(ld_line_edit);
}

/**
 * @brief line: edit set type
 *
 * @param[in] backend_widget backend widget
 * @param[in] type Type
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_type(void *backend_widget, enum picoui_line_edit_type type)
{
    ldLineEdit_t *ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL) {
        return -1;
    }

    ldLineEditSetType(ld_line_edit, (ldEditType_t)type);
    return 0;
}

/**
 * @brief line: edit set keyboard
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_keyboard(void *backend_widget, unsigned int keyboard_binding)
{
    return picoui_backend_line_edit_set_keyboard_binding(backend_widget, keyboard_binding);
}

/**
 * @brief line: edit get type
 *
 * @param[in] backend_widget backend widget
 * @param[out] type Type
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_get_type(void *backend_widget, enum picoui_line_edit_type *type)
{
    ldLineEdit_t *ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL || type == NULL) {
        return -1;
    }

    *type = (enum picoui_line_edit_type)ld_line_edit->editType;
    return 0;
}

/**
 * @brief line: edit set keyboard binding
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_keyboard_binding(void *backend_widget,
                                                  unsigned int keyboard_binding)
{
    ldLineEdit_t *ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL || keyboard_binding == 0U || keyboard_binding > 0xFFFFU) {
        return -1;
    }

    ldLineEditSetKeyboard(ld_line_edit, (uint16_t)keyboard_binding);
    return 0;
}

/**
 * @brief line: edit get keyboard binding
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_get_keyboard_binding(void *backend_widget,
                                                  unsigned int *keyboard_binding)
{
    ldLineEdit_t *ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL || keyboard_binding == NULL) {
        return -1;
    }

    *keyboard_binding = (unsigned int)ld_line_edit->kbNameId;
    return 0;
}

/**
 * @brief line: edit bind host
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_bind_host(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldLineEdit_t *ld_line_edit;

    if (backend == NULL) {
        return -1;
    }

    ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);
    if (ld_line_edit == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_line_edit, SIGNAL_PRESS, picoui_backend_line_edit_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_line_edit, SIGNAL_FINISHED, picoui_backend_line_edit_native_slot)) {
        return -1;
    }
    return 0;
}

/**
 * @brief line: edit get editing
 *
 * @param[in] backend_widget backend widget
 * @param[in] editing editing
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_get_editing(void *backend_widget, int *editing)
{
    ldLineEdit_t *ld_line_edit = picoui_backend_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL || editing == NULL) {
        return -1;
    }

    *editing = ld_line_edit->isEditing ? 1 : 0;
    return 0;
}
