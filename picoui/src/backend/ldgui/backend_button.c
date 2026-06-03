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
#include "ldButton.h"
#include "xBtnAction.h"

#include <stdlib.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_a1_font_t ARM_2D_FONT_16x24;

struct picoui_backend_button_host {
    struct picoui_backend_widget widget;
    xBtnInfo_t action_info;
};

static struct picoui_backend_app_state *picoui_backend_button_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldButton_t *picoui_backend_button_get_ld(struct picoui_button *button)
{
    struct picoui_backend_widget *backend;

    if (button == NULL || button->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)button->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_BUTTON || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldButton_t *)backend->ld_widget;
}

static arm_2d_font_t *picoui_backend_button_default_font(void)
{
    return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
}

static arm_2d_font_t *picoui_backend_button_resolve_font(const struct picoui_font *font)
{
    if (font != NULL && font->kind == PICOUI_FONT_KIND_VRES && font->vres_addr != 0) {
        return (arm_2d_font_t *)ldBaseGetVresFont(font->vres_addr);
    }

    if (font == NULL || font->family == NULL || font->size <= 0) {
        return picoui_backend_button_default_font();
    }

    if (strcmp(font->family, "Sans") == 0 && font->size >= 20) {
        return (arm_2d_font_t *)&ARM_2D_FONT_16x24;
    }

    return picoui_backend_button_default_font();
}

/**
 * @brief Create backend for button
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_button(void *parent, const char *id)
{
    struct picoui_backend_button_host *host;
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldButton_t *ld_button;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_button_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    host = calloc(1, sizeof(*host));
    if (host == 0) {
        return 0;
    }
    widget = &host->widget;

    name_id = ++app_state->next_ld_name_id;
    ld_button = ldButton_init(app_state->ld_scene, NULL, name_id, parent_widget->ld_name_id, 0, 0, 160, 36);
    if (ld_button == NULL) {
        free(host);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_BUTTON;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_button;
    widget->ld_name_id = name_id;
    _xBtnInit(name_id, (isBtnPressFunc)ldButtonActionIsPressById, &host->action_info);
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(host);
        return 0;
    }
    return widget;
}

/**
 * @brief Set font of button backend
 *
 * @param[in] button Button widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_set_font(struct picoui_button *button, const struct picoui_font *font)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);
    arm_2d_font_t *resolved_font;

    if (ld_button == NULL) {
        return -1;
    }

    resolved_font = picoui_backend_button_resolve_font(font);
    if (resolved_font == NULL) {
        return -1;
    }

    ldButtonSetFont(ld_button, resolved_font);
    return 0;
}

/**
 * @brief Set release image of button backend
 *
 * @param[in] button Button widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_set_release_image(struct picoui_button *button,
                                            struct picoui_image_source *source)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL) {
        return -1;
    }

    ldButtonSetImage(ld_button,
                     source != NULL ? source->img_tile : NULL,
                     source != NULL ? source->mask_tile : NULL,
                     ld_button->ptPressImgTile,
                     ld_button->ptPressMaskTile);
    return 0;
}

/**
 * @brief Set press image of button backend
 *
 * @param[in] button Button widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_set_press_image(struct picoui_button *button,
                                          struct picoui_image_source *source)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL) {
        return -1;
    }

    ldButtonSetImage(ld_button,
                     ld_button->ptReleaseImgTile,
                     ld_button->ptReleaseMaskTile,
                     source != NULL ? source->img_tile : NULL,
                     source != NULL ? source->mask_tile : NULL);
    return 0;
}

/**
 * @brief Set transparent of button backend
 *
 * @param[in] button Button widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_set_transparent(struct picoui_button *button, int transparent)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL) {
        return -1;
    }

    ldButtonSetTransparent(ld_button, transparent != 0);
    return 0;
}

/**
 * @brief Get transparent from button backend
 *
 * @param[out] button Button widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_get_transparent(struct picoui_button *button, int *transparent)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL || transparent == NULL) {
        return -1;
    }

    *transparent = ldButtonGetTransparent(ld_button) ? 1 : 0;
    return 0;
}

/**
 * @brief Set checkable of button backend
 *
 * @param[in] button Button widget instance
 * @param[in] checkable checkable
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_set_checkable(struct picoui_button *button, int checkable)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL) {
        return -1;
    }

    ldButtonSetCheckable(ld_button, checkable != 0);
    return 0;
}

/**
 * @brief Get checkable from button backend
 *
 * @param[out] button Button widget instance
 * @param[in] checkable checkable
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_get_checkable(struct picoui_button *button, int *checkable)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL || checkable == NULL) {
        return -1;
    }

    *checkable = ldButtonGetCheckable(ld_button) ? 1 : 0;
    return 0;
}

/**
 * @brief Set key value of button backend
 *
 * @param[in] button Button widget instance
 * @param[in] key_value key value
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_set_key_value(struct picoui_button *button, unsigned int key_value)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL) {
        return -1;
    }

    ldButtonSetKeyValue(ld_button, (uint32_t)key_value);
    return 0;
}

/**
 * @brief Get key value from button backend
 *
 * @param[out] button Button widget instance
 * @param[in] key_value key value
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_get_key_value(struct picoui_button *button, unsigned int *key_value)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL || key_value == NULL) {
        return -1;
    }

    *key_value = (unsigned int)ldButtonGetKeyValue(ld_button);
    return 0;
}

/**
 * @brief Set pressed of button backend
 *
 * @param[in] button Button widget instance
 * @param[in] pressed Pressed state
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_set_pressed(struct picoui_button *button, int pressed)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL) {
        return -1;
    }

    ldButtonSetPress(ld_button, pressed != 0);
    return 0;
}

/**
 * @brief Get pressed from button backend
 *
 * @param[out] button Button widget instance
 * @param[in] pressed Pressed state
 * @return 0 on success, -1 on failure
 */

int picoui_backend_button_get_pressed(struct picoui_button *button, int *pressed)
{
    ldButton_t *ld_button = picoui_backend_button_get_ld(button);

    if (ld_button == NULL || pressed == NULL) {
        return -1;
    }

    *pressed = ldButtonGetPress(ld_button) ? 1 : 0;
    return 0;
}
