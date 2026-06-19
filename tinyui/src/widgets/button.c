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
#include "button.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/misc/xBtnAction.h"

#include <stdlib.h>
#include <string.h>

struct tinyui_button_backend_host {
    struct tinyui_backend_widget widget;
    xBtnInfo_t action_info;
};

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_a1_font_t ARM_2D_FONT_16x24;
int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static int tinyui_button_fail_next_set_font = 0;

static ldButton_t *tinyui_button_get_ld(const struct tinyui_button *button)
{
    const struct tinyui_backend_widget *backend;

    if (button == 0 || button->widget.backend_widget == 0) {
        return 0;
    }

    backend = (const struct tinyui_backend_widget *)button->widget.backend_widget;
    return (ldButton_t *)backend->ld_widget;
}

static arm_2d_font_t *tinyui_button_default_font(void)
{
    return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
}

static arm_2d_font_t *tinyui_button_resolve_font(const struct tinyui_font *font)
{
    if (font != NULL && font->kind == TINYUI_FONT_KIND_VRES && font->vres_addr != 0) {
        return (arm_2d_font_t *)ldBaseGetVresFont(font->vres_addr);
    }

    if (font == NULL || font->family == NULL || font->size <= 0) {
        return tinyui_button_default_font();
    }

    if (strcmp(font->family, "Sans") == 0 && font->size >= 20) {
        return (arm_2d_font_t *)&ARM_2D_FONT_16x24;
    }

    return tinyui_button_default_font();
}

static void tinyui_button_dispose_partial(struct tinyui_button *button)
{
    struct tinyui_button_backend_host *host;
    struct tinyui_app *app_state;

    if (button == 0) {
        return;
    }

    host = (struct tinyui_button_backend_host *)button->widget.backend_widget;
    if (host != 0) {
        app_state = tinyui_runtime_bridge_backend_state(host->widget.owner);
        xBtnRemove(&host->action_info);
        if (host->widget.parent != 0) {
            (void)tinyui_runtime_bridge_detach_from_parent(&host->widget);
        }
        (void)tinyui_runtime_bridge_unbind_host(&host->widget);
        if (app_state != 0 && app_state->ld_scene != 0 && host->widget.ld_widget != 0) {
            ldButton_depose(app_state->ld_scene, (ldButton_t *)host->widget.ld_widget);
        }
        free(host);
    }

    free(button);
}

void tinyui_button_test_fail_next_set_font(void)
{
    tinyui_button_fail_next_set_font = 1;
}

static int tinyui_button_props_are_valid(const struct tinyui_button_props *props)
{
    return props != 0
        && props->id != 0
        && (props->release_image == 0 || props->release_image->img_tile != 0)
        && (props->press_image == 0 || props->press_image->img_tile != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static struct tinyui_button *tinyui_button_alloc(struct tinyui_window *parent, const char *id)
{
    struct tinyui_button *button;
    struct tinyui_button_backend_host *host;
    struct tinyui_backend_widget *parent_backend;
    struct tinyui_app *app_state;
    ldButton_t *ld_button;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct tinyui_backend_widget *)parent->widget.backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    button = calloc(1, sizeof(*button));
    if (button == 0) {
        return 0;
    }

    host = calloc(1, sizeof(*host));
    if (host == 0) {
        free(button);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(host);
        free(button);
        return 0;
    }

    ld_button = ldButton_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_backend->ld_name_id,
                              0,
                              0,
                              160,
                              36);
    if (ld_button == 0) {
        free(host);
        free(button);
        return 0;
    }

    if (tinyui_widget_init_child(&host->widget,
                                         parent_backend,
                                         TINYUI_BACKEND_WIDGET_BUTTON,
                                         id,
                                         parent_backend->theme) != 0) {
        ldButton_depose(app_state->ld_scene, ld_button);
        free(host);
        free(button);
        return 0;
    }
    host->widget.ld_widget = ld_button;
    host->widget.ld_name_id = name_id;
    _xBtnInit(name_id, (isBtnPressFunc)ldButtonActionIsPressById, &host->action_info);
    if (tinyui_widget_attach_child(parent_backend, &host->widget) != 0) {
        ldButton_depose(app_state->ld_scene, ld_button);
        free(host);
        free(button);
        return 0;
    }

    button->id = id;
    button->widget.backend_widget = &host->widget;
    button->widget.visible = 1;
    button->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(button->widget.backend_widget, &button->widget) != 0) {
        free(button);
        return 0;
    }
    return button;
}

