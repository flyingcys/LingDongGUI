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

#ifndef PICOUI_RUNTIME_H
#define PICOUI_RUNTIME_H

#include "../widget.h"

struct picoui_window;

int picoui_init(void);
void picoui_deinit(void);
struct picoui_window *picoui_screen_create(void);
int picoui_screen_load(struct picoui_window *screen);
void picoui_timer_handler(void);

#define tinyui_init picoui_init
#define tinyui_deinit picoui_deinit
#define tinyui_timer_handler picoui_timer_handler

static inline tinyui_obj_t *tinyui_screen_create(void)
{
    return (tinyui_obj_t *)picoui_screen_create();
}

static inline int tinyui_screen_load(tinyui_obj_t *screen)
{
    return picoui_screen_load((struct picoui_window *)screen);
}

#endif
