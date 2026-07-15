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


static struct tinyui_button *tinyui_button_as_button(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_BUTTON)) {
        return 0;
    }
    return (struct tinyui_button *)w;
}

static const struct tinyui_button *tinyui_button_as_button_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_BUTTON)) {
        return 0;
    }
    return (const struct tinyui_button *)w;
}

static int tinyui_button_fail_next_set_font = 0;

/* ── dispose machinery: rollback-on-create-failure path ─────────────────
 * Mirrors the arc/gauge pattern.  The depose callback receives only the
 * raw ld widget pointer, so the scene needed by ldButton_depose is parked
 * in a file-static between rollback and destroy_common.
 *
 * The function tinyui_button_rollback is the rollback entry called
 * from tinyui_button_create_with_props on prop-application failure; it
 * removes the action_info first (xBtnRemove must run while the ld widget
 * is still alive) and then defers to tinyui_runtime_internal_widget_destroy_common, which
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
        tinyui_runtime_internal_widget_destroy_common(&button->widget);
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

static int tinyui_button_props_are_valid(const tinyui_button_props_t *props)
{
    return props != 0
        && (props->release_image == 0 || tinyui_image_source_get_image_tile(props->release_image) != 0)
        && (props->press_image == 0 || tinyui_image_source_get_image_tile(props->press_image) != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_runtime_internal_button_ld_init(void *ctx,
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

static struct tinyui_button *tinyui_button_alloc(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_button *button;

    if (parent == 0 || id == 0) {
        return 0;
    }

    if (((struct tinyui_widget *)(void *)parent)->ld_widget == 0 || ((struct tinyui_widget *)(void *)parent)->owner == 0) {
        return 0;
    }

    button = (struct tinyui_button *)tinyui_runtime_internal_widget_create_leaf(parent,
                                                               TINYUI_BACKEND_WIDGET_BUTTON,
                                                               tinyui_runtime_internal_button_ld_init,
                                                               0,
                                                               sizeof(*button));
    if (button == 0) {
        return 0;
    }

    button->id = id;
    button->widget.visible    = 1;
    button->widget.enabled    = 1;
    button->widget.host_cleanup = tinyui_button_host_cleanup;
    button->on_pressed_handle = 0U;
    button->on_released_handle = 0U;
    button->on_clicked_handle = 0U;
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

tinyui_obj_t *tinyui_button_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "button";
    if (parent_w == 0) { return 0; }

    struct tinyui_button *button = tinyui_button_alloc(parent_w, id);

    if (button == 0) {
        return 0;
    }
    if (tinyui_button_set_font(button, NULL) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    return (tinyui_obj_t *)button;
}

/**
 * @brief Create button widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_button_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_button_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_button *button;

    if (props == 0) {
        return tinyui_button_create(parent);
    }

    obj = tinyui_button_create(parent);
    if (obj == 0) {
        return 0;
    }
    button = (struct tinyui_button *)(void *)obj;

    if ((props->fields & TINYUI_BUTTON_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&button->widget, props->user_data) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&button->widget, props->style_class) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
        if ((props->fields & TINYUI_BUTTON_FIELD_WIDTH) != 0 || (props->fields & TINYUI_BUTTON_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&button->widget);
        int h = tinyui_runtime_internal_widget_get_height(&button->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_BUTTON_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_BUTTON_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&button->widget, w, h) != 0) {
            tinyui_button_rollback(button);
            return 0;
        }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_TEXT) != 0) {
    if (tinyui_button_set_text((tinyui_obj_t *)button, props->text) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_FONT) != 0) {
    if (tinyui_button_set_font((tinyui_obj_t *)button, props->font) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_BG_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_bg_color(&button->widget, props->bg_color) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_TEXT_COLOR) != 0) {
    if (tinyui_button_set_text_color((tinyui_obj_t *)button, props->text_color) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_BORDER_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_border_color(&button->widget, props->border_color) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_RADIUS) != 0) {
    if (tinyui_runtime_internal_widget_set_radius(&button->widget, props->radius) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_PADDING) != 0) {
    if (tinyui_runtime_internal_widget_set_padding(&button->widget, props->padding) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_RELEASE_IMAGE) != 0) {
    if (tinyui_button_set_release_image((tinyui_obj_t *)button, props->release_image) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_PRESS_IMAGE) != 0) {
    if (tinyui_button_set_press_image((tinyui_obj_t *)button, props->press_image) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_TRANSPARENT) != 0) {
    if (tinyui_button_set_transparent((tinyui_obj_t *)button, props->transparent) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_CHECKABLE) != 0) {
    if (tinyui_button_set_checkable((tinyui_obj_t *)button, props->checkable) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_KEY_VALUE) != 0) {
    if (tinyui_button_set_key_value((tinyui_obj_t *)button, props->key_value) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_PRESSED) != 0) {
    if (tinyui_button_set_pressed((tinyui_obj_t *)button, props->pressed) != 0) {
        tinyui_button_rollback(button);
        return 0;
    }
    }
    if ((props->fields & TINYUI_BUTTON_FIELD_ON_CLICKED) != 0) {
        /* props still carry legacy tinyui_event_cb; Task 6 set_on_* is unified-pool only.
         * Legacy props callback shape is not expressible without a second event system —
         * reject rather than fake-success. Prefer tinyui_obj_add_event_cb after create. */
        if (props->on_clicked != 0) {
            tinyui_button_rollback(button);
            return 0;
        }
    }

    return obj;
}