/**
 * @brief Create button widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_button *tinyui_button_create(struct tinyui_window *parent, const char *id)
{
    return tinyui_button_alloc(parent, id);
}

/**
 * @brief button init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_button *tinyui_button_init(struct tinyui_window *parent, const char *id)
{
    return tinyui_button_create(parent, id);
}

/**
 * @brief Create button widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_button *tinyui_button_create_with_props(struct tinyui_window *parent,
                                                      const struct tinyui_button_props *props)
{
    struct tinyui_button *button;

    if (!tinyui_button_props_are_valid(props)) {
        return 0;
    }

    button = tinyui_button_alloc(parent, props->id);
    if (button == 0) {
        return 0;
    }

    button->on_clicked = props->on_clicked;
    button->user_data = props->user_data;
    if (tinyui_widget_set_user_data(&button->widget, props->user_data) != 0) {
        tinyui_button_dispose_partial(button);
        return 0;
    }
    if (props->text != 0 && tinyui_button_set_text(button, props->text) != 0) {
        tinyui_button_dispose_partial(button);
        return 0;
    }
    if (props->font != 0 && tinyui_button_set_font(button, props->font) != 0) {
        tinyui_button_dispose_partial(button);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && tinyui_widget_set_size(&button->widget, props->width, props->height) != 0) {
        tinyui_button_dispose_partial(button);
        return 0;
    }
    if (props->style_class != 0
        && tinyui_widget_set_style_class(&button->widget, props->style_class) != 0) {
        tinyui_button_dispose_partial(button);
        return 0;
    }
    if (tinyui_widget_set_bg_color(&button->widget, props->bg_color) != 0
        || tinyui_widget_set_text_color(&button->widget, props->text_color) != 0
        || tinyui_widget_set_border_color(&button->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&button->widget, props->radius) != 0
        || tinyui_widget_set_padding(&button->widget, props->padding) != 0) {
        tinyui_button_dispose_partial(button);
        return 0;
    }
    if (tinyui_button_set_release_image(button, props->release_image) != 0
        || tinyui_button_set_press_image(button, props->press_image) != 0
        || tinyui_button_set_transparent(button, props->transparent) != 0
        || tinyui_button_set_checkable(button, props->checkable) != 0
        || tinyui_button_set_key_value(button, props->key_value) != 0
        || tinyui_button_set_pressed(button, props->pressed) != 0) {
        tinyui_button_dispose_partial(button);
        return 0;
    }

    return button;
}

static int tinyui_button_set_event(struct tinyui_button *button,
                                   tinyui_event_cb cb,
                                   void *user_data,
                                   int kind)
{
    if (button == 0) {
        return -1;
    }

    if (kind == 0) {
        button->on_pressed = cb;
        button->on_pressed_user_data = user_data;
    } else if (kind == 1) {
        button->on_released = cb;
        button->on_released_user_data = user_data;
    } else {
        return -1;
    }
    return 0;
}

/**
 * @brief Set text of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] text Text widget instance
 * @return -1 on failure
 */

int tinyui_button_set_text(struct tinyui_button *button, const char *text)
{
    if (button == 0 || text == 0) {
        return -1;
    }

    if (tinyui_widget_set_text(&button->widget, text) != 0) {
        return -1;
    }
    return tinyui_widget_set_backend_text(button->widget.backend_widget, text);
}

