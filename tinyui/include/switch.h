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

#include "obj.h"
#include "picoui/switch.h"

static inline int tinyui_switch_set_checked(tinyui_obj_t *sw, int checked)
{
    return picoui_switch_set_checked((struct picoui_switch *)sw, checked);
}

static inline int tinyui_switch_is_checked(tinyui_obj_t *sw)
{
    return picoui_switch_is_checked((struct picoui_switch *)sw);
}

static inline int tinyui_switch_set_on_toggled(tinyui_obj_t *sw,
                                               tinyui_value_changed_cb cb,
                                               void *user_data)
{
    return picoui_switch_set_on_toggled((struct picoui_switch *)sw, cb, user_data);
}

#endif
