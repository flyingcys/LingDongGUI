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

#ifndef TINYUI_SWITCH_H
#define TINYUI_SWITCH_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_SWITCH
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_SWITCH is disabled"
#  endif
#endif

#include "core/obj.h"
#include "core/event.h"
#include <stdint.h>

#ifndef TINYUI_LEGACY_VALUE_CHANGED_CB_DEFINED
#define TINYUI_LEGACY_VALUE_CHANGED_CB_DEFINED
/* Legacy widget-style callback kept for props/compat; set_on_* uses tinyui_event_cb_t. */
typedef void (*tinyui_value_changed_cb)(tinyui_obj_t *obj, int value, void *user_data);
#endif


struct tinyui_image_source;

typedef enum tinyui_switch_field {
    TINYUI_SWITCH_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_SWITCH_FIELD_CHECKED = UINT32_C(1) << 1,
    TINYUI_SWITCH_FIELD_ON_TOGGLED = UINT32_C(1) << 2,
    TINYUI_SWITCH_FIELD_USER_DATA = UINT32_C(1) << 3,
    TINYUI_SWITCH_FIELD_STYLE_CLASS = UINT32_C(1) << 4,
    TINYUI_SWITCH_FIELD_WIDTH = UINT32_C(1) << 5,
    TINYUI_SWITCH_FIELD_HEIGHT = UINT32_C(1) << 6,
    TINYUI_SWITCH_FIELD_BG_COLOR = UINT32_C(1) << 7,
    TINYUI_SWITCH_FIELD_TEXT_COLOR = UINT32_C(1) << 8,
    TINYUI_SWITCH_FIELD_BORDER_COLOR = UINT32_C(1) << 9,
    TINYUI_SWITCH_FIELD_RADIUS = UINT32_C(1) << 10,
    TINYUI_SWITCH_FIELD_PADDING = UINT32_C(1) << 11,
    TINYUI_SWITCH_FIELD_OFF_SOURCE = UINT32_C(1) << 12,
    TINYUI_SWITCH_FIELD_ON_SOURCE = UINT32_C(1) << 13,
    TINYUI_SWITCH_FIELD_KNOB_SOURCE = UINT32_C(1) << 14,
    TINYUI_SWITCH_FIELD_HORIZONTAL = UINT32_C(1) << 15,
    TINYUI_SWITCH_FIELD_DIRECTION = UINT32_C(1) << 16,
    TINYUI_SWITCH_FIELD_DISABLED = UINT32_C(1) << 17,
} tinyui_switch_field_t;

typedef struct tinyui_switch_props {
    uint32_t fields;
    uint16_t id;
    int checked;
    tinyui_value_changed_cb on_toggled;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    struct tinyui_image_source *off_source;
    struct tinyui_image_source *on_source;
    struct tinyui_image_source *knob_source;
    int horizontal;
    int direction;
    int disabled;
} tinyui_switch_props_t;

tinyui_obj_t *tinyui_switch_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_switch_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_switch_props_t *props);

int tinyui_switch_set_checked(tinyui_obj_t *sw, int checked);

int tinyui_switch_is_checked(tinyui_obj_t *sw);

int tinyui_switch_set_off_source(tinyui_obj_t *sw, struct tinyui_image_source *source);

int tinyui_switch_set_on_source(tinyui_obj_t *sw, struct tinyui_image_source *source);

int tinyui_switch_set_knob_source(tinyui_obj_t *sw, struct tinyui_image_source *source);

int tinyui_switch_set_horizontal(tinyui_obj_t *sw, int horizontal);

int tinyui_switch_get_horizontal(tinyui_obj_t *sw, int *horizontal);

int tinyui_switch_set_direction(tinyui_obj_t *sw, int direction);

int tinyui_switch_get_direction(tinyui_obj_t *sw, int *direction);

int tinyui_switch_set_disabled(tinyui_obj_t *sw, int disabled);

int tinyui_switch_get_disabled(tinyui_obj_t *sw, int *disabled);

int tinyui_switch_can_navigate(tinyui_obj_t *sw, int direction, int *can_navigate);

int tinyui_switch_navigate(tinyui_obj_t *sw, int direction);

/* Narrow forward to tinyui_obj_add_event_cb (unified pool). Replace semantics. */
int tinyui_switch_set_on_toggled(tinyui_obj_t *sw, tinyui_event_cb_t cb, void *user_data);

#endif /* TINYUI_SWITCH_H */
