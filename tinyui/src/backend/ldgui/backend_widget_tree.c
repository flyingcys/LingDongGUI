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
#include "ldBase.h"

void ldBaseNodeRemove(arm_2d_control_node_t *ptNode);

/**
 * @brief Check is kind of widget
 *
 * @param[in] backend_widget backend widget
 * @param[in] kind kind
 * @return 0 on success
 */

int picoui_backend_widget_is_kind(const void *backend_widget,
                                  enum picoui_backend_widget_kind kind)
{
    const struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return 0;
    }

    return widget->kind == kind;
}

/**
 * @brief widget: unbind host
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_unbind_host(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return -1;
    }

    if (widget->ld_widget != NULL) {
        ((ldBase_t *)widget->ld_widget)->pInfo = NULL;
    }
    widget->host_widget = NULL;
    widget->ld_event_bridge_scene = NULL;
    widget->ld_event_bridge_sender = NULL;
    widget->ld_event_bridge_next = NULL;
    return 0;
}

int picoui_backend_widget_detach_from_parent(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return -1;
    }

    if (widget->ld_widget != NULL) {
        ldBaseNodeRemove((arm_2d_control_node_t *)widget->ld_widget);
    }

    return picoui_widget_backend_detach(widget);
}
