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
#include "ldCheckBox.h"

#include <stdlib.h>

static ldColor picoui_backend_checkbox_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static ldCheckBox_t *picoui_backend_checkbox_get_ld(struct picoui_checkbox *checkbox)
{
    struct picoui_backend_widget *backend;

    if (checkbox == NULL || checkbox->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_CHECKBOX || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldCheckBox_t *)backend->ld_widget;
}

static struct picoui_backend_app_state *picoui_backend_checkbox_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

/**
 * @brief Create backend for checkbox
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_checkbox(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldCheckBox_t *ld_checkbox;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_checkbox_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_checkbox = ldCheckBox_init(app_state->ld_scene,
                                  NULL,
                                  name_id,
                                  parent_widget->ld_name_id,
                                  0,
                                  0,
                                  220,
                                  30);
    if (ld_checkbox == NULL) {
        free(widget);
        return 0;
    }
    ldCheckBoxSetColor(ld_checkbox, __RGB(238, 233, 224), __RGB(32, 87, 196));
    ldCheckBoxSetTextColor(ld_checkbox, __RGB(32, 87, 196));

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_CHECKBOX;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_checkbox;
    widget->ld_name_id = name_id;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set check color of checkbox backend
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_checkbox_set_check_color(struct picoui_checkbox *checkbox, unsigned int rgb)
{
    ldCheckBox_t *ld_checkbox = picoui_backend_checkbox_get_ld(checkbox);

    if (ld_checkbox == NULL) {
        return -1;
    }

    ldCheckBoxSetColor(ld_checkbox, ld_checkbox->bgColor, picoui_backend_checkbox_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set text color of checkbox backend
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_checkbox_set_text_color(struct picoui_checkbox *checkbox, unsigned int rgb)
{
    ldCheckBox_t *ld_checkbox = picoui_backend_checkbox_get_ld(checkbox);

    if (ld_checkbox == NULL) {
        return -1;
    }

    ldCheckBoxSetTextColor(ld_checkbox, picoui_backend_checkbox_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Set unchecked source of checkbox backend
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_checkbox_set_unchecked_source(struct picoui_checkbox *checkbox,
                                                 struct picoui_image_source *source)
{
    ldCheckBox_t *ld_checkbox = picoui_backend_checkbox_get_ld(checkbox);

    if (ld_checkbox == NULL) {
        return -1;
    }

    ldCheckBoxSetImage(ld_checkbox,
                       source != NULL ? source->img_tile : NULL,
                       source != NULL ? source->mask_tile : NULL,
                       ld_checkbox->ptCheckedImgTile,
                       ld_checkbox->ptCheckedMaskTile);
    return 0;
}

/**
 * @brief Set checked source of checkbox backend
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_checkbox_set_checked_source(struct picoui_checkbox *checkbox,
                                               struct picoui_image_source *source)
{
    ldCheckBox_t *ld_checkbox = picoui_backend_checkbox_get_ld(checkbox);

    if (ld_checkbox == NULL) {
        return -1;
    }

    ldCheckBoxSetImage(ld_checkbox,
                       ld_checkbox->ptUncheckedImgTile,
                       ld_checkbox->ptUncheckedMaskTile,
                       source != NULL ? source->img_tile : NULL,
                       source != NULL ? source->mask_tile : NULL);
    return 0;
}

/**
 * @brief Set radio group of checkbox backend
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] radio_group Radio button group ID
 * @return 0 on success, -1 on failure
 */

int picoui_backend_checkbox_set_radio_group(struct picoui_checkbox *checkbox, int radio_group)
{
    ldCheckBox_t *ld_checkbox = picoui_backend_checkbox_get_ld(checkbox);

    if (ld_checkbox == NULL || radio_group < 0 || radio_group > 255) {
        return -1;
    }

    ldCheckBoxSetRadioButtonGroup(ld_checkbox, (uint8_t)radio_group);
    return 0;
}

/**
 * @brief Set string left space of checkbox backend
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] space Spacing
 * @return 0 on success, -1 on failure
 */

int picoui_backend_checkbox_set_string_left_space(struct picoui_checkbox *checkbox, int space)
{
    ldCheckBox_t *ld_checkbox = picoui_backend_checkbox_get_ld(checkbox);

    if (ld_checkbox == NULL || space < 0 || space > 65535) {
        return -1;
    }

    ldCheckBoxSetStringLeftSpace(ld_checkbox, (uint16_t)space);
    return 0;
}
