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
#include "widgets/checkbox.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldCheckBox.h"

#include <stdlib.h>


static struct tinyui_checkbox *tinyui_checkbox_as_checkbox(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_CHECKBOX)) {
        return 0;
    }
    return (struct tinyui_checkbox *)w;
}

static const struct tinyui_checkbox *tinyui_checkbox_as_checkbox_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_CHECKBOX)) {
        return 0;
    }
    return (const struct tinyui_checkbox *)w;
}

/* ---- test seam state ---- */


static ldCheckBox_t *tinyui_checkbox_backend(struct tinyui_checkbox *checkbox)
{
    if (checkbox == 0 || checkbox->widget.ld_widget == 0
        || checkbox->widget.kind != TINYUI_BACKEND_WIDGET_CHECKBOX) {
        return 0;
    }
    return (ldCheckBox_t *)checkbox->widget.ld_widget;
}

static void *tinyui_runtime_internal_checkbox_ld_init(void *ctx,
                                     struct ld_scene_t *scene,
                                     uint16_t name_id,
                                     uint16_t parent_name_id)
{
    ldCheckBox_t *ld_checkbox;

    (void)ctx;
    ld_checkbox = ldCheckBox_init(scene, NULL, name_id, parent_name_id, 0, 0, 220, 30);
    if (ld_checkbox == 0) {
        return 0;
    }
    return ld_checkbox;
}

static int checkbox_props_valid(const tinyui_checkbox_props_t *props)
{
    return props != 0
        && (props->unchecked_source == 0
            || tinyui_image_source_get_image_tile(props->unchecked_source) != 0)
        && (props->checked_source == 0
            || tinyui_image_source_get_image_tile(props->checked_source) != 0)
        && (props->radio_group == -1
            || (props->radio_group >= 0 && props->radio_group <= 255))
        && (props->string_left_space == -1
            || (props->string_left_space >= 0 && props->string_left_space <= 65535))
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

/**
 * @brief Create checkbox widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_checkbox_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "checkbox";
    if (parent_w == 0) { return 0; }

    struct tinyui_checkbox *checkbox;
    ldCheckBox_t *ld_checkbox;

    if (parent_w == 0 || id == 0) {
        return 0;
    }
    checkbox = (struct tinyui_checkbox *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                                   TINYUI_BACKEND_WIDGET_CHECKBOX,
                                                                   tinyui_runtime_internal_checkbox_ld_init,
                                                                   0,
                                                                   sizeof(*checkbox));
    if (checkbox == 0) {
        return 0;
    }
    checkbox->id = id;
    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        tinyui_runtime_internal_widget_destroy_common(&checkbox->widget);
        return 0;
    }
    checkbox->widget.font = 0;
    ld_checkbox->ptFont = tinyui_resolve_ld_font(0, 12);
    if (ld_checkbox->ptFont == 0) {
        tinyui_runtime_internal_widget_destroy_common(&checkbox->widget);
        return 0;
    }

    return (tinyui_obj_t *)checkbox;
}

/**
 * @brief Create checkbox widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_checkbox_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_checkbox_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_checkbox *checkbox;

    if (props == 0) {
        return tinyui_checkbox_create(parent);
    }

    obj = tinyui_checkbox_create(parent);
    if (obj == 0) {
        return 0;
    }
    checkbox = (struct tinyui_checkbox *)(void *)obj;

    if ((props->fields & TINYUI_CHECKBOX_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&checkbox->widget, props->user_data) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&checkbox->widget, props->style_class) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
        if ((props->fields & TINYUI_CHECKBOX_FIELD_WIDTH) != 0 || (props->fields & TINYUI_CHECKBOX_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&checkbox->widget);
        int h = tinyui_runtime_internal_widget_get_height(&checkbox->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_CHECKBOX_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_CHECKBOX_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&checkbox->widget, w, h) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
            return 0;
        }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_TEXT) != 0) {
    if (tinyui_checkbox_set_text((tinyui_obj_t *)checkbox, props->text) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_BG_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_bg_color(&checkbox->widget, props->bg_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_TEXT_COLOR) != 0) {
    if (tinyui_checkbox_set_text_color((tinyui_obj_t *)checkbox, props->text_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_BORDER_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_border_color(&checkbox->widget, props->border_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_RADIUS) != 0) {
    if (tinyui_runtime_internal_widget_set_radius(&checkbox->widget, props->radius) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_PADDING) != 0) {
    if (tinyui_runtime_internal_widget_set_padding(&checkbox->widget, props->padding) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_CHECKED) != 0) {
    if (tinyui_checkbox_set_checked((tinyui_obj_t *)checkbox, props->checked) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_ON_TOGGLED) != 0) {
        /* props still carry legacy tinyui_value_changed_cb; Task 7 set_on_* is
         * unified-pool only. Reject rather than fake-success. Prefer
         * tinyui_obj_add_event_cb after create. */
        if (props->on_toggled != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
            return 0;
        }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_CHECK_COLOR) != 0) {
    if (tinyui_checkbox_set_check_color((tinyui_obj_t *)checkbox, props->check_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_UNCHECKED_SOURCE) != 0) {
    if (tinyui_checkbox_set_unchecked_source((tinyui_obj_t *)checkbox, props->unchecked_source) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_CHECKED_SOURCE) != 0) {
    if (tinyui_checkbox_set_checked_source((tinyui_obj_t *)checkbox, props->checked_source) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_RADIO_GROUP) != 0) {
    if (tinyui_checkbox_set_radio_group((tinyui_obj_t *)checkbox, props->radio_group) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }
    if ((props->fields & TINYUI_CHECKBOX_FIELD_STRING_LEFT_SPACE) != 0) {
    if (tinyui_checkbox_set_string_left_space((tinyui_obj_t *)checkbox, props->string_left_space) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)checkbox);
        return 0;
    }
    }

    return obj;
}


