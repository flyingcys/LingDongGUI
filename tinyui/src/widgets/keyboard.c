/*
 * Copyright (c) 2023-2026 flyingcys (59935554@qq.com). All rights reserved.
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
#include "widgets/keyboard.h"
#include "../core/runtime_bridge.h"
#include "ldLineEdit.h"
#include "ldKeyboard.h"

#include <string.h>

/* Soft capacity for custom button tables. Public API returns -1 beyond this. */
enum {
    TINYUI_KEYBOARD_LAYOUT_MAX = 64
};

static struct tinyui_keyboard *tinyui_keyboard_as_keyboard(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_KEYBOARD)) {
        return 0;
    }
    return (struct tinyui_keyboard *)w;
}

static const struct tinyui_keyboard *tinyui_keyboard_as_keyboard_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_KEYBOARD)) {
        return 0;
    }
    return (const struct tinyui_keyboard *)w;
}

void ldKeyboardInputAscii(ldKeyboard_t *ptWidget, uint8_t ascii);

struct tinyui_keyboard_create_ctx {
    arm_2d_font_t *font;
};

/* ── C2 depose machinery ─────────────────────────────────────────────────── */



/* Free dynamic layout resources BEFORE tinyui_runtime_internal_widget_destroy_common frees
 * the host struct.  Also called by tinyui_keyboard_set_buttons during reuse. */
static void tinyui_keyboard_free_layout(struct tinyui_keyboard *keyboard);

static void tinyui_keyboard_host_cleanup(struct tinyui_widget *w)
{
    tinyui_keyboard_free_layout((struct tinyui_keyboard *)w);
}

static void tinyui_keyboard_free_layout(struct tinyui_keyboard *keyboard)
{
    int i;
    void *native_layout;

    if (keyboard == 0) {
        return;
    }

    native_layout = keyboard->native_layout;

    if (keyboard->layout_entries == 0) {
        if (native_layout != 0) {
            ldFree(native_layout);
        }
        keyboard->native_layout = 0;
        keyboard->layout_count = 0;
        return;
    }

    for (i = 0; i < keyboard->layout_count; ++i) {
        ldFree(keyboard->layout_entries[i].text);
    }
    ldFree(keyboard->layout_entries);
    if (native_layout != 0 && native_layout != (void *)keyboard->layout_entries) {
        ldFree(native_layout);
    }
    keyboard->layout_entries = 0;
    keyboard->native_layout = 0;
    keyboard->layout_count = 0;
}

static void tinyui_keyboard_rollback(struct tinyui_keyboard *keyboard)
{
    if (keyboard == 0) {
        return;
    }
    if (((struct tinyui_widget *)(void *)keyboard)->ld_widget != 0) {
        tinyui_keyboard_free_layout(keyboard);
        tinyui_runtime_internal_widget_destroy_common(&keyboard->widget);
    } else {
        ldFree(keyboard);
    }
}

static int tinyui_keyboard_props_are_valid(const tinyui_keyboard_props_t *props)
{
    return props != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_runtime_internal_keyboard_ld_init(void *ctx,
                                     struct ld_scene_t *scene,
                                     uint16_t name_id,
                                     uint16_t parent_name_id)
{
    struct tinyui_keyboard_create_ctx *create_ctx =
        (struct tinyui_keyboard_create_ctx *)ctx;

    if (create_ctx == 0 || scene == 0 || create_ctx->font == 0) {
        return 0;
    }

    return ldKeyboard_init(scene,
                           NULL,
                           name_id,
                           parent_name_id,
                           create_ctx->font);
}

static int tinyui_keyboard_get_selected_key_code_internal(const struct tinyui_keyboard *keyboard,
                                                          unsigned int *key_code)
{
    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0 || key_code == 0 ||
        ((struct tinyui_widget *)(void *)keyboard)->kind != TINYUI_BACKEND_WIDGET_KEYBOARD ||
        ((struct tinyui_widget *)(void *)keyboard)->ld_widget == 0) {
        return -1;
    }

    ld_keyboard = (ldKeyboard_t *)((struct tinyui_widget *)(void *)keyboard)->ld_widget;
    if (ld_keyboard == 0) {
        return -1;
    }

    *key_code = ld_keyboard->keyCode;
    return 0;
}

