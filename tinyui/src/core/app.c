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
#include "app.h"
#include "background.h"
#include "../../../src/misc/xBtnAction.h"

#include <stdlib.h>

static void tinyui_app_timer_unlink(struct picoui_app_timer *timer)
{
    struct picoui_app_timer **cursor;

    if (timer == NULL || timer->app == NULL) {
        return;
    }

    cursor = &timer->app->timers;
    while (*cursor != NULL) {
        if (*cursor == timer) {
            *cursor = timer->next;
            timer->next = NULL;
            return;
        }
        cursor = &(*cursor)->next;
    }
}

void tinyui_app_pump_timers(struct picoui_app *app, unsigned int now_ticks)
{
    struct picoui_app_timer_snapshot_entry {
        struct picoui_app_timer *timer;
        struct picoui_app_timer *expected_predecessor;
    };

    struct picoui_app_timer_snapshot_entry *snapshot;
    struct picoui_app_timer *timer;
    struct picoui_app_timer *previous_timer = NULL;
    size_t timer_count = 0;
    size_t index = 0;

    if (app == NULL) {
        return;
    }

    timer = app->timers;
    while (timer != NULL) {
        timer_count += 1U;
        timer = timer->next;
    }

    if (timer_count == 0U) {
        return;
    }

    snapshot = calloc(timer_count, sizeof(*snapshot));
    if (snapshot == NULL) {
        return;
    }

    timer = app->timers;
    while (timer != NULL && index < timer_count) {
        snapshot[index].timer = timer;
        snapshot[index].expected_predecessor = previous_timer;
        index += 1U;
        previous_timer = timer;
        timer = timer->next;
    }

    for (index = 0; index < timer_count; ++index) {
        int timer_is_linked = 0;
        struct picoui_app_timer *cursor;
        struct picoui_app_timer *current_predecessor = NULL;

        timer = snapshot[index].timer;
        cursor = app->timers;
        while (cursor != NULL) {
            if (cursor == timer) {
                timer_is_linked = 1;
                break;
            }
            current_predecessor = cursor;
            cursor = cursor->next;
        }

        if (!timer_is_linked) {
            continue;
        }

        if (current_predecessor != snapshot[index].expected_predecessor) {
            continue;
        }

        if (timer->running && timer->callback != NULL) {
            if (timer->next_fire_ticks == 0U) {
                timer->next_fire_ticks = now_ticks + timer->interval_ms;
            } else if (now_ticks >= timer->next_fire_ticks) {
                if (timer->repeat) {
                    timer->next_fire_ticks = now_ticks + timer->interval_ms;
                } else {
                    timer->running = 0;
                }
                timer->callback(app, timer, timer->user_data);
            }
        }
    }

    free(snapshot);
}

/**
 * @brief Create app instance
 *
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_app *picoui_app_create(void)
{
    struct picoui_app *app = calloc(1, sizeof(struct picoui_app));
    if (app == NULL) {
        return NULL;
    }

    if (tinyui_runtime_bridge_init_app(app) != 0) {
        free(app);
        return NULL;
    }

    return app;
}

/**
 * @brief Run application event loop
 *
 * @param[in] app Application instance
 * @param[in] window Window instance
 * @return -1 on failure
 */

int picoui_app_run(struct picoui_app *app, struct picoui_window *window)
{
    if (app == NULL || !tinyui_runtime_bridge_window_is_owned_by(app, window)) {
        return -1;
    }

    return tinyui_runtime_bridge_run_app(app, window);
}

/**
 * @brief Run application in background mode
 *
 * @param[in] app Application instance
 * @param[in] background Background widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_run_background(struct picoui_app *app, struct picoui_background *background)
{
    return picoui_app_run(app, (struct picoui_window *)background);
}

/**
 * @brief Set window of app
 *
 * @param[in] app Application instance
 * @param[in] window Window instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_set_window(struct picoui_app *app, struct picoui_window *window)
{
    if (app == NULL || !tinyui_runtime_bridge_window_is_owned_by(app, window)) {
        return -1;
    }

    app->root_window = window;
    app->focus_owner = 0;
    app->editing_owner = 0;
    tinyui_runtime_bridge_reset_window_switch(app);
    return 0;
}

/**
 * @brief Set background of app
 *
 * @param[in] app Application instance
 * @param[in] background Background widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_set_background(struct picoui_app *app, struct picoui_background *background)
{
    return picoui_app_set_window(app, (struct picoui_window *)background);
}

/**
 * @brief Switch application window
 *
 * @param[in] app Application instance
 * @param[in] window Window instance
 * @param[in] mode Operation mode
 * @param[in] duration_ms Duration in milliseconds
 * @return 0 on success, -1 on failure
 */

