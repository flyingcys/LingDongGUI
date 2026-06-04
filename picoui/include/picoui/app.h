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

#ifndef PICOUI_APP_H
#define PICOUI_APP_H

struct picoui_app;
struct picoui_app_timer;
struct picoui_background;
struct picoui_window;

typedef void (*picoui_app_timer_cb_t)(struct picoui_app *app,
                                      struct picoui_app_timer *timer,
                                      void *user_data);

/**
 * @brief Create app instance
 *
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_app *picoui_app_create(void);

/**
 * @brief Run application event loop
 *
 * @param[in] app Application instance
 * @param[in] window Window instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_run(struct picoui_app *app, struct picoui_window *window);

/**
 * @brief Run application in background mode
 *
 * @param[in] app Application instance
 * @param[in] background Background widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_run_background(struct picoui_app *app, struct picoui_background *background);

/**
 * @brief Set window of app
 *
 * @param[in] app Application instance
 * @param[in] window Window instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_set_window(struct picoui_app *app, struct picoui_window *window);

/**
 * @brief Set background of app
 *
 * @param[in] app Application instance
 * @param[in] background Background widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_set_background(struct picoui_app *app, struct picoui_background *background);

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
                             unsigned int duration_ms);

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
                                 unsigned int duration_ms);

/**
 * @brief Create app timer instance
 *
 * Timer instances are owned by the application passed to this function.
 * When picoui_app_destroy(app) is called, all timers associated with that app
 * are invalidated and cleaned up by the app lifecycle.
 *
 * @param[in] app Application instance
 * @return Timer instance on success, NULL on failure
 */

struct picoui_app_timer *picoui_app_timer_create(struct picoui_app *app);

/**
 * @brief Start or restart app timer
 *
 * @param[in] timer Timer instance
 * @param[in] interval_ms Interval in milliseconds
 * @param[in] repeat Repeat flag
 * @param[in] callback Timer callback
 * @param[in] user_data User data passed to callback. Ownership remains with
 * the caller; PicoUI stores the raw pointer and passes it back unchanged when
 * the callback is invoked.
 * @return 0 on success, -1 on failure
 */

int picoui_app_timer_start(struct picoui_app_timer *timer,
                           unsigned int interval_ms,
                           int repeat,
                           picoui_app_timer_cb_t callback,
                           void *user_data);

/**
 * @brief Stop app timer
 *
 * @param[in] timer Timer instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_timer_stop(struct picoui_app_timer *timer);

/**
 * @brief Query app timer running state
 *
 * @param[in] timer Timer instance
 * @return 1 if running, 0 otherwise
 */

int picoui_app_timer_is_running(const struct picoui_app_timer *timer);

/**
 * @brief Destroy app timer
 *
 * A timer may be destroyed explicitly before its owning app is destroyed.
 * After picoui_app_destroy(app), associated timers are already invalidated and
 * must not be used again.
 * After picoui_app_timer_destroy(timer) returns, the timer handle is
 * immediately invalid and must not be passed to start/stop/destroy again.
 *
 * @param[in] timer Timer instance
 */

void picoui_app_timer_destroy(struct picoui_app_timer *timer);

/**
 * @brief Destroy app instance
 *
 * @param[in] app Application instance
 */

void picoui_app_destroy(struct picoui_app *app);

#endif
