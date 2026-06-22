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
#include "line_edit.h"

#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldLineEdit.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

#define TINYUI_BACKEND_LINE_EDIT_TEXT_MAX 255

int tinyui_runtime_bridge_unbind_host(void *backend_widget);

static ldLineEdit_t *tinyui_line_edit_get_ld(void *widget_ptr)
{
    struct tinyui_widget *widget = widget_ptr;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldLineEdit_t *)widget->ld_widget;
}

static arm_2d_align_t tinyui_line_edit_align_to_ld(enum tinyui_align align)
{
    switch (align) {
    case TINYUI_ALIGN_CENTER:
        return ARM_2D_ALIGN_CENTRE;
    case TINYUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case TINYUI_ALIGN_START:
    default:
        return ARM_2D_ALIGN_LEFT;
    }
}

static ldColor tinyui_line_edit_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static const char *tinyui_line_edit_get_text_ld(void *backend_widget);

static bool tinyui_line_edit_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    /* C1-T7: pInfo now points to tinyui_widget, not tinyui_backend_widget */
    struct tinyui_widget *w;
    struct tinyui_line_edit *line_edit;

    (void)scene;

    if (msg.ptSender == NULL) {
        return false;
    }

    w = (struct tinyui_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (w == NULL) {
        return false;
    }

    line_edit = (struct tinyui_line_edit *)w;
    if (msg.signal == SIGNAL_PRESS) {
        line_edit->editing = 1;
        w->edit_result_on_finish = TINYUI_EDIT_RESULT_COMMIT;
        (void)tinyui_widget_claim_backend_focus(w);
        (void)tinyui_widget_claim_editing(&line_edit->widget);
        return false;
    }

    if (msg.signal == SIGNAL_FINISHED) {
        ldLineEdit_t *ld_line_edit = (ldLineEdit_t *)w->ld_widget;
        line_edit->editing = 0;
        line_edit->widget.text = (ld_line_edit != NULL)
                                     ? (const char *)ldLineEditGetText(ld_line_edit)
                                     : NULL;
        (void)tinyui_widget_mark_edit_result(&line_edit->widget, w->edit_result_on_finish);
        (void)tinyui_widget_release_editing(&line_edit->widget);
        w->edit_result_on_finish = TINYUI_EDIT_RESULT_NONE;
        if (line_edit->on_edit_finished != 0) {
            line_edit->on_edit_finished(line_edit, line_edit->on_edit_finished_user_data);
        }
    }

    return false;
}

static int tinyui_line_edit_type_is_valid(enum tinyui_line_edit_type type)
{
    return type >= TINYUI_LINE_EDIT_TYPE_STRING && type <= TINYUI_LINE_EDIT_TYPE_FLOAT;
}

static int tinyui_line_edit_keyboard_binding_is_valid(unsigned int keyboard_binding)
{
    return keyboard_binding > 0U && keyboard_binding <= 0xFFFFU;
}

static int tinyui_line_edit_props_are_valid(const struct tinyui_line_edit_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->radius >= 0 &&
           props->padding >= 0 &&
           (props->has_type == 0 || tinyui_line_edit_type_is_valid(props->type)) &&
           (props->has_keyboard_binding == 0 ||
            tinyui_line_edit_keyboard_binding_is_valid(props->keyboard_binding));
}

int tinyui_line_edit_set_text_ld(void *backend_widget, const char *text)
{
    ldLineEdit_t *ld_line_edit;

    if (backend_widget == NULL || text == NULL) {
        return -1;
    }

    ld_line_edit = tinyui_line_edit_get_ld(backend_widget);
    if (ld_line_edit == NULL) {
        return -1;
    }

    ldLineEditSetText(ld_line_edit, (uint8_t *)text);
    ((struct tinyui_widget *)backend_widget)->text = text;
    return 0;
}

