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
#include "runtime_bridge.h"
#include "ldSwitch.h"
#include "ldSwitchInternal.h"

#include <stdlib.h>

static ldSwitch_t *picoui_backend_switch_get_ld(struct picoui_switch *sw)
{
    struct picoui_backend_widget *backend;

    if (sw == NULL || sw->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_SWITCH || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldSwitch_t *)backend->ld_widget;
}

/**
 * @brief Create backend for switch
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_switch(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldSwitch_t *ld_switch;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_runtime_bridge_backend_state_from_parent(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(widget);
        return 0;
    }
    ld_switch = ldSwitch_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_widget->ld_name_id,
                              0,
                              0,
                              48,
                              24);
    if (ld_switch == NULL) {
        free(widget);
        return 0;
    }
    ldSwitchSetColor(ld_switch,
                     __RGB(224, 224, 224),
                     __RGB(33, 150, 243),
                     GLCD_COLOR_WHITE,
                     GLCD_COLOR_WHITE);

    if (picoui_backend_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_SWITCH,
                                         id,
                                         parent_widget->theme) != 0) {
        ldSwitch_depose(app_state->ld_scene, ld_switch);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_switch;
    widget->ld_name_id = name_id;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldSwitch_depose(app_state->ld_scene, ld_switch);
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set off source of switch backend
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_off_source(struct picoui_switch *sw, struct picoui_image_source *source)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL) {
        return -1;
    }

    ldSwitchSetImage(ld_switch,
                     source != NULL ? source->img_tile : NULL,
                     source != NULL ? source->mask_tile : NULL,
                     ld_switch->ptOnImgTile,
                     ld_switch->ptOnMaskTile,
                     ld_switch->ptKnobImgTile,
                     ld_switch->ptKnobMaskTile);
    return 0;
}

/**
 * @brief Set on source of switch backend
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_on_source(struct picoui_switch *sw, struct picoui_image_source *source)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL) {
        return -1;
    }

    ldSwitchSetImage(ld_switch,
                     ld_switch->ptOffImgTile,
                     ld_switch->ptOffMaskTile,
                     source != NULL ? source->img_tile : NULL,
                     source != NULL ? source->mask_tile : NULL,
                     ld_switch->ptKnobImgTile,
                     ld_switch->ptKnobMaskTile);
    return 0;
}

/**
 * @brief Set knob source of switch backend
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_knob_source(struct picoui_switch *sw, struct picoui_image_source *source)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL) {
        return -1;
    }

    ldSwitchSetImage(ld_switch,
                     ld_switch->ptOffImgTile,
                     ld_switch->ptOffMaskTile,
                     ld_switch->ptOnImgTile,
                     ld_switch->ptOnMaskTile,
                     source != NULL ? source->img_tile : NULL,
                     source != NULL ? source->mask_tile : NULL);
    return 0;
}

/**
 * @brief Set horizontal of switch backend
 *
 * @param[in] sw sw
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_horizontal(struct picoui_switch *sw, int horizontal)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL) {
        return -1;
    }

    ldSwitchSetHorizontal(ld_switch, horizontal != 0);
    return 0;
}

/**
 * @brief Get horizontal from switch backend
 *
 * @param[out] sw sw
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_get_horizontal(struct picoui_switch *sw, int *horizontal)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL || horizontal == NULL) {
        return -1;
    }

    *horizontal = ldSwitchIsHorizontal(ld_switch) ? 1 : 0;
    return 0;
}

/**
 * @brief Set direction of switch backend
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_direction(struct picoui_switch *sw, int direction)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL || direction < 0 || direction > 2) {
        return -1;
    }

    ldSwitchSetDirection(ld_switch, (ldSwitchDirection_t)direction);
    return 0;
}

/**
 * @brief Get direction from switch backend
 *
 * @param[out] sw sw
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_get_direction(struct picoui_switch *sw, int *direction)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL || direction == NULL) {
        return -1;
    }

    *direction = (int)ldSwitchGetDirection(ld_switch);
    return 0;
}

/**
 * @brief Set disabled of switch backend
 *
 * @param[in] sw sw
 * @param[in] disabled disabled
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_disabled(struct picoui_switch *sw, int disabled)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL) {
        return -1;
    }

    ldSwitchSetDisabled(ld_switch, disabled != 0);
    return 0;
}

/**
 * @brief Get disabled from switch backend
 *
 * @param[out] sw sw
 * @param[in] disabled disabled
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_get_disabled(struct picoui_switch *sw, int *disabled)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);

    if (ld_switch == NULL || disabled == NULL) {
        return -1;
    }

    *disabled = ldSwitchIsDisabled(ld_switch) ? 1 : 0;
    return 0;
}

/**
 * @brief switch: can navigate
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @param[in] can_navigate can navigate
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_can_navigate(struct picoui_switch *sw, int direction, int *can_navigate)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);
    int ld_dir;

    if (ld_switch == NULL || can_navigate == NULL) {
        return -1;
    }

    switch (direction) {
    case 1:
        ld_dir = picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_UP);
        break;
    case 2:
        ld_dir = picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_DOWN);
        break;
    case 3:
        ld_dir = picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_LEFT);
        break;
    case 4:
        ld_dir = picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_RIGHT);
        break;
    default:
        return -1;
    }

    *can_navigate = ldSwitchCanNavigate(ld_switch, (ldNavDir_t)ld_dir) ? 1 : 0;
    return 0;
}

/**
 * @brief switch: navigate
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_navigate(struct picoui_switch *sw, int direction)
{
    ldSwitch_t *ld_switch = picoui_backend_switch_get_ld(sw);
    struct picoui_backend_app_state *app_state;
    struct picoui_backend_widget *backend;
    int ld_dir;

    if (ld_switch == NULL || sw == NULL || sw->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    app_state = picoui_runtime_bridge_backend_state_from_parent(backend);
    if (app_state == NULL || app_state->ld_scene == NULL) {
        return -1;
    }

    switch (direction) {
    case 1:
        ld_dir = picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_UP);
        break;
    case 2:
        ld_dir = picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_DOWN);
        break;
    case 3:
        ld_dir = picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_LEFT);
        break;
    case 4:
        ld_dir = picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_RIGHT);
        break;
    default:
        return -1;
    }

    ldSwitchNavigate(app_state->ld_scene, ld_switch, (ldNavDir_t)ld_dir);
    sw->checked = ldSwitchIsChecked(ld_switch) ? 1 : 0;
    backend->value = sw->checked;
    return 0;
}