/**
 * @brief Set checked of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] checked Checked state
 * @return 0 on success, -1 on failure
 */

int tinyui_checkbox_set_checked(tinyui_obj_t *checkbox_obj, int checked)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    ldCheckBox_t *ld_checkbox;
    int normalized_checked;

    if (checkbox == 0) {
        return -1;
    }

    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    normalized_checked = checked != 0;
    if (ld_checkbox->isChecked == (normalized_checked != 0)
        && checkbox->checked == normalized_checked
        && checkbox->widget.value == normalized_checked) {
        return 0;
    }

    /* Programmatic path: sync LD + cache only. Do not emit user events. */
    checkbox->checked = normalized_checked;
    checkbox->widget.value = normalized_checked;
    tinyui_runtime_internal_widget_sync_ld_value(&checkbox->widget, normalized_checked);
    return 0;
}

/**
 * @brief checkbox is checked
 *
 * @param[in] checkbox Checkbox widget instance
 * @return 0 on success
 */

int tinyui_checkbox_is_checked(tinyui_obj_t *checkbox_obj)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0) {
        return -1;
    }

    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    /* Read real LD state; keep wrapper cache coherent. */
    checkbox->checked = ld_checkbox->isChecked ? 1 : 0;
    checkbox->widget.value = checkbox->checked;
    return checkbox->checked;
}

/**
 * @brief Set text of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] text Text widget instance
 * @return -1 on failure
 */

int tinyui_checkbox_set_text(tinyui_obj_t *checkbox_obj, const char *text)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    if (checkbox == 0) { return -1; }

    if (checkbox == 0 || text == 0) {
        return -1;
    }

    return tinyui_runtime_internal_widget_set_text(&checkbox->widget, text);
}