int tinyui_line_edit_set_align_ld(void *backend_widget, enum tinyui_align align)
{
    ldLineEdit_t *ld_line_edit = tinyui_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL) {
        return -1;
    }

    ldLineEditSetAlign(ld_line_edit, tinyui_line_edit_align_to_ld(align));
    return 0;
}

int tinyui_line_edit_set_color_ld(void *backend_widget,
                                       unsigned int text_color,
                                       unsigned int background_color,
                                       unsigned int frame_color)
{
    ldLineEdit_t *ld_line_edit = tinyui_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL) {
        return -1;
    }

    ldLineEditSetColor(ld_line_edit,
                       tinyui_line_edit_rgb_to_ld_color(text_color),
                       tinyui_line_edit_rgb_to_ld_color(background_color),
                       tinyui_line_edit_rgb_to_ld_color(frame_color));
    return 0;
}

const char *tinyui_line_edit_get_text_ld(void *backend_widget)
{
    ldLineEdit_t *ld_line_edit = tinyui_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL) {
        return NULL;
    }

    return (const char *)ldLineEditGetText(ld_line_edit);
}

int tinyui_line_edit_set_type_ld(void *backend_widget, enum tinyui_line_edit_type type)
{
    ldLineEdit_t *ld_line_edit = tinyui_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL) {
        return -1;
    }

    ldLineEditSetType(ld_line_edit, (ldEditType_t)type);
    return 0;
}

int tinyui_line_edit_get_type_ld(void *backend_widget, enum tinyui_line_edit_type *type)
{
    ldLineEdit_t *ld_line_edit = tinyui_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL || type == NULL) {
        return -1;
    }

    *type = (enum tinyui_line_edit_type)ld_line_edit->editType;
    return 0;
}

int tinyui_line_edit_set_keyboard_binding_ld(void *backend_widget,
                                                  unsigned int keyboard_binding)
{
    ldLineEdit_t *ld_line_edit = tinyui_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL || keyboard_binding == 0U || keyboard_binding > 0xFFFFU) {
        return -1;
    }

    ldLineEditSetKeyboard(ld_line_edit, (uint16_t)keyboard_binding);
    return 0;
}

int tinyui_line_edit_get_keyboard_binding_ld(void *backend_widget,
                                                  unsigned int *keyboard_binding)
{
    ldLineEdit_t *ld_line_edit = tinyui_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL || keyboard_binding == NULL) {
        return -1;
    }

    *keyboard_binding = (unsigned int)ld_line_edit->kbNameId;
    return 0;
}

int tinyui_line_edit_bind_host(void *backend_widget)
{
    ldLineEdit_t *ld_line_edit;

    if (backend_widget == NULL) {
        return -1;
    }

    ld_line_edit = tinyui_line_edit_get_ld(backend_widget);
    if (ld_line_edit == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_line_edit, SIGNAL_PRESS, tinyui_line_edit_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_line_edit, SIGNAL_FINISHED, tinyui_line_edit_native_slot)) {
        return -1;
    }
    return 0;
}

int tinyui_line_edit_get_editing_ld(void *backend_widget, int *editing)
{
    ldLineEdit_t *ld_line_edit = tinyui_line_edit_get_ld(backend_widget);

    if (ld_line_edit == NULL || editing == NULL) {
        return -1;
    }

    *editing = ld_line_edit->isEditing ? 1 : 0;
    return 0;
}

static void tinyui_line_edit_cleanup(struct tinyui_line_edit *line_edit)
{
    struct tinyui_app *app_state;

    if (line_edit == 0) {
        return;
    }

    if (line_edit->widget.ld_widget != 0) {
        void *saved_ld_widget = line_edit->widget.ld_widget;
        app_state = line_edit->widget.owner != 0
            ? tinyui_runtime_bridge_backend_state(line_edit->widget.owner)
            : 0;
        (void)tinyui_widget_detach_from_parent(&line_edit->widget);
        (void)tinyui_runtime_bridge_unbind_host(&line_edit->widget);
        if (app_state != 0 && app_state->ld_scene != 0) {
            ldLineEdit_depose(app_state->ld_scene, (ldLineEdit_t *)saved_ld_widget);
        }
    }

    free(line_edit);
}

