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
#include "widgets/line_edit.h"
#include "widgets/keyboard.h"

#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldLineEdit.h"

#include <string.h>


static struct tinyui_line_edit *tinyui_line_edit_as_line_edit(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_LINE_EDIT)) {
        return 0;
    }
    return (struct tinyui_line_edit *)w;
}

static const struct tinyui_line_edit *tinyui_line_edit_as_line_edit_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_LINE_EDIT)) {
        return 0;
    }
    return (const struct tinyui_line_edit *)w;
}


#define TINYUI_BACKEND_LINE_EDIT_TEXT_MAX 255



static void tinyui_line_edit_rollback(struct tinyui_line_edit *line_edit)
{
    if (line_edit == 0) {
        return;
    }
    if (line_edit->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&line_edit->widget);
    } else {
        ldFree(line_edit);
    }
}

static bool tinyui_line_edit_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct tinyui_widget *w;
    struct tinyui_line_edit *line_edit;
    ldLineEdit_t *ld_line_edit;

    if (msg.ptSender == NULL) {
        return false;
    }

    w = tinyui_runtime_internal_widget_from_ld_scene(scene, msg.ptSender);
    if (w == NULL) {
        return false;
    }

    line_edit = (struct tinyui_line_edit *)w;
    ld_line_edit = (ldLineEdit_t *)w->ld_widget;

    if (msg.signal == SIGNAL_PRESS) {
        line_edit->editing = 1;
        if (ld_line_edit != NULL) {
            ld_line_edit->isEditing = true;
        }
        w->edit_result_on_finish = TINYUI_EDIT_RESULT_COMMIT;
        (void)tinyui_runtime_internal_widget_claim_backend_focus(w);
        (void)tinyui_runtime_internal_widget_claim_editing(&line_edit->widget);
        return false;
    }

    if (msg.signal == SIGNAL_FINISHED) {
        line_edit->editing = 0;
        if (ld_line_edit != NULL) {
            ld_line_edit->isEditing = false;
            line_edit->widget.text = (const char *)ldLineEditGetText(ld_line_edit);
        } else {
            line_edit->widget.text = NULL;
        }
        (void)tinyui_runtime_internal_widget_mark_edit_result(
            &line_edit->widget,
            (enum tinyui_edit_result)w->edit_result_on_finish);
        (void)tinyui_runtime_internal_widget_release_editing(&line_edit->widget);
        w->edit_result_on_finish = TINYUI_EDIT_RESULT_NONE;
        /*
         * Dedicated finished callback remains the line_edit-specific contract.
         * Unified event pool (tinyui_event_fire) is static inside core/event.c and
         * dispatch_native_signal has no LINE_EDIT case; cannot emit
         * TINYUI_EVENT_VALUE_CHANGED from this TU without shared-core changes.
         */
        if (line_edit->on_edit_finished != 0) {
            line_edit->on_edit_finished((tinyui_obj_t *)line_edit,
                                        line_edit->on_edit_finished_user_data);
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

static int tinyui_line_edit_props_are_valid(const tinyui_line_edit_props_t *props)
{
    if (props == 0) {
        return 0;
    }

    if ((props->fields & TINYUI_LINE_EDIT_FIELD_WIDTH) != 0 && props->width < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_HEIGHT) != 0 && props->height < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_RADIUS) != 0 && props->radius < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_PADDING) != 0 && props->padding < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_TYPE) != 0
        && !tinyui_line_edit_type_is_valid(props->type)) {
        return 0;
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_KEYBOARD_BINDING) != 0
        && !tinyui_line_edit_keyboard_binding_is_valid(props->keyboard_binding)) {
        return 0;
    }

    return 1;
}

static void *tinyui_runtime_internal_line_edit_ld_init(void *ctx,
                                      struct ld_scene_t *scene,
                                      uint16_t name_id,
                                      uint16_t parent_name_id)
{
    (void)ctx;
    return ldLineEdit_init(scene,
                           NULL,
                           name_id,
                           parent_name_id,
                           0,
                           0,
                           220,
                           32,
                           tinyui_resolve_ld_font(0, 12),
                           TINYUI_BACKEND_LINE_EDIT_TEXT_MAX);
}

