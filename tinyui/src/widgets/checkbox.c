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
#include "picoui/checkbox.h"
#include "../core/runtime_bridge.h"
#include "../backend/ldgui/backend.h"
#include "../../../src/gui/ldCheckBox.h"

#include <stdlib.h>

int picoui_backend_widget_unbind_host(void *backend_widget);
int picoui_backend_widget_detach_from_parent(void *backend_widget);

static int picoui_checkbox_fail_next_set_check_color = 0;

static ldColor picoui_checkbox_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static ldCheckBox_t *picoui_checkbox_get_ld(struct picoui_checkbox *checkbox)
{
    struct picoui_backend_widget *backend;

    if (checkbox == 0 || checkbox->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_CHECKBOX || backend->ld_widget == 0) {
        return 0;
    }

    return (ldCheckBox_t *)backend->ld_widget;
}

static int picoui_checkbox_props_are_valid(const struct picoui_checkbox_props *props)
{
    return props != 0
        && props->id != 0
        && (props->has_unchecked_source == 0
            || props->unchecked_source == 0
            || props->unchecked_source->img_tile != 0)
        && (props->has_checked_source == 0
            || props->checked_source == 0
            || props->checked_source->img_tile != 0)
        && (props->has_radio_group == 0
            || (props->radio_group >= 0 && props->radio_group <= 255))
        && (props->has_string_left_space == 0
            || (props->string_left_space >= 0 && props->string_left_space <= 65535))
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void picoui_checkbox_dispose_partial(struct picoui_checkbox *checkbox)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;

    if (checkbox == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    if (backend != 0) {
        app_state = picoui_runtime_bridge_backend_state(backend->owner);
        if (backend->parent != 0) {
            (void)picoui_backend_widget_detach_from_parent(backend);
        }
        (void)picoui_backend_widget_unbind_host(backend);
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldCheckBox_depose(app_state->ld_scene, (ldCheckBox_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(checkbox);
}

void picoui_backend_checkbox_test_fail_next_set_check_color(void)
{
    picoui_checkbox_fail_next_set_check_color = 1;
}

/**
 * @brief Create checkbox widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_checkbox *picoui_checkbox_create(struct picoui_window *parent, const char *id)
{
    struct picoui_checkbox *checkbox;
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldCheckBox_t *ld_checkbox;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->widget.backend_widget;
    app_state = picoui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    checkbox = calloc(1, sizeof(*checkbox));
    if (checkbox == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(checkbox);
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(checkbox);
        return 0;
    }

    ld_checkbox = ldCheckBox_init(app_state->ld_scene,
                                  NULL,
                                  name_id,
                                  parent_backend->ld_name_id,
                                  0,
                                  0,
                                  220,
                                  30);
    if (ld_checkbox == 0) {
        free(backend);
        free(checkbox);
        return 0;
    }
    ldCheckBoxSetColor(ld_checkbox, __RGB(238, 233, 224), __RGB(32, 87, 196));
    ldCheckBoxSetTextColor(ld_checkbox, __RGB(32, 87, 196));

    if (picoui_backend_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_CHECKBOX,
                                         id,
                                         parent_backend->theme) != 0) {
        ldCheckBox_depose(app_state->ld_scene, ld_checkbox);
        free(backend);
        free(checkbox);
        return 0;
    }
    backend->ld_widget = ld_checkbox;
    backend->ld_name_id = name_id;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent_backend, backend) != 0) {
        ldCheckBox_depose(app_state->ld_scene, ld_checkbox);
        free(backend);
        free(checkbox);
        return 0;
    }

    checkbox->id = id;
    checkbox->widget.backend_widget = backend;
    checkbox->widget.visible = 1;
    checkbox->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(checkbox->widget.backend_widget, &checkbox->widget) != 0) {
        picoui_checkbox_dispose_partial(checkbox);
        return 0;
    }
    return checkbox;
}

/**
 * @brief Create checkbox widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_checkbox *picoui_checkbox_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_checkbox_props *props)
{
    struct picoui_checkbox *checkbox;
    struct picoui_backend_widget *backend;

    if (!picoui_checkbox_props_are_valid(props)) {
        return 0;
    }

    checkbox = picoui_checkbox_create(parent, props->id);
    if (checkbox == 0) {
        return 0;
    }

    checkbox->checked = 0;
    checkbox->cb = 0;
    checkbox->user_data = 0;
    if (props->text != 0 && picoui_checkbox_set_text(checkbox, props->text) != 0) {
        picoui_checkbox_dispose_partial(checkbox);
        return 0;
    }
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    checkbox->checked = props->checked != 0;
    if (picoui_widget_update_value(backend,
                                   checkbox->checked,
                                   0,
                                   &checkbox->widget,
                                   0) != 0) {
        picoui_checkbox_dispose_partial(checkbox);
        return 0;
    }
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    backend->dispatch_count = 0;
    checkbox->cb = props->on_toggled;
    checkbox->user_data = props->user_data;
    if (picoui_widget_set_user_data(&checkbox->widget, props->user_data) != 0) {
        picoui_checkbox_dispose_partial(checkbox);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&checkbox->widget, props->style_class) != 0) {
        picoui_checkbox_dispose_partial(checkbox);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&checkbox->widget, props->width, props->height) != 0) {
        picoui_checkbox_dispose_partial(checkbox);
        return 0;
    }
    if (picoui_widget_set_bg_color(&checkbox->widget, props->bg_color) != 0
        || picoui_checkbox_set_text_color(checkbox, props->text_color) != 0
        || picoui_widget_set_border_color(&checkbox->widget, props->border_color) != 0
        || picoui_widget_set_radius(&checkbox->widget, props->radius) != 0
        || picoui_widget_set_padding(&checkbox->widget, props->padding) != 0
        || (props->has_check_color != 0
            && picoui_checkbox_set_check_color(checkbox, props->check_color) != 0)
        || (props->has_unchecked_source != 0
            && picoui_checkbox_set_unchecked_source(checkbox, props->unchecked_source) != 0)
        || (props->has_checked_source != 0
            && picoui_checkbox_set_checked_source(checkbox, props->checked_source) != 0)
        || (props->has_radio_group != 0
            && picoui_checkbox_set_radio_group(checkbox, props->radio_group) != 0)
        || (props->has_string_left_space != 0
            && picoui_checkbox_set_string_left_space(checkbox, props->string_left_space) != 0)) {
        picoui_checkbox_dispose_partial(checkbox);
        return 0;
    }
    return checkbox;
}

/**
 * @brief Set checked of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] checked Checked state
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_checked(struct picoui_checkbox *checkbox, int checked)
{
    int normalized_checked;

    if (checkbox == 0) {
        return -1;
    }

    normalized_checked = checked != 0;
    if (checkbox->checked == normalized_checked) {
        return 0;
    }

    if (checkbox->widget.backend_widget == 0) {
        return -1;
    }

    checkbox->checked = normalized_checked;
    return picoui_widget_update_value(checkbox->widget.backend_widget,
                                      checkbox->checked,
                                      checkbox->cb,
                                      &checkbox->widget,
                                      checkbox->user_data);
}

/**
 * @brief checkbox is checked
 *
 * @param[in] checkbox Checkbox widget instance
 * @return 0 on success
 */

int picoui_checkbox_is_checked(struct picoui_checkbox *checkbox)
{
    if (checkbox == 0) {
        return 0;
    }

    return checkbox->checked;
}

/**
 * @brief Set text of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] text Text widget instance
 * @return -1 on failure
 */

int picoui_checkbox_set_text(struct picoui_checkbox *checkbox, const char *text)
{
    if (checkbox == 0 || text == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&checkbox->widget, text) != 0) {
        return -1;
    }
    return picoui_backend_set_text(checkbox->widget.backend_widget, text);
}

/**
 * @brief Set check color of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_checkbox_set_check_color(struct picoui_checkbox *checkbox, unsigned int rgb)
{
    ldCheckBox_t *ld_checkbox;

    if (picoui_checkbox_fail_next_set_check_color != 0) {
        picoui_checkbox_fail_next_set_check_color = 0;
        return -1;
    }

    ld_checkbox = picoui_checkbox_get_ld(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetColor(ld_checkbox,
                       ld_checkbox->bgColor,
                       picoui_checkbox_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set text color of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_checkbox_set_text_color(struct picoui_checkbox *checkbox, unsigned int rgb)
{
    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0 || picoui_widget_set_text_color(&checkbox->widget, rgb) != 0) {
        return -1;
    }

    ld_checkbox = picoui_checkbox_get_ld(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetTextColor(ld_checkbox, picoui_checkbox_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set unchecked source of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int picoui_checkbox_set_unchecked_source(struct picoui_checkbox *checkbox,
                                         struct picoui_image_source *source)
{
    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_checkbox = picoui_checkbox_get_ld(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetImage(ld_checkbox,
                       source != 0 ? source->img_tile : 0,
                       source != 0 ? source->mask_tile : 0,
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

int picoui_checkbox_set_checked_source(struct picoui_checkbox *checkbox,
                                       struct picoui_image_source *source)
{
    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_checkbox = picoui_checkbox_get_ld(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetImage(ld_checkbox,
                       ld_checkbox->ptUncheckedImgTile,
                       ld_checkbox->ptUncheckedMaskTile,
                       source != 0 ? source->img_tile : 0,
                       source != 0 ? source->mask_tile : 0);
    return 0;
}

/**
 * @brief Set radio group of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] radio_group Radio button group ID
 * @return -1 on failure
 */

int picoui_checkbox_set_radio_group(struct picoui_checkbox *checkbox, int radio_group)
{
    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0 || radio_group < 0 || radio_group > 255) {
        return -1;
    }

    ld_checkbox = picoui_checkbox_get_ld(checkbox);
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

int picoui_checkbox_set_string_left_space(struct picoui_checkbox *checkbox, int space)
{
    ldCheckBox_t *ld_checkbox;

    if (checkbox == 0 || space < 0 || space > 65535) {
        return -1;
    }

    ld_checkbox = picoui_checkbox_get_ld(checkbox);
    if (ld_checkbox == 0) {
        return -1;
    }

    ldCheckBoxSetStringLeftSpace(ld_checkbox, (uint16_t)space);
    return 0;
}

/**
 * @brief Set on toggled of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_on_toggled(struct picoui_checkbox *checkbox,
                                   picoui_value_changed_cb cb,
                                   void *user_data)
{
    if (checkbox == 0) {
        return -1;
    }

    checkbox->cb = cb;
    checkbox->user_data = user_data;
    return 0;
}
