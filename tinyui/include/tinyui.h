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

#ifndef TINYUI_H
#define TINYUI_H

#include "tinyui_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* core */
#include "core/result.h"
#include "core/obj.h"
#include "core/runtime.h"
#include "core/event.h"
#include "core/timer.h"
#include "core/focus.h"

/* style / theme / layout / resource */
#include "style/style.h"
#if TINYUI_ENABLE_THEME
#include "theme/theme.h"
#endif
#include "layout/layout.h"
#include "resource/font.h"
#include "resource/image_source.h"

/* widgets */
#if TINYUI_ENABLE_ANIMATION
#include "widgets/animation.h"
#endif
#if TINYUI_ENABLE_ARC
#include "widgets/arc.h"
#endif
#if TINYUI_ENABLE_BACKGROUND
#include "widgets/background.h"
#endif
#if TINYUI_ENABLE_BUTTON
#include "widgets/button.h"
#endif
#if TINYUI_ENABLE_CALENDAR
#include "widgets/calendar.h"
#endif
#if TINYUI_ENABLE_CANVAS
#include "widgets/canvas.h"
#endif
#if TINYUI_ENABLE_CHECKBOX
#include "widgets/checkbox.h"
#endif
#if TINYUI_ENABLE_CLOCK
#include "widgets/clock.h"
#endif
#if TINYUI_ENABLE_COMBO_BOX
#include "widgets/combo_box.h"
#endif
#if TINYUI_ENABLE_DATE_TIME
#include "widgets/date_time.h"
#endif
#if TINYUI_ENABLE_GAUGE
#include "widgets/gauge.h"
#endif
#if TINYUI_ENABLE_GRAPH
#include "widgets/graph.h"
#endif
#if TINYUI_ENABLE_ICON_SLIDER
#include "widgets/icon_slider.h"
#endif
#if TINYUI_ENABLE_IMAGE
#include "widgets/image.h"
#endif
#if TINYUI_ENABLE_KEYBOARD
#include "widgets/keyboard.h"
#endif
#if TINYUI_ENABLE_LABEL
#include "widgets/label.h"
#endif
#if TINYUI_ENABLE_LINE_EDIT
#include "widgets/line_edit.h"
#endif
#if TINYUI_ENABLE_LIST
#include "widgets/list.h"
#endif
#if TINYUI_ENABLE_MESSAGE_BOX
#include "widgets/message_box.h"
#endif
#if TINYUI_ENABLE_PROGRESS_BAR
#include "widgets/progress_bar.h"
#endif
#if TINYUI_ENABLE_PROGRESS_WHEEL
#include "widgets/progress_wheel.h"
#endif
#if TINYUI_ENABLE_QRCODE
#include "widgets/qrcode.h"
#endif
#if TINYUI_ENABLE_RADIAL_MENU
#include "widgets/radial_menu.h"
#endif
#if TINYUI_ENABLE_SCROLL_SELECTOR
#include "widgets/scroll_selector.h"
#endif
#if TINYUI_ENABLE_SLIDER
#include "widgets/slider.h"
#endif
#if TINYUI_ENABLE_SWITCH
#include "widgets/switch.h"
#endif
#if TINYUI_ENABLE_TABLE
#include "widgets/table.h"
#endif
#if TINYUI_ENABLE_TEXT
#include "widgets/text.h"
#endif
#if TINYUI_ENABLE_WINDOW
#include "widgets/window.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* TINYUI_H */