/**
 * @brief Get text of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_button_get_text(struct tinyui_button *button, const char **text)
{
    ldButton_t *ld_button;

    if (button == 0 || text == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *text = (const char *)ldButtonGetText(ld_button);
    return 0;
}

/**
 * @brief Set font of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_font(struct tinyui_button *button, const struct tinyui_font *font)
{
    ldButton_t *ld_button;
    arm_2d_font_t *resolved_font;

    if (button == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    if (tinyui_button_fail_next_set_font != 0) {
        tinyui_button_fail_next_set_font = 0;
        return -1;
    }

    resolved_font = tinyui_button_resolve_font(font);
    if (resolved_font == 0) {
        return -1;
    }

    ldButtonSetFont(ld_button, resolved_font);
    button->widget.font = font;
    return 0;
}

/**
 * @brief Get font of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int tinyui_button_get_font(struct tinyui_button *button, const struct tinyui_font **font)
{
    if (button == 0 || font == 0) {
        return -1;
    }

    *font = button->widget.font;
    return 0;
}

/**
 * @brief Set color of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_color(struct tinyui_button *button,
                            unsigned int release_color,
                            unsigned int press_color)
{
    ldButton_t *ld_button;

    if (button == 0 || release_color > 0xFFFFFFU || press_color > 0xFFFFFFU) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetColor(ld_button, (ldColor)release_color, (ldColor)press_color);
    button->widget.bg_color = release_color;
    button->widget.border_color = press_color;
    return 0;
}

/**
 * @brief Get release color of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_button_get_release_color(struct tinyui_button *button, unsigned int *rgb)
{
    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *rgb = button->widget.bg_color;
    return 0;
}

/**
 * @brief Get press color of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_button_get_press_color(struct tinyui_button *button, unsigned int *rgb)
{
    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *rgb = button->widget.border_color;
    return 0;
}

/**
 * @brief Set release image of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_button_set_release_image(struct tinyui_button *button,
                                    struct tinyui_image_source *source)
{
    ldButton_t *ld_button;

    if (button == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
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
 * @brief Set press image of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_button_set_press_image(struct tinyui_button *button,
                                  struct tinyui_image_source *source)
{
    ldButton_t *ld_button;

    if (button == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
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
 * @brief Set image of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] release_source release source
 * @param[in] press_source press source
 * @return -1 on failure
 */

int tinyui_button_set_image(struct tinyui_button *button,
                            struct tinyui_image_source *release_source,
                            struct tinyui_image_source *press_source)
{
    if (tinyui_button_set_release_image(button, release_source) != 0) {
        return -1;
    }
    return tinyui_button_set_press_image(button, press_source);
}

/**
 * @brief Set transparent of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] transparent transparent
 * @return -1 on failure
 */