struct tinyui_line_edit *tinyui_line_edit_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_line_edit *line_edit;
    struct tinyui_app *app_state;
    ldLineEdit_t *ld_line_edit;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = parent->widget.owner;
    if (parent->widget.ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    line_edit = calloc(1, sizeof(*line_edit));
    if (line_edit == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;

    ld_line_edit = ldLineEdit_init(app_state->ld_scene,
                                   NULL,
                                   name_id,
                                   parent->widget.ld_name_id,
                                   0,
                                   0,
                                   220,
                                   32,
                                   (arm_2d_font_t *)&ARM_2D_FONT_6x8,
                                   TINYUI_BACKEND_LINE_EDIT_TEXT_MAX);
    if (ld_line_edit == NULL) {
        free(line_edit);
        return 0;
    }

    line_edit->id = id;
    line_edit->type = TINYUI_LINE_EDIT_TYPE_STRING;
    line_edit->widget.ld_widget  = ld_line_edit;
    line_edit->widget.ld_name_id = name_id;
    line_edit->widget.kind       = TINYUI_BACKEND_WIDGET_TEXT;
    line_edit->widget.owner      = app_state;
    line_edit->widget.visible    = 1;
    line_edit->widget.enabled    = 1;
    ((ldBase_t *)ld_line_edit)->pInfo = &line_edit->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&line_edit->widget, app_state);

    if (tinyui_line_edit_bind_host(&line_edit->widget) != 0) {
        tinyui_line_edit_cleanup(line_edit);
        return 0;
    }

    return line_edit;
}

struct tinyui_line_edit *tinyui_line_edit_create_with_props(struct tinyui_window *parent,
                                                            const struct tinyui_line_edit_props *props)
{
    struct tinyui_line_edit *line_edit;

    if (!tinyui_line_edit_props_are_valid(props)) {
        return 0;
    }

    line_edit = tinyui_line_edit_create(parent, props->id);
    if (line_edit == 0) {
        return 0;
    }

    if ((props->text != 0 && tinyui_line_edit_set_text(line_edit, props->text) != 0) ||
        (props->has_type != 0 && tinyui_line_edit_set_type(line_edit, props->type) != 0) ||
        (props->has_keyboard_binding != 0 &&
         tinyui_line_edit_set_keyboard_binding(line_edit, props->keyboard_binding) != 0)) {
        tinyui_line_edit_cleanup(line_edit);
        return 0;
    }

    if (tinyui_widget_set_user_data(&line_edit->widget, props->user_data) != 0 ||
        tinyui_widget_set_bg_color(&line_edit->widget, props->bg_color) != 0 ||
        tinyui_widget_set_text_color(&line_edit->widget, props->text_color) != 0 ||
        tinyui_widget_set_border_color(&line_edit->widget, props->border_color) != 0 ||
        tinyui_widget_set_radius(&line_edit->widget, props->radius) != 0 ||
        tinyui_widget_set_padding(&line_edit->widget, props->padding) != 0) {
        tinyui_line_edit_cleanup(line_edit);
        return 0;
    }
    if (props->style_class != 0 &&
        tinyui_widget_set_style_class(&line_edit->widget, props->style_class) != 0) {
        tinyui_line_edit_cleanup(line_edit);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        tinyui_widget_set_size(&line_edit->widget, props->width, props->height) != 0) {
        tinyui_line_edit_cleanup(line_edit);
        return 0;
    }

    return line_edit;
}

int tinyui_line_edit_set_text(struct tinyui_line_edit *line_edit, const char *text)
{
    if (line_edit == 0 || text == 0) {
        return -1;
    }

    if (tinyui_widget_set_text(&line_edit->widget, text) != 0) {
        return -1;
    }

    return tinyui_line_edit_set_text_ld(&line_edit->widget, text);
}