static struct tinyui_line_edit *tinyui_keyboard_get_target_line_edit_local(
    struct tinyui_widget *keyboard_widget)
{
    struct tinyui_app *app;
    struct tinyui_widget *target;
    ldBase_t *ld_base;

    if (keyboard_widget == 0 || keyboard_widget->owner == 0) {
        return 0;
    }

    app = keyboard_widget->owner;
    target = app->editing_owner != 0 ? app->editing_owner : app->focus_owner;
    if (target == 0 || target->ld_widget == 0) {
        return 0;
    }

    /* line_edit owns TINYUI_BACKEND_WIDGET_LINE_EDIT (not TEXT). */
    if (target->kind != TINYUI_BACKEND_WIDGET_LINE_EDIT) {
        return 0;
    }

    ld_base = (ldBase_t *)target->ld_widget;
    if (ld_base->widgetType != widgetTypeLineEdit) {
        return 0;
    }
    return (struct tinyui_line_edit *)target;
}

static const kbBtnInfo_t *tinyui_keyboard_get_custom_button_list_from_host(struct tinyui_keyboard *keyboard)
{
    kbBtnInfo_t *native_buttons;
    int i;

    if (keyboard == 0) {
        return 0;
    }
    if (keyboard->layout_entries == 0 || keyboard->layout_count <= 0) {
        return 0;
    }

    if (keyboard->native_layout != 0 &&
        keyboard->native_layout != (void *)keyboard->layout_entries) {
        return (const kbBtnInfo_t *)keyboard->native_layout;
    }

    native_buttons = ldCalloc((size_t)keyboard->layout_count + 1U, sizeof(*native_buttons));
    if (native_buttons == 0) {
        return 0;
    }

    for (i = 0; i < keyboard->layout_count; ++i) {
        struct tinyui_keyboard_button button_info;

        native_buttons[i].region.tLocation.iX = (int16_t)keyboard->layout_entries[i].x;
        native_buttons[i].region.tLocation.iY = (int16_t)keyboard->layout_entries[i].y;
        native_buttons[i].region.tSize.iWidth = (int16_t)keyboard->layout_entries[i].width;
        native_buttons[i].region.tSize.iHeight = (int16_t)keyboard->layout_entries[i].height;
        native_buttons[i].pText = (uint8_t *)keyboard->layout_entries[i].text;
        native_buttons[i].keyCode = (uint8_t)keyboard->layout_entries[i].key_code;
        native_buttons[i].pressColor = __RGB((keyboard->layout_entries[i].press_color >> 16) & 0xFFU,
                                             (keyboard->layout_entries[i].press_color >> 8) & 0xFFU,
                                             keyboard->layout_entries[i].press_color & 0xFFU);
        native_buttons[i].releaseColor = __RGB((keyboard->layout_entries[i].release_color >> 16) & 0xFFU,
                                               (keyboard->layout_entries[i].release_color >> 8) & 0xFFU,
                                               keyboard->layout_entries[i].release_color & 0xFFU);

        button_info.x = keyboard->layout_entries[i].x;
        button_info.y = keyboard->layout_entries[i].y;
        button_info.width = keyboard->layout_entries[i].width;
        button_info.height = keyboard->layout_entries[i].height;
        button_info.text = keyboard->layout_entries[i].text;
        button_info.key_code = keyboard->layout_entries[i].key_code;
        button_info.press_color = keyboard->layout_entries[i].press_color;
        button_info.release_color = keyboard->layout_entries[i].release_color;
        if (keyboard->draw_cb != 0) {
            keyboard->draw_invocation_count++;
            keyboard->last_draw_key_code = button_info.key_code;
            keyboard->draw_cb(keyboard, &button_info, keyboard->draw_user_data);
        }
    }

    keyboard->native_layout = native_buttons;
    return native_buttons;
}