int tinyui_button_set_transparent(struct tinyui_button *button, int transparent)
{
    ldButton_t *ld_button;

    if (button == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetTransparent(ld_button, transparent != 0);
    return 0;
}

/**
 * @brief Get transparent of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] transparent transparent
 * @return -1 on failure
 */

int tinyui_button_get_transparent(struct tinyui_button *button, int *transparent)
{
    ldButton_t *ld_button;

    if (button == 0 || transparent == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *transparent = ldButtonGetTransparent(ld_button) ? 1 : 0;
    return 0;
}

/**
 * @brief Set checkable of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] checkable checkable
 * @return -1 on failure
 */

int tinyui_button_set_checkable(struct tinyui_button *button, int checkable)
{
    ldButton_t *ld_button;

    if (button == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetCheckable(ld_button, checkable != 0);
    return 0;
}

/**
 * @brief Get checkable of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] checkable checkable
 * @return -1 on failure
 */

int tinyui_button_get_checkable(struct tinyui_button *button, int *checkable)
{
    ldButton_t *ld_button;

    if (button == 0 || checkable == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *checkable = ldButtonGetCheckable(ld_button) ? 1 : 0;
    return 0;
}

/**
 * @brief Set key value of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] key_value key value
 * @return -1 on failure
 */

int tinyui_button_set_key_value(struct tinyui_button *button, unsigned int key_value)
{
    ldButton_t *ld_button;

    if (button == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetKeyValue(ld_button, (uint32_t)key_value);
    return 0;
}

/**
 * @brief Get key value of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] key_value key value
 * @return -1 on failure
 */

int tinyui_button_get_key_value(struct tinyui_button *button, unsigned int *key_value)
{
    ldButton_t *ld_button;

    if (button == 0 || key_value == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *key_value = (unsigned int)ldButtonGetKeyValue(ld_button);
    return 0;
}

/**
 * @brief Set pressed of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] pressed Pressed state
 * @return -1 on failure
 */

int tinyui_button_set_pressed(struct tinyui_button *button, int pressed)
{
    ldButton_t *ld_button;

    if (button == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetPress(ld_button, pressed != 0);
    return 0;
}

/**
 * @brief Set press of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] pressed Pressed state
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_press(struct tinyui_button *button, int pressed)
{
    return tinyui_button_set_pressed(button, pressed);
}

/**
 * @brief Get pressed of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] pressed Pressed state
 * @return -1 on failure
 */

int tinyui_button_get_pressed(struct tinyui_button *button, int *pressed)
{
    ldButton_t *ld_button;

    if (button == 0 || pressed == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *pressed = ldButtonGetPress(ld_button) ? 1 : 0;
    return 0;
}

/**
 * @brief Get press of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] pressed Pressed state
 * @return The property value, negative on error
 */

int tinyui_button_get_press(struct tinyui_button *button, int *pressed)
{
    return tinyui_button_get_pressed(button, pressed);
}

/**
 * @brief Get pressed by name id of button widget
 *
 * @param[in] root root
 * @param[in] name_id Name identifier ID
 * @param[in] pressed Pressed state
 * @return -1 on failure
 */

int tinyui_button_get_pressed_by_name_id(const struct tinyui_widget *root,
                                         int name_id,
                                         int *pressed)
{
    struct tinyui_widget *widget;

    if (root == 0 || pressed == 0) {
        return -1;
    }

    widget = tinyui_widget_find_by_name_id(root, name_id);
    if (widget == 0 || tinyui_widget_get_type(widget) != TINYUI_WIDGET_TYPE_BUTTON) {
        return -1;
    }

    return tinyui_button_get_pressed((struct tinyui_button *)widget, pressed);
}

/**
 * @brief Get action state by name id of button widget
 *
 * @param[in] root root
 * @param[in] name_id Name identifier ID
 * @param[in] action action
 * @return -1 on failure
 */

int tinyui_button_get_action_state_by_name_id(const struct tinyui_widget *root,
                                              int name_id,
                                              enum tinyui_button_action_state action)
{
    struct tinyui_widget *widget;

    if (root == 0 || name_id < 0 || name_id > 65535) {
        return -1;
    }

    widget = tinyui_widget_find_by_name_id(root, name_id);
    if (widget == 0 || tinyui_widget_get_type(widget) != TINYUI_WIDGET_TYPE_BUTTON) {
        return -1;
    }

    return (int)xBtnGetState((uint16_t)name_id, (uint8_t)action);
}

/**
 * @brief Set text color of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] text_color Text color
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_text_color(struct tinyui_button *button, unsigned int text_color)
{
    ldButton_t *ld_button;

    if (button == 0 || text_color > 0xFFFFFFU) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetTextColor(ld_button,
                         (ldColor)__RGB((text_color >> 16) & 0xFFU,
                                        (text_color >> 8) & 0xFFU,
                                        text_color & 0xFFU));
    button->widget.text_color = text_color;
    return 0;
}

/**
 * @brief Get text color of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_button_get_text_color(struct tinyui_button *button, unsigned int *rgb)
{
    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = tinyui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *rgb = button->widget.text_color;
    return 0;
}

/**
 * @brief Set on clicked of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_on_clicked(struct tinyui_button *button,
                                 tinyui_event_cb cb,
                                 void *user_data)
{
    if (button == 0) {
        return -1;
    }

    button->on_clicked = cb;
    button->user_data = user_data;
    return 0;
}

/**
 * @brief Set on pressed of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_on_pressed(struct tinyui_button *button,
                                 tinyui_event_cb cb,
                                 void *user_data)
{
    return tinyui_button_set_event(button, cb, user_data, 0);
}

/**
 * @brief Set on released of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_on_released(struct tinyui_button *button,
                                  tinyui_event_cb cb,
                                  void *user_data)
{
    return tinyui_button_set_event(button, cb, user_data, 1);
}