tinyui_obj_t *tinyui_line_edit_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "line_edit";
    struct tinyui_line_edit *line_edit;
    ldLineEdit_t *ld_line_edit;

    if (parent_w == 0) {
        return 0;
    }

    if (parent_w->ld_widget == 0 || parent_w->owner == 0) {
        return 0;
    }

    line_edit = (struct tinyui_line_edit *)tinyui_runtime_internal_widget_create_leaf(
        parent_w,
        TINYUI_BACKEND_WIDGET_LINE_EDIT,
        tinyui_runtime_internal_line_edit_ld_init,
        0,
        sizeof(*line_edit));
    if (line_edit == 0) {
        return 0;
    }

    line_edit->id = id;
    line_edit->type = TINYUI_LINE_EDIT_TYPE_STRING;
    line_edit->editing = 0;
    line_edit->keyboard_binding = 0U;
    line_edit->on_edit_finished = 0;
    line_edit->on_edit_finished_user_data = 0;
    line_edit->widget.visible = 1;
    line_edit->widget.enabled = 1;
    ld_line_edit = (ldLineEdit_t *)line_edit->widget.ld_widget;

    if (!ldMsgConnect(ld_line_edit, SIGNAL_PRESS, tinyui_line_edit_native_slot)
        || !ldMsgConnect(ld_line_edit, SIGNAL_FINISHED, tinyui_line_edit_native_slot)) {
        tinyui_line_edit_rollback(line_edit);
        return 0;
    }

    return (tinyui_obj_t *)line_edit;
}

tinyui_obj_t *tinyui_line_edit_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_line_edit_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_line_edit *line_edit;

    if (props == 0) {
        return tinyui_line_edit_create(parent);
    }

    if (!tinyui_line_edit_props_are_valid(props)) {
        return 0;
    }

    obj = tinyui_line_edit_create(parent);
    if (obj == 0) {
        return 0;
    }
    line_edit = (struct tinyui_line_edit *)(void *)obj;

    if ((props->fields & TINYUI_LINE_EDIT_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_USER_DATA) != 0) {
        if (tinyui_runtime_internal_widget_set_user_data(&line_edit->widget, props->user_data) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_STYLE_CLASS) != 0) {
        if (tinyui_runtime_internal_widget_set_style_class(&line_edit->widget, props->style_class) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_WIDTH) != 0
        || (props->fields & TINYUI_LINE_EDIT_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&line_edit->widget);
        int h = tinyui_runtime_internal_widget_get_height(&line_edit->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_LINE_EDIT_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_LINE_EDIT_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&line_edit->widget, w, h) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_TEXT) != 0) {
        if (tinyui_line_edit_set_text((tinyui_obj_t *)line_edit, props->text) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    /*
     * Common widget_set_bg/text_color has no LINE_EDIT case (shared-core
     * write surface forbidden here). Map props colors through the dedicated
     * ldLineEditSetColor path so create_with_props does not falsely refuse.
     */
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_BG_COLOR) != 0
        || (props->fields & TINYUI_LINE_EDIT_FIELD_TEXT_COLOR) != 0) {
        unsigned int text_color = line_edit->widget.text_color;
        unsigned int bg_color = line_edit->widget.bg_color;
        unsigned int frame_color = line_edit->widget.border_color;

        if ((props->fields & TINYUI_LINE_EDIT_FIELD_TEXT_COLOR) != 0) {
            text_color = props->text_color;
        }
        if ((props->fields & TINYUI_LINE_EDIT_FIELD_BG_COLOR) != 0) {
            bg_color = props->bg_color;
        }
        if (tinyui_line_edit_set_color((tinyui_obj_t *)line_edit,
                                       text_color,
                                       bg_color,
                                       frame_color) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_BORDER_COLOR) != 0) {
        if (tinyui_runtime_internal_widget_set_border_color(&line_edit->widget, props->border_color) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_RADIUS) != 0) {
        if (tinyui_runtime_internal_widget_set_radius(&line_edit->widget, props->radius) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_PADDING) != 0) {
        if (tinyui_runtime_internal_widget_set_padding(&line_edit->widget, props->padding) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_TYPE) != 0) {
        if (tinyui_line_edit_set_type((tinyui_obj_t *)line_edit, props->type) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }
    if ((props->fields & TINYUI_LINE_EDIT_FIELD_KEYBOARD_BINDING) != 0) {
        if (tinyui_line_edit_set_keyboard_binding((tinyui_obj_t *)line_edit,
                                                  props->keyboard_binding) != 0) {
            tinyui_line_edit_rollback(line_edit);
            return 0;
        }
    }

    return obj;
}