static void tinyui_keyboard_prepare_local(ldKeyboard_t *ld_keyboard,
                                          struct tinyui_line_edit *line_edit,
                                          struct tinyui_keyboard *keyboard_host)
{
    const kbBtnInfo_t *custom_buttons;

    if (ld_keyboard == 0) {
        return;
    }

    if (line_edit != 0) {
        ld_keyboard->editType = (ldEditType_t)line_edit->type;
    }
    custom_buttons = tinyui_keyboard_get_custom_button_list_from_host(keyboard_host);
    if (ld_keyboard->pBtnList == 0 || ld_keyboard->isWaitInit) {
        ld_keyboard->pBtnList = custom_buttons != 0
                              ? custom_buttons
                              : ldKeyboardGetTargetBtnList(ld_keyboard);
        ld_keyboard->isWaitInit = false;
    } else if (custom_buttons != 0) {
        ld_keyboard->pBtnList = custom_buttons;
    } else {
        ld_keyboard->pBtnList = ldKeyboardGetTargetBtnList(ld_keyboard);
    }
    ldBaseSetHidden((ldBase_t *)ld_keyboard, false);
}

/**
 * @brief Create keyboard widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_keyboard_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "keyboard";
    if (parent_w == 0) { return 0; }

    struct tinyui_keyboard_create_ctx create_ctx;
    struct tinyui_keyboard *keyboard;

    if (parent_w == 0 || id == 0) {
        return 0;
    }

    if (parent_w->ld_widget == 0 || parent_w->owner == 0) {
        return 0;
    }

    create_ctx.font = tinyui_resolve_ld_font(0, 12);
    if (create_ctx.font == 0) {
        return 0;
    }
    keyboard = (struct tinyui_keyboard *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                                   TINYUI_BACKEND_WIDGET_KEYBOARD,
                                                                   tinyui_runtime_internal_keyboard_ld_init,
                                                                   &create_ctx,
                                                                   sizeof(*keyboard));
    if (keyboard == 0) {
        return 0;
    }

    keyboard->id = id;
    keyboard->widget.visible    = 1;
    keyboard->widget.enabled    = 1;
    keyboard->widget.host_cleanup = tinyui_keyboard_host_cleanup;

    return (tinyui_obj_t *)keyboard;
}

/**
 * @brief Create keyboard widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_keyboard_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_keyboard_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_keyboard *keyboard;

    if (props == 0) {
        return tinyui_keyboard_create(parent);
    }

    obj = tinyui_keyboard_create(parent);
    if (obj == 0) {
        return 0;
    }
    keyboard = (struct tinyui_keyboard *)(void *)obj;

    if ((props->fields & TINYUI_KEYBOARD_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_KEYBOARD_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&keyboard->widget, props->user_data) != 0) {
        tinyui_keyboard_rollback(keyboard);
        return 0;
    }
    }
    if ((props->fields & TINYUI_KEYBOARD_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&keyboard->widget, props->style_class) != 0) {
        tinyui_keyboard_rollback(keyboard);
        return 0;
    }
    }
        if ((props->fields & TINYUI_KEYBOARD_FIELD_WIDTH) != 0 || (props->fields & TINYUI_KEYBOARD_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&keyboard->widget);
        int h = tinyui_runtime_internal_widget_get_height(&keyboard->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_KEYBOARD_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_KEYBOARD_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&keyboard->widget, w, h) != 0) {
            tinyui_keyboard_rollback(keyboard);
            return 0;
        }
    }
    if ((props->fields & TINYUI_KEYBOARD_FIELD_BG_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_bg_color(&keyboard->widget, props->bg_color) != 0) {
        tinyui_keyboard_rollback(keyboard);
        return 0;
    }
    }
    if ((props->fields & TINYUI_KEYBOARD_FIELD_TEXT_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_text_color(&keyboard->widget, props->text_color) != 0) {
        tinyui_keyboard_rollback(keyboard);
        return 0;
    }
    }
    if ((props->fields & TINYUI_KEYBOARD_FIELD_BORDER_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_border_color(&keyboard->widget, props->border_color) != 0) {
        tinyui_keyboard_rollback(keyboard);
        return 0;
    }
    }
    if ((props->fields & TINYUI_KEYBOARD_FIELD_RADIUS) != 0) {
    if (tinyui_runtime_internal_widget_set_radius(&keyboard->widget, props->radius) != 0) {
        tinyui_keyboard_rollback(keyboard);
        return 0;
    }
    }
    if ((props->fields & TINYUI_KEYBOARD_FIELD_PADDING) != 0) {
    if (tinyui_runtime_internal_widget_set_padding(&keyboard->widget, props->padding) != 0) {
        tinyui_keyboard_rollback(keyboard);
        return 0;
    }
    }

    return obj;
}


/**
 * @brief keyboard input ascii
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] ascii ascii
 * @return -1 on failure
 */

