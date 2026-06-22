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

/**
 * @file runtime_internal.h
 * @brief Runtime/backend internal types shared across the framework.
 *
 * After Phase C3-T4 the legacy backend mirror type and the layout cache
 * structs have been deleted — real binding state now lives
 * folded onto struct tinyui_widget / struct tinyui_window directly.  What
 * remains here are the shared enums and constants that have no better home.
 */

#ifndef TINYUI_RUNTIME_INTERNAL_H
#define TINYUI_RUNTIME_INTERNAL_H

#include <stdint.h>

#include "widget.h"
#include "window.h"

struct ld_scene_t;

/* ── Constants ─────────────────────────────────────────────────── */

#define TINYUI_BACKEND_LAYOUT_MAX_TRACKS 16
#define TINYUI_BACKEND_LIST_MAX_ITEMS 16

/* ── Shared enums ──────────────────────────────────────────────── */

/* enum tinyui_backend_widget_kind is defined in internal.h when this file is
 * reached via that header (TINYUI_BACKEND_WIDGET_KIND_DEFINED is set there).
 * When runtime_internal.h is included directly (e.g. in unit tests that skip
 * internal.h), we define it here as a fallback. */
#ifndef TINYUI_BACKEND_WIDGET_KIND_DEFINED
#define TINYUI_BACKEND_WIDGET_KIND_DEFINED

enum tinyui_backend_widget_kind {
    TINYUI_BACKEND_WIDGET_WINDOW = 0,
    TINYUI_BACKEND_WIDGET_BACKGROUND,
    TINYUI_BACKEND_WIDGET_LABEL,
    TINYUI_BACKEND_WIDGET_BUTTON,
    TINYUI_BACKEND_WIDGET_CHECKBOX,
    TINYUI_BACKEND_WIDGET_SWITCH,
    TINYUI_BACKEND_WIDGET_SLIDER,
    TINYUI_BACKEND_WIDGET_ARC,
    TINYUI_BACKEND_WIDGET_GAUGE,
    TINYUI_BACKEND_WIDGET_ICON_SLIDER,
    TINYUI_BACKEND_WIDGET_RADIAL_MENU,
    TINYUI_BACKEND_WIDGET_PROGRESS_BAR,
    TINYUI_BACKEND_WIDGET_QRCODE,
    TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL,
    TINYUI_BACKEND_WIDGET_ANIMATION,
    TINYUI_BACKEND_WIDGET_LIST,
    TINYUI_BACKEND_WIDGET_MESSAGE_BOX,
    TINYUI_BACKEND_WIDGET_DATE_TIME,
    TINYUI_BACKEND_WIDGET_CLOCK,
    TINYUI_BACKEND_WIDGET_TEXT,
    TINYUI_BACKEND_WIDGET_KEYBOARD,
    TINYUI_BACKEND_WIDGET_COMBO_BOX,
    TINYUI_BACKEND_WIDGET_SCROLL_SELECTER,
    TINYUI_BACKEND_WIDGET_TABLE,
    TINYUI_BACKEND_WIDGET_GRAPH,
    TINYUI_BACKEND_WIDGET_IMAGE,
    TINYUI_BACKEND_WIDGET_CALENDAR,
    TINYUI_BACKEND_WIDGET_CANVAS,
};

#endif /* TINYUI_BACKEND_WIDGET_KIND_DEFINED */

enum tinyui_backend_signal {
    TINYUI_BACKEND_SIGNAL_NONE = 0,
    TINYUI_BACKEND_SIGNAL_VALUE_CHANGED,
    TINYUI_BACKEND_SIGNAL_PRESSED,
    TINYUI_BACKEND_SIGNAL_RELEASED,
};

#endif /* TINYUI_RUNTIME_INTERNAL_H */
