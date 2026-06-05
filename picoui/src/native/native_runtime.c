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

#include "picoui/runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct picoui_runtime_state {
    int initialized;
    int ready_logged;
    int auto_quit_enabled;
    unsigned long auto_quit_ms;
    clock_t start_ticks;
};

static struct picoui_runtime_state g_picoui_runtime;

static unsigned long picoui_runtime_parse_auto_quit_ms(void)
{
    const char *value = getenv("PICOUI_DEMO_AUTO_QUIT_MS");
    char *end = NULL;
    unsigned long parsed;

    if (value == NULL || value[0] == '\0') {
        return 0UL;
    }

    parsed = strtoul(value, &end, 10);
    if (end == value || (end != NULL && *end != '\0')) {
        return 0UL;
    }

    if (parsed > 60000UL) {
        parsed = 60000UL;
    }

    return parsed;
}

static unsigned long picoui_runtime_elapsed_ms(clock_t start_ticks, clock_t now_ticks)
{
    clock_t delta_ticks;

    if (now_ticks <= start_ticks) {
        return 0UL;
    }

    delta_ticks = now_ticks - start_ticks;
    return (unsigned long)(((double)delta_ticks * 1000.0) / (double)CLOCKS_PER_SEC);
}

int picoui_init(void)
{
    g_picoui_runtime.initialized = 1;
    g_picoui_runtime.ready_logged = 0;
    g_picoui_runtime.auto_quit_ms = picoui_runtime_parse_auto_quit_ms();
    g_picoui_runtime.auto_quit_enabled = g_picoui_runtime.auto_quit_ms > 0UL;
    g_picoui_runtime.start_ticks = clock();
    return 0;
}

void picoui_deinit(void)
{
    g_picoui_runtime.initialized = 0;
    g_picoui_runtime.ready_logged = 0;
    g_picoui_runtime.auto_quit_enabled = 0;
    g_picoui_runtime.auto_quit_ms = 0UL;
    g_picoui_runtime.start_ticks = 0;
}

int picoui_timer_handler(void)
{
    unsigned long elapsed_ms;

    if (!g_picoui_runtime.initialized) {
        return -1;
    }

    if (!g_picoui_runtime.ready_logged) {
        printf("PICOUI_RUNTIME_READY\n");
        fflush(stdout);
        g_picoui_runtime.ready_logged = 1;
    }

    if (!g_picoui_runtime.auto_quit_enabled) {
        return 0;
    }

    elapsed_ms = picoui_runtime_elapsed_ms(g_picoui_runtime.start_ticks, clock());
    if (elapsed_ms >= g_picoui_runtime.auto_quit_ms) {
        return 1;
    }

    return 0;
}
