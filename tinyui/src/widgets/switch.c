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
#include "widgets/switch.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldSwitch.h"

#include <stdlib.h>

/* ---- C2 depose closure state (file-static, single-threaded scope) ---- */


static int tinyui_switch_nav_dir_to_ld(int direction, int *ld_dir)
{
    if (ld_dir == 0) {
        return -1;
    }

    switch (direction) {
    case 1:
        *ld_dir = tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_UP);
        return 0;
    case 2:
        *ld_dir = tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_DOWN);
        return 0;
    case 3:
        *ld_dir = tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_LEFT);
        return 0;
    case 4:
        *ld_dir = tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_RIGHT);
        return 0;
    default:
        return -1;
    }
}

static int tinyui_switch_props_are_valid(const struct tinyui_switch_props *props)
{
    return props != 0
        && props->id != 0
        && (props->off_source == 0 || props->off_source->img_tile != 0)
        && (props->on_source == 0 || props->on_source->img_tile != 0)
        && (props->knob_source == 0 || props->knob_source->img_tile != 0)
        && (props->horizontal == -1 || props->horizontal == 0 || props->horizontal == 1)
        && (props->direction == -1 || (props->direction >= 0 && props->direction <= 2))
        && (props->disabled == -1 || props->disabled == 0 || props->disabled == 1)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_switch_ld_init(void *ctx,
                                   struct ld_scene_t *scene,
                                   uint16_t name_id,
                                   uint16_t parent_name_id)
{
    ldSwitch_t *ld_switch;

    (void)ctx;
    ld_switch = ldSwitch_init(scene, 0, name_id, parent_name_id, 0, 0, 48, 24);
    if (ld_switch == 0) {
        return 0;
    }
    return ld_switch;
}

/**
 * @brief Create switch widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_switch *tinyui_switch_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_switch *sw;

    if (parent == 0 || id == 0) {
        return 0;
    }
    sw = (struct tinyui_switch *)tinyui_widget_create_leaf(&parent->widget,
                                                           TINYUI_BACKEND_WIDGET_SWITCH,
                                                           tinyui_switch_ld_init,
                                                           0,
                                                           sizeof(*sw));
    if (sw == 0) {
        return 0;
    }
    sw->id = id;

    return sw;
}

/**
 * @brief Create switch widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_switch *tinyui_switch_create_with_props(struct tinyui_window *parent,
                                                      const struct tinyui_switch_props *props)
{
    struct tinyui_switch *sw;

    if (!tinyui_switch_props_are_valid(props)) {
        return 0;
    }

    sw = tinyui_switch_create(parent, props->id);
    if (sw == 0) {
        return 0;
    }

    sw->checked = props->checked != 0;
    sw->cb = 0;
    sw->user_data = 0;
    if (tinyui_widget_update_value(&sw->widget,
                                   sw->checked,
                                   0,
                                   0) != 0
        || tinyui_widget_set_user_data(&sw->widget, props->user_data) != 0
        || (props->style_class != 0
            && tinyui_widget_set_style_class(&sw->widget, props->style_class) != 0)
        || ((props->width > 0 || props->height > 0)
            && tinyui_widget_set_size(&sw->widget, props->width, props->height) != 0)
        || tinyui_widget_set_bg_color(&sw->widget, props->bg_color) != 0
        || tinyui_widget_set_text_color(&sw->widget, props->text_color) != 0
        || tinyui_widget_set_border_color(&sw->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&sw->widget, props->radius) != 0
        || tinyui_widget_set_padding(&sw->widget, props->padding) != 0
        || (props->off_source != 0
            && tinyui_switch_set_off_source(sw, props->off_source) != 0)
        || (props->on_source != 0
            && tinyui_switch_set_on_source(sw, props->on_source) != 0)
        || (props->knob_source != 0
            && tinyui_switch_set_knob_source(sw, props->knob_source) != 0)
        || (props->horizontal != -1
            && tinyui_switch_set_horizontal(sw, props->horizontal) != 0)
        || (props->direction != -1
            && tinyui_switch_set_direction(sw, props->direction) != 0)
        || (props->disabled != -1
            && tinyui_switch_set_disabled(sw, props->disabled) != 0)) {
        tinyui_widget_destroy_common(&sw->widget);
        return 0;
    }
    sw->cb = props->on_toggled;
    sw->user_data = props->user_data;
    return sw;
}

/**
 * @brief Set checked of switch widget
 *
 * @param[in] sw sw
 * @param[in] checked Checked state
 * @return 0 on success, -1 on failure
 */

int tinyui_switch_set_checked(struct tinyui_switch *sw, int checked)
{
    int normalized_checked;

    if (sw == 0) {
        return -1;
    }

    normalized_checked = checked != 0;
    if (sw->checked == normalized_checked) {
        return 0;
    }

    if (sw->widget.ld_widget == 0) {
        return -1;
    }

    sw->checked = normalized_checked;
    return tinyui_widget_update_value(&sw->widget,
                                      sw->checked,
                                      sw->cb,
                                      sw->user_data);
}

/**
 * @brief switch is checked
 *
 * @param[in] sw sw
 * @return 0 on success
 */

int tinyui_switch_is_checked(struct tinyui_switch *sw)
{
    if (sw == 0) {
        return 0;
    }

    return sw->checked;
}

/**
 * @brief Set off source of switch widget
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_switch_set_off_source(struct tinyui_switch *sw, struct tinyui_image_source *source)
{
    ldSwitch_t *ld_switch;

    if (sw == 0 || (source != 0 && source->img_tile == 0)
        || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    ld_switch = (ldSwitch_t *)sw->widget.ld_widget;
    ldSwitchSetImage(ld_switch,
                     source != 0 ? source->img_tile : 0,
                     source != 0 ? source->mask_tile : 0,
                     ld_switch->ptOnImgTile,
                     ld_switch->ptOnMaskTile,
                     ld_switch->ptKnobImgTile,
                     ld_switch->ptKnobMaskTile);
    return 0;
}

/**
 * @brief Set on source of switch widget
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_switch_set_on_source(struct tinyui_switch *sw, struct tinyui_image_source *source)
{
    ldSwitch_t *ld_switch;

    if (sw == 0 || (source != 0 && source->img_tile == 0)
        || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    ld_switch = (ldSwitch_t *)sw->widget.ld_widget;
    ldSwitchSetImage(ld_switch,
                     ld_switch->ptOffImgTile,
                     ld_switch->ptOffMaskTile,
                     source != 0 ? source->img_tile : 0,
                     source != 0 ? source->mask_tile : 0,
                     ld_switch->ptKnobImgTile,
                     ld_switch->ptKnobMaskTile);
    return 0;
}

/**
 * @brief Set knob source of switch widget
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_switch_set_knob_source(struct tinyui_switch *sw, struct tinyui_image_source *source)
{
    ldSwitch_t *ld_switch;

    if (sw == 0 || (source != 0 && source->img_tile == 0)
        || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    ld_switch = (ldSwitch_t *)sw->widget.ld_widget;
    ldSwitchSetImage(ld_switch,
                     ld_switch->ptOffImgTile,
                     ld_switch->ptOffMaskTile,
                     ld_switch->ptOnImgTile,
                     ld_switch->ptOnMaskTile,
                     source != 0 ? source->img_tile : 0,
                     source != 0 ? source->mask_tile : 0);
    return 0;
}

/**
 * @brief Set horizontal of switch widget
 *
 * @param[in] sw sw
 * @param[in] horizontal horizontal
 * @return -1 on failure
 */

int tinyui_switch_set_horizontal(struct tinyui_switch *sw, int horizontal)
{
    if (sw == 0 || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    ldSwitchSetHorizontal((ldSwitch_t *)sw->widget.ld_widget, horizontal != 0);
    return 0;
}

/**
 * @brief Get horizontal of switch widget
 *
 * @param[out] sw sw
 * @param[in] horizontal horizontal
 * @return -1 on failure
 */

int tinyui_switch_get_horizontal(struct tinyui_switch *sw, int *horizontal)
{
    if (sw == 0 || horizontal == 0 || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    *horizontal = ldSwitchIsHorizontal((ldSwitch_t *)sw->widget.ld_widget) ? 1 : 0;
    return 0;
}

/**
 * @brief Set direction of switch widget
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @return -1 on failure
 */

int tinyui_switch_set_direction(struct tinyui_switch *sw, int direction)
{
    if (sw == 0 || direction < 0 || direction > 2
        || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    ldSwitchSetDirection((ldSwitch_t *)sw->widget.ld_widget, (ldSwitchDirection_t)direction);
    return 0;
}

/**
 * @brief Get direction of switch widget
 *
 * @param[out] sw sw
 * @param[in] direction direction
 * @return -1 on failure
 */

int tinyui_switch_get_direction(struct tinyui_switch *sw, int *direction)
{
    if (sw == 0 || direction == 0 || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    *direction = (int)ldSwitchGetDirection((ldSwitch_t *)sw->widget.ld_widget);
    return 0;
}

/**
 * @brief Set disabled of switch widget
 *
 * @param[in] sw sw
 * @param[in] disabled disabled
 * @return -1 on failure
 */

int tinyui_switch_set_disabled(struct tinyui_switch *sw, int disabled)
{
    if (sw == 0) {
        return -1;
    }

    return tinyui_widget_set_enabled(&sw->widget, disabled == 0);
}

/**
 * @brief Get disabled of switch widget
 *
 * @param[out] sw sw
 * @param[in] disabled disabled
 * @return -1 on failure
 */

int tinyui_switch_get_disabled(struct tinyui_switch *sw, int *disabled)
{
    if (sw == 0 || disabled == 0 || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    *disabled = ldSwitchIsDisabled((ldSwitch_t *)sw->widget.ld_widget) ? 1 : 0;
    return 0;
}

/**
 * @brief switch can navigate
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @param[in] can_navigate can navigate
 * @return -1 on failure
 */

int tinyui_switch_can_navigate(struct tinyui_switch *sw, int direction, int *can_navigate)
{
    int ld_dir;

    if (sw == 0 || can_navigate == 0 || direction < 1 || direction > 4
        || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    if (tinyui_switch_nav_dir_to_ld(direction, &ld_dir) != 0) {
        return -1;
    }

    *can_navigate = ldSwitchCanNavigate((ldSwitch_t *)sw->widget.ld_widget,
                                        (ldNavDir_t)ld_dir) ? 1 : 0;
    return 0;
}

/**
 * @brief switch navigate
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @return -1 on failure
 */

int tinyui_switch_navigate(struct tinyui_switch *sw, int direction)
{
    struct tinyui_app *app_state;
    ldSwitch_t *ld_switch;
    int ld_dir;

    if (sw == 0 || direction < 1 || direction > 4
        || sw->widget.ld_widget == 0
        || sw->widget.kind != TINYUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    if (tinyui_switch_nav_dir_to_ld(direction, &ld_dir) != 0) {
        return -1;
    }

    app_state = sw->widget.owner;
    if (app_state == 0 || app_state->ld_scene == 0) {
        return -1;
    }

    ld_switch = (ldSwitch_t *)sw->widget.ld_widget;
    ldSwitchNavigate(app_state->ld_scene, ld_switch, (ldNavDir_t)ld_dir);
    sw->checked = ldSwitchIsChecked(ld_switch) ? 1 : 0;
    sw->widget.value = sw->checked;
    return 0;
}

/**
 * @brief Set on toggled of switch widget
 *
 * @param[in] sw sw
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_switch_set_on_toggled(struct tinyui_switch *sw,
                                 tinyui_value_changed_cb cb,
                                 void *user_data)
{
    if (sw == 0) {
        return -1;
    }

    sw->cb = cb;
    sw->user_data = user_data;
    return 0;
}
