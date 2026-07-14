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

/* core */
#include "core/result.h"
#include "core/obj.h"
#include "core/runtime.h"
#include "core/event.h"
#include "core/timer.h"
#include "core/focus.h"

/* style / theme / layout / resource */
#include "style/style.h"
#include "theme/theme.h"
#include "layout/layout.h"
#include "resource/font.h"
#include "resource/image_source.h"

/* widgets */
#include "widgets/animation.h"
#include "widgets/arc.h"
#include "widgets/background.h"
#include "widgets/button.h"
#include "widgets/calendar.h"
#include "widgets/canvas.h"
#include "widgets/checkbox.h"
#include "widgets/clock.h"
#include "widgets/combo_box.h"
#include "widgets/date_time.h"
#include "widgets/gauge.h"
#include "widgets/graph.h"
#include "widgets/icon_slider.h"
#include "widgets/image.h"
#include "widgets/keyboard.h"
#include "widgets/label.h"
#include "widgets/line_edit.h"
#include "widgets/list.h"
#include "widgets/message_box.h"
#include "widgets/progress_bar.h"
#include "widgets/progress_wheel.h"
#include "widgets/qrcode.h"
#include "widgets/radial_menu.h"
#include "widgets/scroll_selector.h"
#include "widgets/slider.h"
#include "widgets/switch.h"
#include "widgets/table.h"
#include "widgets/text.h"
#include "widgets/window.h"

/* In-tree v2.2 demo/test migration bridge only. Install trees never define this. */
#if defined(TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE) && (TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE)
#include "internal/v22_demo_bridge.h"
#endif

#endif /* TINYUI_H */
