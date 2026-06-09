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

#ifndef TINYUI_BUTTON_H
#define TINYUI_BUTTON_H

#include "obj.h"
#include "picoui/button.h"

static inline tinyui_obj_t *tinyui_button_create(tinyui_obj_t *parent, const char *id)
{
    return (tinyui_obj_t *)picoui_button_create((struct picoui_window *)parent, id);
}

static inline int tinyui_button_set_text(tinyui_obj_t *button, const char *text)
{
    return picoui_button_set_text((struct picoui_button *)button, text);
}

static inline int tinyui_button_set_on_clicked(tinyui_obj_t *button,
                                               tinyui_event_cb cb,
                                               void *user_data)
{
    return picoui_button_set_on_clicked((struct picoui_button *)button, cb, user_data);
}

#endif
