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
#include "ldKeyboard.h"
#include "ldLineEdit.h"

#include <stdlib.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
void ldKeyboardInputAscii(ldKeyboard_t *ptWidget, uint8_t ascii);

static ldColor picoui_backend_keyboard_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void picoui_backend_keyboard_invoke_event(struct picoui_backend_widget *backend,
                                                 unsigned int key_code,
                                                 enum picoui_native_signal signal)
{
    struct picoui_keyboard *keyboard;

    if (backend == NULL || backend->host_widget == NULL) {
        return;
    }

    keyboard = (struct picoui_keyboard *)backend->host_widget;
    if (keyboard->event_cb == NULL) {
        return;
    }

    keyboard->event_cb(keyboard, key_code, signal, keyboard->event_user_data);
}

static void picoui_backend_keyboard_invoke_draw(struct picoui_keyboard *keyboard,
                                                const struct picoui_keyboard_button *button)
{
    if (keyboard == NULL || button == NULL || keyboard->draw_cb == NULL) {
        return;
    }

    keyboard->draw_invocation_count++;
    keyboard->last_draw_key_code = button->key_code;
    keyboard->draw_cb(keyboard, button, keyboard->draw_user_data);
}

static const kbBtnInfo_t *picoui_backend_keyboard_get_custom_button_list(struct picoui_backend_widget *backend)
{
    struct picoui_keyboard *keyboard;
    kbBtnInfo_t *native_buttons;
    int i;

    if (backend == NULL || backend->host_widget == NULL) {
        return NULL;
    }

    keyboard = (struct picoui_keyboard *)backend->host_widget;
    if (keyboard->layout_entries == NULL || keyboard->layout_count <= 0) {
        return NULL;
    }

    if (keyboard->native_layout != NULL &&
        keyboard->native_layout != (void *)keyboard->layout_entries) {
        return (const kbBtnInfo_t *)keyboard->native_layout;
    }

    native_buttons = calloc((size_t)keyboard->layout_count + 1U, sizeof(*native_buttons));
    if (native_buttons == NULL) {
        return NULL;
    }

    for (i = 0; i < keyboard->layout_count; ++i) {
        struct picoui_keyboard_button button_info;

        native_buttons[i].region.tLocation.iX = (int16_t)keyboard->layout_entries[i].x;
        native_buttons[i].region.tLocation.iY = (int16_t)keyboard->layout_entries[i].y;
        native_buttons[i].region.tSize.iWidth = (int16_t)keyboard->layout_entries[i].width;
        native_buttons[i].region.tSize.iHeight = (int16_t)keyboard->layout_entries[i].height;
        native_buttons[i].pText = (uint8_t *)keyboard->layout_entries[i].text;
        native_buttons[i].keyCode = (uint8_t)keyboard->layout_entries[i].key_code;
        native_buttons[i].pressColor = picoui_backend_keyboard_rgb_to_ld_color(
            keyboard->layout_entries[i].press_color);
        native_buttons[i].releaseColor = picoui_backend_keyboard_rgb_to_ld_color(
            keyboard->layout_entries[i].release_color);

        button_info.x = keyboard->layout_entries[i].x;
        button_info.y = keyboard->layout_entries[i].y;
        button_info.width = keyboard->layout_entries[i].width;
        button_info.height = keyboard->layout_entries[i].height;
        button_info.text = keyboard->layout_entries[i].text;
        button_info.key_code = keyboard->layout_entries[i].key_code;
        button_info.press_color = keyboard->layout_entries[i].press_color;
        button_info.release_color = keyboard->layout_entries[i].release_color;
        picoui_backend_keyboard_invoke_draw(keyboard, &button_info);
    }

    keyboard->native_layout = native_buttons;
    return native_buttons;
}

static struct picoui_backend_app_state *picoui_backend_keyboard_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldKeyboard_t *picoui_backend_keyboard_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldKeyboard_t *)widget->ld_widget;
}

static void picoui_backend_keyboard_prepare(ldKeyboard_t *ld_keyboard,
                                            struct picoui_line_edit *line_edit)
{
    struct picoui_backend_widget *backend;
    const kbBtnInfo_t *custom_buttons;

    if (ld_keyboard == NULL) {
        return;
    }