int tinyui_keyboard_input_ascii(tinyui_obj_t *keyboard_obj, unsigned int ascii)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    struct tinyui_line_edit *line_edit;
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit;

    if (keyboard == 0 ||
        ((struct tinyui_widget *)(void *)keyboard)->kind != TINYUI_BACKEND_WIDGET_KEYBOARD) {
        return -1;
    }

    line_edit = tinyui_keyboard_get_target_line_edit_local(&keyboard->widget);
    ld_keyboard = (ldKeyboard_t *)((struct tinyui_widget *)(void *)keyboard)->ld_widget;
    if (line_edit == 0 || ld_keyboard == 0) {
        return -1;
    }
    ld_line_edit = (ldLineEdit_t *)line_edit->widget.ld_widget;
    if (ld_line_edit == 0) {
        return -1;
    }

    tinyui_keyboard_prepare_local(ld_keyboard, line_edit, keyboard);
    ld_keyboard->ppStr = &ld_line_edit->pText;
    ld_keyboard->strMax = ld_line_edit->textMax;
    ld_keyboard->editorId = line_edit->widget.ld_name_id;
    ldKeyboardInputAscii(ld_keyboard, (uint8_t)ascii);
    line_edit->widget.text = tinyui_line_edit_get_text(line_edit);
    return 0;
}

/**
 * @brief keyboard navigate
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] direction direction
 * @return -1 on failure
 */

int tinyui_keyboard_navigate(tinyui_obj_t *keyboard_obj, int direction)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0 ||
        ((struct tinyui_widget *)(void *)keyboard)->kind != TINYUI_BACKEND_WIDGET_KEYBOARD) {
        return -1;
    }

    if (!tinyui_runtime_internal_widget_is_focus_owner(&keyboard->widget)) {
        return -1;
    }

    if (direction < (int)TINYUI_NAV_LEFT || direction > (int)TINYUI_NAV_BACK) {
        return -1;
    }

    ld_keyboard = (ldKeyboard_t *)((struct tinyui_widget *)(void *)keyboard)->ld_widget;
    if (ld_keyboard == 0) {
        return -1;
    }

    tinyui_keyboard_prepare_local(ld_keyboard, tinyui_keyboard_get_target_line_edit_local(&keyboard->widget), keyboard);
    ldKeyboardNavigate(ld_keyboard,
                       (ldNavDir_t)tinyui_native_nav_dir_to_ld((enum tinyui_native_nav_dir)direction));
    return 0;
}

/**
 * @brief keyboard update
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int tinyui_keyboard_update(tinyui_obj_t *keyboard_obj)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0 ||
        ((struct tinyui_widget *)(void *)keyboard)->kind != TINYUI_BACKEND_WIDGET_KEYBOARD) {
        return -1;
    }

    ld_keyboard = (ldKeyboard_t *)((struct tinyui_widget *)(void *)keyboard)->ld_widget;
    if (ld_keyboard == 0) {
        return -1;
    }

    tinyui_keyboard_prepare_local(ld_keyboard, tinyui_keyboard_get_target_line_edit_local(&keyboard->widget), keyboard);
    ldKeyboardUpdate(ld_keyboard);
    return 0;
}

/**
 * @brief keyboard button update
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] key_code key code
 * @return -1 on failure
 */

