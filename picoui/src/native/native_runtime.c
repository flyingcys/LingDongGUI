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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct picoui_runtime_state {
    int initialized;
    int ready_logged;
    int auto_quit_enabled;
    unsigned long auto_quit_ms;
    struct timespec start_ticks;
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

static int picoui_runtime_now(struct timespec *value)
{
    if (value == NULL) {
        return -1;
    }
#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, value) == 0) {
        return 0;
    }
#endif
    return clock_gettime(CLOCK_REALTIME, value);
}

static unsigned long picoui_runtime_elapsed_ms(struct timespec start_ticks, struct timespec now_ticks)
{
    time_t delta_sec;
    long delta_nsec;

    if (now_ticks.tv_sec < start_ticks.tv_sec
        || (now_ticks.tv_sec == start_ticks.tv_sec && now_ticks.tv_nsec <= start_ticks.tv_nsec)) {
        return 0UL;
    }

    delta_sec = now_ticks.tv_sec - start_ticks.tv_sec;
    delta_nsec = now_ticks.tv_nsec - start_ticks.tv_nsec;
    if (delta_nsec < 0) {
        delta_sec -= 1;
        delta_nsec += 1000000000L;
    }

    return (unsigned long)delta_sec * 1000UL + (unsigned long)(delta_nsec / 1000000L);
}

int picoui_init(void)
{
    struct timespec now = {0, 0};

    g_picoui_runtime.initialized = 1;
    g_picoui_runtime.ready_logged = 0;
    g_picoui_runtime.auto_quit_ms = picoui_runtime_parse_auto_quit_ms();
    g_picoui_runtime.auto_quit_enabled = g_picoui_runtime.auto_quit_ms > 0UL;
    if (picoui_runtime_now(&now) != 0) {
        g_picoui_runtime.initialized = 0;
        g_picoui_runtime.auto_quit_enabled = 0;
        g_picoui_runtime.auto_quit_ms = 0UL;
        return -1;
    }
    g_picoui_runtime.start_ticks = now;
    return 0;
}

void picoui_native_runtime_deinit(void)
{
    g_picoui_runtime.initialized = 0;
    g_picoui_runtime.ready_logged = 0;
    g_picoui_runtime.auto_quit_enabled = 0;
    g_picoui_runtime.auto_quit_ms = 0UL;
    g_picoui_runtime.start_ticks.tv_sec = 0;
    g_picoui_runtime.start_ticks.tv_nsec = 0;
}

int picoui_native_runtime_timer_handler(void)
{
    unsigned long elapsed_ms;
    struct timespec now = {0, 0};

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

    if (picoui_runtime_now(&now) != 0) {
        return -1;
    }

    elapsed_ms = picoui_runtime_elapsed_ms(g_picoui_runtime.start_ticks, now);
    if (elapsed_ms >= g_picoui_runtime.auto_quit_ms) {
        return 1;
    }

    return 0;
}