int tinyui_line_edit_set_text(tinyui_obj_t *line_edit_obj, const char *text)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit(line_edit_obj);
    ldLineEdit_t *ld_line_edit;
    size_t len;
    const char *old_text;
    const uint8_t *backend_text;

    if (line_edit == 0 || text == 0 || line_edit->widget.ld_widget == 0) {
        return -1;
    }

    ld_line_edit = (ldLineEdit_t *)line_edit->widget.ld_widget;
    len = strlen(text);

    /*
     * Common widget_set_backend_text has no LINE_EDIT case (shared-core is
     * out of this task write surface). Map directly to ldLineEditSetText.
     * LD copies into its fixed pText buffer when textLen < textMax.
     */
    if (ld_line_edit->textMax != 0 && len >= (size_t)ld_line_edit->textMax) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }

    /* Programmatic set_text must not fabricate user events. */
    ldLineEditSetText(ld_line_edit, (uint8_t *)(uintptr_t)text);
    backend_text = ldLineEditGetText(ld_line_edit);
    if (backend_text == 0 || strcmp((const char *)backend_text, text) != 0) {
        return -1;
    }

    old_text = line_edit->widget.text;
    if (line_edit->widget.text_owned && old_text != 0
        && old_text != (const char *)backend_text) {
        ldFree((void *)(uintptr_t)old_text);
    }
    line_edit->widget.text = (const char *)backend_text;
    line_edit->widget.text_owned = 0;
    return 0;
}

int tinyui_line_edit_set_align(tinyui_obj_t *line_edit_obj, enum tinyui_align align)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit(line_edit_obj);

    if (line_edit == 0 || line_edit->widget.ld_widget == 0) {
        return -1;
    }

    ldLineEditSetAlign((ldLineEdit_t *)line_edit->widget.ld_widget,
                       (arm_2d_align_t)tinyui_align_to_arm2d(align));
    line_edit->align = align;
    return 0;
}

int tinyui_line_edit_set_color(tinyui_obj_t *line_edit_obj,
                               unsigned int text_color,
                               unsigned int background_color,
                               unsigned int frame_color)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit(line_edit_obj);

    if (line_edit == 0 || line_edit->widget.ld_widget == 0) {
        return -1;
    }

    ldLineEditSetColor((ldLineEdit_t *)line_edit->widget.ld_widget,
                       (ldColor)tinyui_rgb_to_ld_color(text_color),
                       (ldColor)tinyui_rgb_to_ld_color(background_color),
                       (ldColor)tinyui_rgb_to_ld_color(frame_color));
    line_edit->widget.text_color = text_color;
    line_edit->widget.bg_color = background_color;
    line_edit->widget.border_color = frame_color;
    return 0;
}

const char *tinyui_line_edit_get_text(const tinyui_obj_t *line_edit_obj)
{
    const struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit_const(line_edit_obj);
    const uint8_t *backend_text;

    if (line_edit == 0) {
        return 0;
    }

    if (line_edit->widget.ld_widget != 0) {
        backend_text = ldLineEditGetText((ldLineEdit_t *)line_edit->widget.ld_widget);
        if (backend_text != 0) {
            return (const char *)backend_text;
        }
    }

    return line_edit->widget.text;
}