    if (line_edit != NULL) {
        ld_keyboard->editType = (ldEditType_t)line_edit->type;
    }
    backend = (struct picoui_backend_widget *)((ldBase_t *)ld_keyboard)->pInfo;
    custom_buttons = picoui_backend_keyboard_get_custom_button_list(backend);
    if (ld_keyboard->pBtnList == NULL || ld_keyboard->isWaitInit) {
        ld_keyboard->pBtnList = custom_buttons != NULL
                              ? custom_buttons
                              : ldKeyboardGetTargetBtnList(ld_keyboard);
        ld_keyboard->isWaitInit = false;
    } else if (custom_buttons != NULL) {
        ld_keyboard->pBtnList = custom_buttons;
    } else {
        ld_keyboard->pBtnList = ldKeyboardGetTargetBtnList(ld_keyboard);
    }
    ldBaseSetHidden((ldBase_t *)ld_keyboard, false);
}

static struct picoui_line_edit *picoui_backend_keyboard_get_target_line_edit(struct picoui_backend_widget *backend)
{
    struct picoui_app *app;
    struct picoui_widget *target;
    struct picoui_backend_widget *target_backend;
    ldBase_t *ld_base;

    if (backend == NULL || backend->owner == NULL) {
        return NULL;
    }

    app = backend->owner;
    target = app->editing_owner != NULL ? app->editing_owner : app->focus_owner;
    if (target == NULL || target->backend_widget == NULL) {
        return NULL;
    }

    target_backend = (struct picoui_backend_widget *)target->backend_widget;
    if (target_backend->kind != PICOUI_BACKEND_WIDGET_TEXT || target_backend->ld_widget == NULL) {
        return NULL;
    }

    ld_base = (ldBase_t *)target_backend->ld_widget;
    if (ld_base->widgetType != widgetTypeLineEdit) {
        return NULL;
    }
    return (struct picoui_line_edit *)target;
}