static int tinyui_button_replace_event_cb(struct tinyui_button *button,
                                          tinyui_event_handle_t *slot,
                                          uint32_t event_mask,
                                          tinyui_event_cb_t cb,
                                          void *user_data)
{
    tinyui_event_handle_t handle = 0U;
    tinyui_result_t rc;

    if (button == 0 || slot == 0) {
        return -1;
    }

    if (*slot != 0U) {
        (void)tinyui_obj_remove_event_cb((tinyui_obj_t *)button, *slot);
        *slot = 0U;
    }

    if (cb == 0) {
        return 0;
    }

    rc = tinyui_obj_add_event_cb((tinyui_obj_t *)button,
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
 * @brief Set text of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] text Text widget instance
 * @return -1 on failure
 */

int tinyui_button_set_text(tinyui_obj_t *button_obj, const char *text)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    if (button == 0 || text == 0) {
        return -1;
    }

    return tinyui_runtime_internal_widget_set_text(&button->widget, text);
}

/**
 * @brief Get text of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_button_get_text(tinyui_obj_t *button_obj, const char **text)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_set_font(tinyui_obj_t *button_obj, const struct tinyui_font *font)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_get_font(tinyui_obj_t *button_obj, const struct tinyui_font **font)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_set_color(tinyui_obj_t *button_obj, unsigned int release_color, unsigned int press_color)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    ldButton_t *ld_button;

    if (button == 0 || release_color > 0xFFFFFFU || press_color > 0xFFFFFFU) {
        return -1;
    }

    ld_button = (ldButton_t *)button->widget.ld_widget;
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetColor(ld_button,
                     (ldColor)tinyui_rgb_to_ld_color(release_color),
                     (ldColor)tinyui_rgb_to_ld_color(press_color));
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

int tinyui_button_get_release_color(tinyui_obj_t *button_obj, unsigned int *rgb)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = (ldButton_t *)button->widget.ld_widget;
    if (ld_button == 0) {
        return -1;
    }

    *rgb = tinyui_ld_color_to_rgb((unsigned int)ldButtonGetReleaseColor(ld_button));
    return 0;
}

/**
 * @brief Get press color of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_button_get_press_color(tinyui_obj_t *button_obj, unsigned int *rgb)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = (ldButton_t *)button->widget.ld_widget;
    if (ld_button == 0) {
        return -1;
    }

    *rgb = tinyui_ld_color_to_rgb((unsigned int)ldButtonGetPressColor(ld_button));
    return 0;
}

/**
 * @brief Set release image of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_button_set_release_image(tinyui_obj_t *button_obj, struct tinyui_image_source *source)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_set_press_image(tinyui_obj_t *button_obj, struct tinyui_image_source *source)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_set_image(tinyui_obj_t *button_obj, struct tinyui_image_source *release_source, struct tinyui_image_source *press_source)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    if (tinyui_button_set_release_image((tinyui_obj_t *)button, release_source) != 0) {
        return -1;
    }
    return tinyui_button_set_press_image((tinyui_obj_t *)button, press_source);
}

/**
 * @brief Set transparent of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] transparent transparent
 * @return -1 on failure
 */

