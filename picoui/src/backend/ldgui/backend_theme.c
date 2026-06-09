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

#include "internal.h"
#include "runtime_bridge.h"

#include <stdlib.h>

/**
 * @brief apply: theme
 *
 * @param[in] app Application instance
 * @param[in] theme Theme instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_apply_theme(struct picoui_app *app, struct picoui_theme *theme)
{
    if (app == 0 || theme == 0) {
        return -1;
    }

    return picoui_runtime_bridge_bind_theme(app, theme);
}
