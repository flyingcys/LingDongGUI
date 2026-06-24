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

/*
 * window-internal helpers shared between window.c and the layout/ sources
 * (flex.c / grid.c). These are NOT part of the public TinyUI API; they only
 * exist so the flex/grid layout implementation can live under src/layout/
 * while still resolving the underlying ld window and re-syncing padding.
 */
#ifndef TINYUI_WINDOW_INTERNAL_H
#define TINYUI_WINDOW_INTERNAL_H

#include "internal.h"
#include "../../../src/gui/ldWindow.h"

/* Resolve the underlying ldWindow for a window/background host (validates
 * kind + ld widgetType); returns 0 if the host is not a live window. */
ldWindow_t *tinyui_window_ld_of(struct tinyui_window *window);

/* Re-apply the window's padding to the ld window after a layout change. */
void tinyui_window_sync_padding(struct tinyui_window *window);

#endif /* TINYUI_WINDOW_INTERNAL_H */