int tinyui_button_set_transparent(tinyui_obj_t *button_obj, int transparent)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_get_transparent(tinyui_obj_t *button_obj, int *transparent)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_set_checkable(tinyui_obj_t *button_obj, int checkable)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_get_checkable(tinyui_obj_t *button_obj, int *checkable)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_set_key_value(tinyui_obj_t *button_obj, unsigned int key_value)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_get_key_value(tinyui_obj_t *button_obj, unsigned int *key_value)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_set_pressed(tinyui_obj_t *button_obj, int pressed)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_runtime_internal_button_set_press(tinyui_obj_t *button_obj, int pressed)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    return tinyui_button_set_pressed((tinyui_obj_t *)button, pressed);
}

/**
 * @brief Get pressed of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] pressed Pressed state
 * @return -1 on failure
 */

int tinyui_button_get_pressed(tinyui_obj_t *button_obj, int *pressed)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_runtime_internal_button_get_press(tinyui_obj_t *button_obj, int *pressed)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    return tinyui_button_get_pressed((tinyui_obj_t *)button, pressed);
}

/**
 * @brief Get pressed by name id of button widget
 *
 * @param[in] root root
 * @param[in] name_id Name identifier ID
 * @param[in] pressed Pressed state
 * @return -1 on failure
 */

int tinyui_button_get_pressed_by_name_id(const tinyui_obj_t *root_obj, int name_id, int *pressed)
{
    const struct tinyui_widget *root = (const struct tinyui_widget *)(const void *)root_obj;
    if (root == 0) { return -1; }

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
    widget = ld_found != 0 ? tinyui_runtime_internal_widget_from_ld(ld_found) : 0;
    if (widget == 0) {
        return -1;
    }

    if (tinyui_runtime_internal_widget_get_type(widget) != TINYUI_WIDGET_TYPE_BUTTON) {
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

int tinyui_button_get_action_state_by_name_id(const tinyui_obj_t *root_obj, int name_id, enum tinyui_button_action_state action)
{
    const struct tinyui_widget *root = (const struct tinyui_widget *)(const void *)root_obj;
    if (root == 0) { return -1; }

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
    widget = ld_found != 0 ? tinyui_runtime_internal_widget_from_ld(ld_found) : 0;
    if (widget == 0) {
        return -1;
    }

    if (tinyui_runtime_internal_widget_get_type(widget) != TINYUI_WIDGET_TYPE_BUTTON) {
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

int tinyui_button_set_text_color(tinyui_obj_t *button_obj, unsigned int text_color)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

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

int tinyui_button_get_text_color(tinyui_obj_t *button_obj, unsigned int *rgb)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = (ldButton_t *)button->widget.ld_widget;
    if (ld_button == 0) {
        return -1;
    }

    *rgb = tinyui_ld_color_to_rgb((unsigned int)ldButtonGetTextColor(ld_button));
    return 0;
}

/**
 * @brief Set on clicked of button widget (narrow forward to unified event pool)
 *
 * @param[in] button Button widget instance
 * @param[in] cb Unified event callback (const tinyui_event_t *)
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_on_clicked(tinyui_obj_t *button_obj, tinyui_event_cb_t cb, void *user_data)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    return tinyui_button_replace_event_cb(button,
                                          &button->on_clicked_handle,
                                          TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                          cb,
                                          user_data);
}

/**
 * @brief Set on pressed of button widget (narrow forward to unified event pool)
 *
 * @param[in] button Button widget instance
 * @param[in] cb Unified event callback (const tinyui_event_t *)
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_on_pressed(tinyui_obj_t *button_obj, tinyui_event_cb_t cb, void *user_data)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    return tinyui_button_replace_event_cb(button,
                                          &button->on_pressed_handle,
                                          TINYUI_EVENT_MASK(TINYUI_EVENT_PRESSED),
                                          cb,
                                          user_data);
}

/**
 * @brief Set on released of button widget (narrow forward to unified event pool)
 *
 * @param[in] button Button widget instance
 * @param[in] cb Unified event callback (const tinyui_event_t *)
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_button_set_on_released(tinyui_obj_t *button_obj, tinyui_event_cb_t cb, void *user_data)
{
    struct tinyui_button *button = tinyui_button_as_button(button_obj);
    if (button == 0) { return -1; }

    return tinyui_button_replace_event_cb(button,
                                          &button->on_released_handle,
                                          TINYUI_EVENT_MASK(TINYUI_EVENT_RELEASED),
                                          cb,
                                          user_data);
}