/**
 * @brief Set check color of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_checkbox_set_check_color(tinyui_obj_t *checkbox_obj, unsigned int rgb)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    if (checkbox == 0) { return -1; }

    ldCheckBox_t *ld_checkbox;

    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetColor(ld_checkbox,
                       ld_checkbox->bgColor,
                       (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set text color of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_checkbox_set_text_color(tinyui_obj_t *checkbox_obj, unsigned int rgb)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    if (checkbox == 0) { return -1; }

    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0 || tinyui_runtime_internal_widget_set_text_color(&checkbox->widget, rgb) != 0) {
        return -1;
    }

    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetTextColor(ld_checkbox, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set unchecked source of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_checkbox_set_unchecked_source(tinyui_obj_t *checkbox_obj, struct tinyui_image_source *source)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0) {
        return -1;
    }
    if (source != 0 && source->kind == TINYUI_IMAGE_SOURCE_EMPTY) {
        return -1;
    }

    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetImage(ld_checkbox,
                       source != 0 ? tinyui_image_source_get_image_tile(source) : 0,
                       source != 0 ? tinyui_image_source_get_mask_tile(source) : 0,
                       ld_checkbox->ptCheckedImgTile,
                       ld_checkbox->ptCheckedMaskTile);
    return 0;
}

/**
 * @brief Set checked source of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_checkbox_set_checked_source(tinyui_obj_t *checkbox_obj, struct tinyui_image_source *source)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0) {
        return -1;
    }
    if (source != 0 && source->kind == TINYUI_IMAGE_SOURCE_EMPTY) {
        return -1;
    }

    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetImage(ld_checkbox,
                       ld_checkbox->ptUncheckedImgTile,
                       ld_checkbox->ptUncheckedMaskTile,
                       source != 0 ? tinyui_image_source_get_image_tile(source) : 0,
                       source != 0 ? tinyui_image_source_get_mask_tile(source) : 0);
    return 0;
}

/**
 * @brief Set radio group of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] radio_group Radio button group ID
 * @return -1 on failure
 */

int tinyui_checkbox_set_radio_group(tinyui_obj_t *checkbox_obj, int radio_group)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    if (checkbox == 0) { return -1; }

    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0 || radio_group < 0 || radio_group > 255) {
        return -1;
    }

    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetRadioButtonGroup(ld_checkbox, (uint8_t)radio_group);
    return 0;
}

/**
 * @brief Set string left space of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] space Spacing
 * @return -1 on failure
 */

int tinyui_checkbox_set_string_left_space(tinyui_obj_t *checkbox_obj, int space)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    if (checkbox == 0) { return -1; }

    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0 || space < 0 || space > 65535) {
        return -1;
    }

    ld_checkbox = tinyui_checkbox_backend(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetStringLeftSpace(ld_checkbox, (uint16_t)space);
    return 0;
}

static int tinyui_checkbox_replace_event_cb(struct tinyui_checkbox *checkbox,
                                            tinyui_event_handle_t *slot,
                                            uint32_t event_mask,
                                            tinyui_event_cb_t cb,
                                            void *user_data)
{
    tinyui_event_handle_t handle = 0U;
    tinyui_result_t rc;

    if (checkbox == 0 || slot == 0) {
        return -1;
    }

    if (*slot != 0U) {
        (void)tinyui_obj_remove_event_cb((tinyui_obj_t *)checkbox, *slot);
        *slot = 0U;
    }

    if (cb == 0) {
        return 0;
    }

    rc = tinyui_obj_add_event_cb((tinyui_obj_t *)checkbox,
                                 event_mask,
                                 cb,
                                 user_data,
                                 &handle);
    if (rc != TINYUI_OK) {
        return -1;
    }
    *slot = handle;
    return 0;
}

/**
 * @brief Set on toggled of checkbox widget (narrow forward to unified event pool)
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] cb Unified event callback (const tinyui_event_t *)
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_checkbox_set_on_toggled(tinyui_obj_t *checkbox_obj, tinyui_event_cb_t cb, void *user_data)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_as_checkbox(checkbox_obj);
    if (checkbox == 0) {
        return -1;
    }

    return tinyui_checkbox_replace_event_cb(checkbox,
                                            &checkbox->on_toggled_handle,
                                            TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                            cb,
                                            user_data);
}