int tinyui_line_edit_set_type(tinyui_obj_t *line_edit_obj, enum tinyui_line_edit_type type)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit(line_edit_obj);

    if (line_edit == 0 || !tinyui_line_edit_type_is_valid(type)
        || line_edit->widget.ld_widget == 0) {
        return -1;
    }

    ldLineEditSetType((ldLineEdit_t *)line_edit->widget.ld_widget, (ldEditType_t)type);
    line_edit->type = type;
    return 0;
}

int tinyui_line_edit_get_type(const tinyui_obj_t *line_edit_obj, enum tinyui_line_edit_type *type)
{
    const struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit_const(line_edit_obj);

    if (line_edit == 0 || type == 0 || line_edit->widget.ld_widget == 0) {
        return -1;
    }

    *type = (enum tinyui_line_edit_type)
        ((ldLineEdit_t *)line_edit->widget.ld_widget)->editType;
    return 0;
}

int tinyui_line_edit_set_keyboard(tinyui_obj_t *line_edit_obj, unsigned int keyboard_binding)
{
    return tinyui_line_edit_set_keyboard_binding(line_edit_obj, keyboard_binding);
}

int tinyui_line_edit_set_keyboard_binding(tinyui_obj_t *line_edit_obj, unsigned int keyboard_binding)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit(line_edit_obj);

    if (line_edit == 0 || !tinyui_line_edit_keyboard_binding_is_valid(keyboard_binding)
        || line_edit->widget.ld_widget == 0) {
        return -1;
    }

    ldLineEditSetKeyboard((ldLineEdit_t *)line_edit->widget.ld_widget,
                          (uint16_t)keyboard_binding);
    line_edit->keyboard_binding = keyboard_binding;
    return 0;
}

int tinyui_line_edit_set_keyboard_widget(tinyui_obj_t *line_edit_obj, tinyui_obj_t *keyboard)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit(line_edit_obj);
    struct tinyui_widget *kb;

    if (line_edit == 0) {
        return -1;
    }

    kb = (struct tinyui_widget *)(void *)keyboard;
    if (kb == 0 ||
        line_edit->widget.owner == 0 ||
        kb->owner != line_edit->widget.owner ||
        kb->ld_widget == 0 ||
        kb->kind != TINYUI_BACKEND_WIDGET_KEYBOARD) {
        return -1;
    }

    return tinyui_line_edit_set_keyboard_binding((tinyui_obj_t *)line_edit, kb->ld_name_id);
}

int tinyui_line_edit_get_keyboard_binding(const tinyui_obj_t *line_edit_obj,
                                          unsigned int *keyboard_binding)
{
    const struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit_const(line_edit_obj);

    if (line_edit == 0 || keyboard_binding == 0 || line_edit->widget.ld_widget == 0) {
        return -1;
    }

    *keyboard_binding = (unsigned int)
        ((ldLineEdit_t *)line_edit->widget.ld_widget)->kbNameId;
    return 0;
}

int tinyui_line_edit_get_editing(const tinyui_obj_t *line_edit_obj, int *editing)
{
    const struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit_const(line_edit_obj);

    if (line_edit == 0 || editing == 0 || line_edit->widget.ld_widget == 0) {
        return -1;
    }

    *editing = ((ldLineEdit_t *)line_edit->widget.ld_widget)->isEditing ? 1 : 0;
    return 0;
}

int tinyui_line_edit_set_on_edit_finished(tinyui_obj_t *line_edit_obj,
                                          tinyui_line_edit_finished_cb cb,
                                          void *user_data)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_as_line_edit(line_edit_obj);

    if (line_edit == 0) {
        return -1;
    }

    line_edit->on_edit_finished = cb;
    line_edit->on_edit_finished_user_data = user_data;
    return 0;
}