int tinyui_line_edit_set_align(struct tinyui_line_edit *line_edit, enum tinyui_align align)
{
    if (line_edit == 0) {
        return -1;
    }

    if (tinyui_line_edit_set_align_ld(&line_edit->widget, align) != 0) {
        return -1;
    }

    line_edit->align = align;
    return 0;
}

int tinyui_line_edit_set_color(struct tinyui_line_edit *line_edit,
                               unsigned int text_color,
                               unsigned int background_color,
                               unsigned int frame_color)
{
    if (line_edit == 0) {
        return -1;
    }

    if (tinyui_line_edit_set_color_ld(&line_edit->widget,
                                           text_color,
                                           background_color,
                                           frame_color) != 0) {
        return -1;
    }

    line_edit->widget.text_color = text_color;
    line_edit->widget.bg_color = background_color;
    line_edit->widget.border_color = frame_color;
    return 0;
}

const char *tinyui_line_edit_get_text(const struct tinyui_line_edit *line_edit)
{
    const char *backend_text;

    if (line_edit == 0) {
        return 0;
    }

    backend_text = tinyui_line_edit_get_text_ld((void *)&line_edit->widget);
    if (backend_text != 0) {
        return backend_text;
    }

    return line_edit->widget.text;
}

int tinyui_line_edit_set_type(struct tinyui_line_edit *line_edit, enum tinyui_line_edit_type type)
{
    if (line_edit == 0 || !tinyui_line_edit_type_is_valid(type)) {
        return -1;
    }

    if (tinyui_line_edit_set_type_ld(&line_edit->widget, type) != 0) {
        return -1;
    }

    line_edit->type = type;
    return 0;
}

int tinyui_line_edit_get_type(const struct tinyui_line_edit *line_edit,
                              enum tinyui_line_edit_type *type)
{
    if (line_edit == 0 || type == 0) {
        return -1;
    }

    return tinyui_line_edit_get_type_ld((void *)&line_edit->widget, type);
}

int tinyui_line_edit_set_keyboard(struct tinyui_line_edit *line_edit, unsigned int keyboard_binding)
{
    return tinyui_line_edit_set_keyboard_binding(line_edit, keyboard_binding);
}

int tinyui_line_edit_set_keyboard_binding(struct tinyui_line_edit *line_edit,
                                          unsigned int keyboard_binding)
{
    if (line_edit == 0 || !tinyui_line_edit_keyboard_binding_is_valid(keyboard_binding)) {
        return -1;
    }

    if (tinyui_line_edit_set_keyboard_binding_ld(&line_edit->widget,
                                                      keyboard_binding) != 0) {
        return -1;
    }

    line_edit->keyboard_binding = keyboard_binding;
    return 0;
}

int tinyui_line_edit_get_keyboard_binding(const struct tinyui_line_edit *line_edit,
                                          unsigned int *keyboard_binding)
{
    if (line_edit == 0 || keyboard_binding == 0) {
        return -1;
    }

    return tinyui_line_edit_get_keyboard_binding_ld((void *)&line_edit->widget,
                                                         keyboard_binding);
}

int tinyui_line_edit_get_editing(const struct tinyui_line_edit *line_edit, int *editing)
{
    if (line_edit == 0 || editing == 0) {
        return -1;
    }

    return tinyui_line_edit_get_editing_ld((void *)&line_edit->widget, editing);
}

int tinyui_line_edit_set_on_edit_finished(struct tinyui_line_edit *line_edit,
                                          tinyui_line_edit_finished_cb cb,
                                          void *user_data)
{
    if (line_edit == 0) {
        return -1;
    }

    line_edit->on_edit_finished = cb;
    line_edit->on_edit_finished_user_data = user_data;
    return 0;
}
