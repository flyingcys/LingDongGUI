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
 * TinyUI demo entry — sole runtime/host runner (M4 Task 2).
 *
 * Lifecycle (canonical):
 *   parse name/display size → tinyui_init → display defaults →
 *   SDL test host (window/mouse) → screen_create → demos_build →
 *   screen_load(NONE, 0) → loop tinyui_process(&next_ms) → reverse deinit.
 *
 * Select demo at runtime:
 *   ./tinyui_demo arc_basic
 *   ./tinyui_demo hello_world
 * No argument: hello_world.
 */

#include "tinyui_demos.h"
#include "tinyui.h"
#include "display/display.h"
#include "tinyui_sdl.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

int tinyui_display_set_default_config(const struct tinyui_display_config *config);

static uint32_t tinyui_demo_parse_auto_quit_ms(void)
{
    const char *value = getenv("TINYUI_DEMO_AUTO_QUIT_MS");
    char *end = NULL;
    unsigned long parsed;

    if (value == NULL || value[0] == '\0') {
        return 0U;
    }
    parsed = strtoul(value, &end, 10);
    if (end == value || (end != NULL && *end != '\0')) {
        return 0U;
    }
    if (parsed > 0xFFFFFFFFUL) {
        return 0xFFFFFFFFU;
    }
    return (uint32_t)parsed;
}

int main(int argc, char **argv)
{
    const char *demo_name = (argc > 1) ? argv[1] : NULL;
    uint16_t width = 0;
    uint16_t height = 0;
    struct tinyui_display_config display_config = {0};
    tinyui_obj_t *screen = NULL;
    tinyui_result_t result;
    uint32_t next_ms = 0U;
    uint32_t auto_quit_ms;
    uint32_t start_ticks;
    int exit_code = 1;

    /* 1) Parse name + display metadata (default: hello_world). */
    if (!tinyui_demos_get_display_size(demo_name, &width, &height)) {
        tinyui_demos_show_help();
        return 1;
    }

    /* 2) Canonical runtime init (SDL host needs a live app). */
    if (tinyui_init() != TINYUI_OK) {
        return 1;
    }

    display_config.width = (int)width;
    display_config.height = (int)height;
    display_config.color_format = TINYUI_COLOR_FORMAT_RGB565;
    display_config.buffer_height = 0;
    display_config.user_data = NULL;
    if (tinyui_display_set_default_config(&display_config) != 0) {
        tinyui_demos_show_help();
        tinyui_deinit();
        return 1;
    }

    /* 3) SDL test host — window + input only; frame loop stays in core. */
    if (tinyui_sdl_window_create((int)width, (int)height) != 0 ||
        tinyui_sdl_mouse_create() != 0) {
        tinyui_sdl_quit();
        tinyui_deinit();
        return 1;
    }

    /* 4) Create screen, build UI intent, load with no transition. */
    screen = tinyui_screen_create();
    if (screen == NULL) {
        tinyui_sdl_quit();
        tinyui_deinit();
        return 1;
    }

    result = tinyui_demos_build(demo_name, screen);
    if (result != TINYUI_OK) {
        /* Task 2: unmigrated builders return NOT_SUPPORTED; keep runner linked. */
        fprintf(stderr,
                "tinyui_demo: build '%s' failed (%d)\n",
                (demo_name != NULL) ? demo_name : "hello_world",
                (int)result);
        tinyui_sdl_quit();
        tinyui_deinit();
        return 1;
    }

    if (tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) != TINYUI_OK) {
        tinyui_sdl_quit();
        tinyui_deinit();
        return 1;
    }

    /* 5) Process loop. Host clean-exit is via auto-quit env and/or SDL quit
     * after input pump; backend failures fail the process. */
    auto_quit_ms = tinyui_demo_parse_auto_quit_ms();
    start_ticks = SDL_GetTicks();

    for (;;) {
        result = tinyui_process(&next_ms);
        if (result != TINYUI_OK) {
            exit_code = 1;
            break;
        }
        if (auto_quit_ms > 0U &&
            (SDL_GetTicks() - start_ticks) >= auto_quit_ms) {
            exit_code = 0;
            break;
        }
        /* Best-effort interactive close if QUIT remains queued. */
        if (SDL_QuitRequested()) {
            exit_code = 0;
            break;
        }
        (void)next_ms;
    }

    /* 6) Reverse deinit. */
    tinyui_sdl_quit();
    tinyui_deinit();
    return exit_code;
}