/**
 * @brief Create backend for keyboard
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_keyboard(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldKeyboard_t *ld_keyboard;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_keyboard_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_keyboard = ldKeyboard_init(app_state->ld_scene,
                                  NULL,
                                  name_id,
                                  parent_widget->ld_name_id,
                                  (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_keyboard == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_KEYBOARD;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_keyboard;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief keyboard: input ascii
 *
 * @param[in] backend_widget backend widget
 * @param[in] ascii ascii
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_input_ascii(void *backend_widget, unsigned int ascii)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_line_edit *line_edit;
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit;

    if (backend == NULL) {
        return -1;
    }

    line_edit = picoui_backend_keyboard_get_target_line_edit(backend);
    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (line_edit == NULL || ld_keyboard == NULL) {
        return -1;
    }
    ld_line_edit = (ldLineEdit_t *)((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_widget;
    if (ld_line_edit == NULL) {
        return -1;
    }

    picoui_backend_keyboard_prepare(ld_keyboard, line_edit);
    ld_keyboard->ppStr = &ld_line_edit->pText;
    ld_keyboard->strMax = ld_line_edit->textMax;
    ld_keyboard->editorId = ((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_name_id;
    ldKeyboardInputAscii(ld_keyboard, (uint8_t)ascii);
    line_edit->widget.text = picoui_backend_line_edit_get_text(line_edit->widget.backend_widget);
    return 0;
}

/**
 * @brief keyboard: navigate
 *
 * @param[in] backend_widget backend widget
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_navigate(void *backend_widget, int direction)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldKeyboard_t *ld_keyboard;

    if (backend == NULL || backend->host_widget == NULL || !picoui_widget_is_focus_owner(backend->host_widget)) {
        return -1;
    }

    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (ld_keyboard == NULL) {
        return -1;
    }

    picoui_backend_keyboard_prepare(ld_keyboard, picoui_backend_keyboard_get_target_line_edit(backend));
    ldKeyboardNavigate(ld_keyboard, (ldNavDir_t)direction);
    return 0;
}

/**
 * @brief keyboard: update
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_update(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldKeyboard_t *ld_keyboard;

    if (backend == NULL) {
        return -1;
    }

    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (ld_keyboard == NULL) {
        return -1;
    }

    picoui_backend_keyboard_prepare(ld_keyboard, picoui_backend_keyboard_get_target_line_edit(backend));
    ldKeyboardUpdate(ld_keyboard);
    return 0;
}

/**
 * @brief keyboard: button update
 *
 * @param[in] backend_widget backend widget
 * @param[in] key_code key code
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_button_update(void *backend_widget, unsigned char key_code)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldKeyboard_t *ld_keyboard;

    if (backend == NULL) {
        return -1;
    }

    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (ld_keyboard == NULL) {
        return -1;
    }

    picoui_backend_keyboard_prepare(ld_keyboard, picoui_backend_keyboard_get_target_line_edit(backend));
    ld_keyboard->keyCode = key_code;
    ld_keyboard->isKeySelect = true;
    ldKeyboardBtnUpdate(ld_keyboard, key_code);
    picoui_backend_keyboard_invoke_event(backend, key_code, PICOUI_NATIVE_SIGNAL_VALUE_CHANGED);
    return 0;
}

/**
 * @brief keyboard: click
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_click(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_backend_app_state *app_state;
    ldKeyboard_t *ld_keyboard;

    if (backend == NULL || backend->host_widget == NULL || !picoui_widget_is_focus_owner(backend->host_widget)) {
        return -1;
    }

    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (app_state == NULL || app_state->ld_scene == NULL || ld_keyboard == NULL) {
        return -1;
    }

    picoui_backend_keyboard_prepare(ld_keyboard, picoui_backend_keyboard_get_target_line_edit(backend));
    ldKeyboardClick(app_state->ld_scene, ld_keyboard, SIGNAL_PRESS);
    picoui_backend_keyboard_invoke_event(backend, ld_keyboard->keyCode, PICOUI_NATIVE_SIGNAL_PRESS);
    return 0;
}

/**
 * @brief keyboard: exit
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_exit(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_line_edit *line_edit;
    struct picoui_widget *editing_owner_widget = NULL;
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit = NULL;

    if (backend == NULL) {
        return -1;
    }

    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (ld_keyboard == NULL) {
        return -1;
    }

    if (backend->owner != NULL) {
        editing_owner_widget = backend->owner->editing_owner;
    }
    line_edit = picoui_backend_keyboard_get_target_line_edit(backend);
    ldKeyboardExit(ld_keyboard);
    if (line_edit != NULL) {
        ld_line_edit = (ldLineEdit_t *)((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_widget;
        if (line_edit->widget.text != NULL) {
            (void)picoui_backend_line_edit_set_text(line_edit->widget.backend_widget,
                                                    line_edit->widget.text);
        }
        line_edit->editing = 0;
        if (ld_line_edit != NULL) {
            ld_line_edit->isEditing = false;
            ((ldBase_t *)ld_line_edit)->isDirtyRegionUpdate = true;
        }
        ((struct picoui_backend_widget *)line_edit->widget.backend_widget)->edit_result_on_finish =
            PICOUI_EDIT_RESULT_CANCEL;
        (void)picoui_widget_mark_edit_result(&line_edit->widget, PICOUI_EDIT_RESULT_CANCEL);
        (void)picoui_widget_release_editing(&line_edit->widget);
    } else if (editing_owner_widget != NULL) {
        if (picoui_widget_get_type(editing_owner_widget) == PICOUI_WIDGET_TYPE_TABLE) {
            struct picoui_table *table = (struct picoui_table *)editing_owner_widget;
            int row = -1;
            int column = -1;
            const char *model_text = NULL;

            if (picoui_backend_table_sync_current_cell(table, &row, &column) == 0) {
                struct picoui_table_ext {
                    struct picoui_table table;
                    const char *cell_texts[];
                };
                struct picoui_table_ext *ext = (struct picoui_table_ext *)table;

                if (row >= 0 && column >= 0 && row < table->row_count && column < table->column_count) {
                    model_text = ext->cell_texts[(row * table->column_count) + column];
                }
                if (model_text != NULL) {
                    (void)picoui_backend_table_set_cell_text(editing_owner_widget->backend_widget,
                                                             row,
                                                             column,
                                                             model_text);
                }
            }
        }
        (void)picoui_widget_mark_edit_result(editing_owner_widget, PICOUI_EDIT_RESULT_CANCEL);
        (void)picoui_widget_release_editing(editing_owner_widget);
    }
    if (backend->host_widget != NULL && picoui_widget_is_focus_owner(backend->host_widget)) {
        return picoui_widget_release_focus(backend->host_widget);
    }
    return 0;
}