int tinyui_keyboard_button_update(tinyui_obj_t *keyboard_obj, unsigned int key_code)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0 || key_code > 0xFFU ||
        ((struct tinyui_widget *)(void *)keyboard)->kind != TINYUI_BACKEND_WIDGET_KEYBOARD) {
        return -1;
    }

    ld_keyboard = (ldKeyboard_t *)((struct tinyui_widget *)(void *)keyboard)->ld_widget;
    if (ld_keyboard == 0) {
        return -1;
    }

    tinyui_keyboard_prepare_local(ld_keyboard, tinyui_keyboard_get_target_line_edit_local(&keyboard->widget), keyboard);
    ld_keyboard->keyCode = (uint8_t)key_code;
    ld_keyboard->isKeySelect = true;
    ldKeyboardBtnUpdate(ld_keyboard, (uint8_t)key_code);
    if (keyboard->event_cb != 0) {
        keyboard->event_cb((tinyui_obj_t *)keyboard,
                           (unsigned int)key_code,
                           TINYUI_SIGNAL_VALUE_CHANGED,
                           keyboard->event_user_data);
    }
    return 0;
}

/**
 * @brief keyboard click
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int tinyui_keyboard_click(tinyui_obj_t *keyboard_obj)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    struct tinyui_app *app_state;
    ldKeyboard_t *ld_keyboard;

    if (keyboard == 0 ||
        ((struct tinyui_widget *)(void *)keyboard)->kind != TINYUI_BACKEND_WIDGET_KEYBOARD) {
        return -1;
    }

    if (!tinyui_runtime_internal_widget_is_focus_owner(&keyboard->widget)) {
        return -1;
    }

    app_state = ((struct tinyui_widget *)(void *)keyboard)->owner;
    ld_keyboard = (ldKeyboard_t *)((struct tinyui_widget *)(void *)keyboard)->ld_widget;
    if (app_state == 0 || app_state->ld_scene == 0 || ld_keyboard == 0) {
        return -1;
    }

    ldKeyboardClick(app_state->ld_scene, ld_keyboard, SIGNAL_PRESS);
    if (keyboard->event_cb != 0) {
        keyboard->event_cb((tinyui_obj_t *)keyboard,
                           ld_keyboard->keyCode,
                           TINYUI_SIGNAL_PRESS,
                           keyboard->event_user_data);
    }
    return 0;
}

/**
 * @brief keyboard exit
 *
 * @param[in] keyboard Keyboard widget instance
 * @return -1 on failure
 */

int tinyui_keyboard_exit(tinyui_obj_t *keyboard_obj)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    struct tinyui_line_edit *line_edit;
    struct tinyui_widget *editing_owner_widget = 0;
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit = 0;

    if (keyboard == 0 ||
        ((struct tinyui_widget *)(void *)keyboard)->kind != TINYUI_BACKEND_WIDGET_KEYBOARD ||
        ((struct tinyui_widget *)(void *)keyboard)->ld_widget == 0) {
        return -1;
    }

    ld_keyboard = (ldKeyboard_t *)((struct tinyui_widget *)(void *)keyboard)->ld_widget;
    if (ld_keyboard == 0) {
        return -1;
    }

    if (((struct tinyui_widget *)(void *)keyboard)->owner != 0) {
        editing_owner_widget = ((struct tinyui_widget *)(void *)keyboard)->owner->editing_owner;
    }

    line_edit = tinyui_keyboard_get_target_line_edit_local(&keyboard->widget);
    ldKeyboardExit(ld_keyboard);
    if (line_edit != 0) {
        ld_line_edit = (ldLineEdit_t *)line_edit->widget.ld_widget;
        line_edit->editing = 0;
        if (ld_line_edit != 0) {
            ld_line_edit->isEditing = false;
            ((ldBase_t *)ld_line_edit)->isDirtyRegionUpdate = true;
        }
        line_edit->widget.edit_result_on_finish = TINYUI_EDIT_RESULT_CANCEL;
        (void)tinyui_runtime_internal_widget_mark_edit_result(&line_edit->widget, TINYUI_EDIT_RESULT_CANCEL);
        (void)tinyui_runtime_internal_widget_release_editing(&line_edit->widget);
    } else if (editing_owner_widget != 0) {
        (void)tinyui_runtime_internal_widget_mark_edit_result(editing_owner_widget, TINYUI_EDIT_RESULT_CANCEL);
        (void)tinyui_runtime_internal_widget_release_editing(editing_owner_widget);
    }
    if (tinyui_runtime_internal_widget_is_focus_owner(&keyboard->widget)) {
        return tinyui_runtime_internal_widget_release_focus(&keyboard->widget);
    }
    return 0;
}