int picoui_app_switch_window(struct picoui_app *app,
                             struct picoui_window *window,
                             int mode,
                             unsigned int duration_ms)
{
    if (picoui_app_set_window(app, window) != 0) {
        return -1;
    }

    tinyui_runtime_bridge_set_window_switch(app, mode, duration_ms);
    return 0;
}

/**
 * @brief Switch application background window
 *
 * @param[in] app Application instance
 * @param[in] background Background widget instance
 * @param[in] mode Operation mode
 * @param[in] duration_ms Duration in milliseconds
 * @return 0 on success, -1 on failure
 */

int picoui_app_switch_background(struct picoui_app *app,
                                 struct picoui_background *background,
                                 int mode,
                                 unsigned int duration_ms)
{
    return picoui_app_switch_window(app, (struct picoui_window *)background, mode, duration_ms);
}

/**
 * @brief Create app timer instance
 *
 * @param[in] app Application instance
 * @return Timer instance on success, NULL on failure
 */

struct picoui_app_timer *picoui_app_timer_create(struct picoui_app *app)
{
    struct picoui_app_timer *timer;

    if (app == NULL) {
        return NULL;
    }

    timer = calloc(1, sizeof(struct picoui_app_timer));
    if (timer == NULL) {
        return NULL;
    }

    timer->app = app;
    timer->next = app->timers;
    app->timers = timer;
    return timer;
}

/**
 * @brief Start or restart app timer
 *
 * @param[in] timer Timer instance
 * @param[in] interval_ms Interval in milliseconds
 * @param[in] repeat Repeat flag
 * @param[in] callback Timer callback
 * @param[in] user_data User data passed to callback
 * @return 0 on success, -1 on failure
 */

int picoui_app_timer_start(struct picoui_app_timer *timer,
                           unsigned int interval_ms,
                           int repeat,
                           picoui_app_timer_cb_t callback,
                           void *user_data)
{
    if (timer == NULL || interval_ms == 0 || callback == NULL) {
        return -1;
    }

    timer->interval_ms = interval_ms;
    timer->repeat = repeat ? 1 : 0;
    timer->callback = callback;
    timer->user_data = user_data;
    timer->next_fire_ticks = 0;
    timer->running = 1;
    return 0;
}

/**
 * @brief Stop app timer
 *
 * @param[in] timer Timer instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_timer_stop(struct picoui_app_timer *timer)
{
    if (timer == NULL) {
        return -1;
    }

    timer->running = 0;
    return 0;
}

/**
 * @brief Query app timer running state
 *
 * @param[in] timer Timer instance
 * @return 1 if running, 0 otherwise
 */

int picoui_app_timer_is_running(const struct picoui_app_timer *timer)
{
    if (timer == NULL) {
        return 0;
    }

    return timer->running ? 1 : 0;
}

/**
 * @brief Destroy app timer
 *
 * @param[in] timer Timer instance
 */

void picoui_app_timer_destroy(struct picoui_app_timer *timer)
{
    if (timer == NULL) {
        return;
    }

    tinyui_app_timer_unlink(timer);
    free(timer);
}

/**
 * @brief Destroy app instance
 *
 * @param[in] app Application instance
 */

void picoui_app_destroy(struct picoui_app *app)
{
    struct picoui_app_timer *timer;
    struct picoui_app_timer *next;

    if (app == NULL) {
        return;
    }

    timer = app->timers;
    while (timer != NULL) {
        next = timer->next;
        free(timer);
        timer = next;
    }
    app->timers = NULL;

    tinyui_runtime_bridge_shutdown_app(app);
    xBtnDestroy();
    free(app);
}
