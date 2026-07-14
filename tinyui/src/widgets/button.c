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
#include "widgets/button.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/misc/xBtnAction.h"

#include <string.h>

static int tinyui_button_fail_next_set_font = 0;

/* ── dispose machinery: rollback-on-create-failure path ─────────────────
 * Mirrors the arc/gauge pattern.  The depose callback receives only the
 * raw ld widget pointer, so the scene needed by ldButton_depose is parked
 * in a file-static between rollback and destroy_common.
 *
 * The function tinyui_button_rollback is the rollback entry called
 * from tinyui_button_create_with_props on prop-application failure; it
 * removes the action_info first (xBtnRemove must run while the ld widget
 * is still alive) and then defers to tinyui_widget_destroy_common, which
 * detaches the ld node, clears pInfo, invokes the depose cb and frees the
 * host struct in a single sweep. */



static arm_2d_font_t *tinyui_button_default_font(void)
{
    return tinyui_resolve_ld_font(0, 12);
}

static arm_2d_font_t *tinyui_button_resolve_font(const struct tinyui_font *font)
{
    return font == 0 ? tinyui_button_default_font() : tinyui_resolve_ld_font(font, 12);
}

static void tinyui_button_rollback(struct tinyui_button *button)
{
    if (button == 0) {
        return;
    }

    if (button->widget.ld_widget != 0) {
        /* xBtnRemove must run while the ld widget (and its name_id) is still
         * registered with xBtnAction, before destroy_common detaches it. */
        xBtnRemove(&button->action_info);
        tinyui_widget_destroy_common(&button->widget);
    } else {
        ldFree(button);
    }
}

/* Runtime-destroy hook: ldButton_depose does NOT call xBtnRemove, so the host
 * must unlink its embedded action_info from the global xBtnLink before its
 * memory is freed. Invoked by destroy_common / subtree reclaim / app teardown
 * while the ld widget is still alive (mirrors keyboard host_cleanup). */
static void tinyui_button_host_cleanup(struct tinyui_widget *w)
{
    xBtnRemove(&((struct tinyui_button *)w)->action_info);
}

void tinyui_button_test_fail_next_set_font(void)
{
    tinyui_button_fail_next_set_font = 1;
}

static int tinyui_button_props_are_valid(const struct tinyui_button_props *props)
{
    return props != 0
        && props->id != 0
        && (props->release_image == 0 || tinyui_image_source_get_image_tile(props->release_image) != 0)
        && (props->press_image == 0 || tinyui_image_source_get_image_tile(props->press_image) != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_button_ld_init(void *ctx,
                                   struct ld_scene_t *scene,
                                   uint16_t name_id,
                                   uint16_t parent_name_id)
{
    (void)ctx;
    return ldButton_init(scene,
                         NULL,
                         name_id,
                         parent_name_id,
                         0,
                         0,
                         160,
                         36);
}

static struct tinyui_button *tinyui_button_alloc(struct tinyui_window *parent, const char *id)
{
    struct tinyui_button *button;

    if (parent == 0 || id == 0) {
        return 0;
    }

    if (parent->widget.ld_widget == 0 || parent->widget.owner == 0) {
        return 0;
    }

    button = (struct tinyui_button *)tinyui_widget_create_leaf(&parent->widget,
                                                               TINYUI_BACKEND_WIDGET_BUTTON,
                                                               tinyui_button_ld_init,
                                                               0,
                                                               sizeof(*button));
    if (button == 0) {
        return 0;
    }

    button->id = id;
    button->widget.visible    = 1;
    button->widget.enabled    = 1;
    button->widget.host_cleanup = tinyui_button_host_cleanup;
    _xBtnInit(button->widget.ld_name_id, (isBtnPressFunc)ldButtonActionIsPressById, &button->action_info);

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
    struct tinyui_button *button = tinyui_button_alloc(parent, id);

    if (button == 0) {
        return 0;
    }
    if (tinyui_button_set_font(button, NULL) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    return button;
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
        tinyui_button_rollback(button);
        return 0;
    }
    if (props->text != 0 && tinyui_button_set_text(button, props->text) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    if (props->font != 0 && tinyui_button_set_font(button, props->font) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && tinyui_widget_set_size(&button->widget, props->width, props->height) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    if (props->style_class != 0
        && tinyui_widget_set_style_class(&button->widget, props->style_class) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    if (tinyui_widget_set_bg_color(&button->widget, props->bg_color) != 0
        || tinyui_widget_set_text_color(&button->widget, props->text_color) != 0
        || tinyui_widget_set_border_color(&button->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&button->widget, props->radius) != 0
        || tinyui_widget_set_padding(&button->widget, props->padding) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    if (tinyui_button_set_release_image(button, props->release_image) != 0
        || tinyui_button_set_press_image(button, props->press_image) != 0
        || tinyui_button_set_transparent(button, props->transparent) != 0
        || tinyui_button_set_checkable(button, props->checkable) != 0
        || tinyui_button_set_key_value(button, props->key_value) != 0
        || tinyui_button_set_pressed(button, props->pressed) != 0) {
        tinyui_button_rollback(button);
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
    return tinyui_widget_set_backend_text(&button->widget, text);
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    if (button == 0 || (source != 0 && tinyui_image_source_get_image_tile(source) == 0)) {
        return -1;
    }

    ld_button = (ldButton_t *)button->widget.ld_widget;
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetImage(ld_button,
                     source != NULL ? tinyui_image_source_get_image_tile(source) : NULL,
                     source != NULL ? tinyui_image_source_get_mask_tile(source) : NULL,
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

    if (button == 0 || (source != 0 && tinyui_image_source_get_image_tile(source) == 0)) {
        return -1;
    }

    ld_button = (ldButton_t *)button->widget.ld_widget;
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetImage(ld_button,
                     ld_button->ptReleaseImgTile,
                     ld_button->ptReleaseMaskTile,
                     source != NULL ? tinyui_image_source_get_image_tile(source) : NULL,
                     source != NULL ? tinyui_image_source_get_mask_tile(source) : NULL);
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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
    ldBase_t *ld_found;
    struct tinyui_widget *widget;

    if (root == 0 || pressed == 0 || name_id <= 0 || name_id > 65535) {
        return -1;
    }

    if (root->ld_widget == 0) {
        return -1;
    }

    ld_found = (ldBase_t *)ldBaseGetWidget(
        (arm_2d_control_node_t *)root->ld_widget, (uint16_t)name_id);
    widget = ld_found != 0 ? tinyui_widget_from_ld(ld_found) : 0;
    if (widget == 0) {
        return -1;
    }

    if (tinyui_widget_get_type(widget) != TINYUI_WIDGET_TYPE_BUTTON) {
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
    ldBase_t *ld_found;
    struct tinyui_widget *widget;

    if (root == 0 || name_id <= 0 || name_id > 65535) {
        return -1;
    }

    if (root->ld_widget == 0) {
        return -1;
    }

    ld_found = (ldBase_t *)ldBaseGetWidget(
        (arm_2d_control_node_t *)root->ld_widget, (uint16_t)name_id);
    widget = ld_found != 0 ? tinyui_widget_from_ld(ld_found) : 0;
    if (widget == 0) {
        return -1;
    }

    if (tinyui_widget_get_type(widget) != TINYUI_WIDGET_TYPE_BUTTON) {
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetTextColor(ld_button,
                         (ldColor)tinyui_rgb_to_ld_color(text_color));
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

    ld_button = (ldButton_t *)button->widget.ld_widget;
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