/**
 * @brief Set buttons of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int tinyui_keyboard_set_buttons(tinyui_obj_t *keyboard_obj, const struct tinyui_keyboard_button *buttons, int count)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    struct tinyui_keyboard_layout_entry *entries;
    int i;

    if (keyboard == 0) {
        return -1;
    }

    if (buttons == 0 || count <= 0) {
        tinyui_keyboard_free_layout(keyboard);
        keyboard->buttons = 0;
        return 0;
    }

    if (count > TINYUI_KEYBOARD_LAYOUT_MAX) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }

    entries = ldCalloc((size_t)count, sizeof(*entries));
    if (entries == 0) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        if (buttons[i].text == 0 || buttons[i].key_code > 0xFFU ||
            buttons[i].width < 0 || buttons[i].height < 0) {
            while (--i >= 0) {
                ldFree(entries[i].text);
            }
            ldFree(entries);
            return -1;
        }
        {
            size_t text_len = strlen(buttons[i].text) + 1U;
            entries[i].text = (char *)ldMalloc(text_len);
            if (entries[i].text != 0) {
                memcpy(entries[i].text, buttons[i].text, text_len);
            }
        }
        if (entries[i].text == 0) {
            while (--i >= 0) {
                ldFree(entries[i].text);
            }
            ldFree(entries);
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

    tinyui_keyboard_free_layout(keyboard);
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

int tinyui_keyboard_get_buttons(const tinyui_obj_t *keyboard_obj, const struct tinyui_keyboard_button **buttons, int *count)
{
    const struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard_const(keyboard_obj);
    if (keyboard == 0) { return -1; }

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

int tinyui_keyboard_set_on_key_event(tinyui_obj_t *keyboard_obj, tinyui_keyboard_event_cb cb, void *user_data)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

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

int tinyui_keyboard_get_selected_key_code(const tinyui_obj_t *keyboard_obj)
{
    const struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard_const(keyboard_obj);
    if (keyboard == 0) { return -1; }

    unsigned int key_code = 0;

    if (tinyui_keyboard_get_selected_key_code_internal(keyboard, &key_code) != 0) {
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

int tinyui_keyboard_set_layout(tinyui_obj_t *keyboard_obj, const struct tinyui_keyboard_button *buttons, int count)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    return tinyui_keyboard_set_buttons((tinyui_obj_t *)keyboard, buttons, count);
}

/**
 * @brief Set event callback of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_keyboard_set_event_callback(tinyui_obj_t *keyboard_obj, tinyui_keyboard_event_cb cb, void *user_data)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    return tinyui_keyboard_set_on_key_event((tinyui_obj_t *)keyboard, cb, user_data);
}

/**
 * @brief Set draw callback of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_keyboard_set_draw_callback(tinyui_obj_t *keyboard_obj, tinyui_keyboard_draw_cb cb, void *user_data)
{
    struct tinyui_keyboard *keyboard = tinyui_keyboard_as_keyboard(keyboard_obj);
    if (keyboard == 0) { return -1; }

    if (keyboard == 0) {
        return -1;
    }

    keyboard->draw_cb = cb;
    keyboard->draw_user_data = user_data;
    return 0;
}
