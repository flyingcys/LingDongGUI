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

#include <SDL.h>
#include "arm_2d.h"
#include "arm_2d_helper.h"
#include "ldConfig.h"
#include "ldBase.h"
#include "ldGui.h"
#include "arm_2d_disp_adapter_0.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PICOUI_RUNTIME_PADDING 16
#define PICOUI_RUNTIME_ROW_HEIGHT 34
#define PICOUI_RUNTIME_ROW_GAP 10

/**
 * @brief   attribute  
 *
 * @param[in] (weak) (weak)
 */

__attribute__((weak)) void VT_enter_global_mutex(void) {}

/**
 * @brief   attribute  
 *
 * @param[in] (weak) (weak)
 */

__attribute__((weak)) void VT_leave_global_mutex(void) {}

/**
 * @brief   attribute  
 *
 * @param[in] (weak) (weak)
 */

__attribute__((weak)) void ldCfgTouchSetPoint(int16_t x, int16_t y, bool pressed)
{
    (void)x;
    (void)y;
    (void)pressed;
}

static int tinyui_runtime_host_touch_log_enabled(void)
{
    static int initialized = 0;
    static int enabled = 0;

    if (!initialized) {
        const char *env = getenv("PICOUI_TOUCH_LOG");
        enabled = (env != NULL && env[0] != '\0' && env[0] != '0') ? 1 : 0;
        initialized = 1;
    }

    return enabled;
}

static int tinyui_runtime_host_benchmark_log_enabled(void)
{
    static int initialized = 0;
    static int enabled = 0;

    if (!initialized) {
        const char *env = getenv("PICOUI_BENCHMARK_LOG");
        enabled = (env != NULL && env[0] != '\0' && env[0] != '0') ? 1 : 0;
        initialized = 1;
    }

    return enabled;
}

static void tinyui_runtime_host_runtime_bootstrap(void)
{
    static int initialized = 0;

    if (initialized) {
        return;
    }

    arm_2d_helper_init();
    initialized = 1;
}

static void tinyui_runtime_host_runtime_page_init(ld_scene_t *scene)
{
    (void)scene;
}

static void tinyui_runtime_host_runtime_page_quit(ld_scene_t *scene)
{
    (void)scene;
}

struct tinyui_runtime_host_state {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    COLOUR_INT *real_pixels;
    uint32_t *present_pixels;
    arm_2d_tile_t real_tile;
    Uint32 start_ticks;
    Uint32 screen_create_start_ticks;
    Uint32 screen_create_end_ticks;
    Uint32 auto_quit_ms;
    int display_width;
    int display_height;
    int ready_logged;
    int capture_written;
    int static_mapping_logged;
    int fallback_boundary_logged;
    int temporary_smoke_logged;
    int smoke_layout_used;
    int smoke_layout_marker_logged;
    int benchmark_screen_create_logged;
    int benchmark_first_frame_logged;
};

static void tinyui_runtime_host_log_screen_create_benchmark(struct tinyui_runtime_host_state *state)
{
    double elapsed_ms;

    if (state == NULL || state->benchmark_screen_create_logged ||
        !tinyui_runtime_host_benchmark_log_enabled()) {
        return;
    }

    if (state->screen_create_end_ticks < state->screen_create_start_ticks) {
        return;
    }

    elapsed_ms = (double)(state->screen_create_end_ticks - state->screen_create_start_ticks);
    printf("PICOUI_BENCHMARK_SCREEN_OBJECT_CREATE_MS=%.3f\n", elapsed_ms);
    printf("PICOUI_BENCHMARK_SCREEN_CREATE_MS=%.3f\n", elapsed_ms);
    fflush(stdout);
    state->benchmark_screen_create_logged = 1;
}

static void tinyui_runtime_host_log_first_frame_benchmark(struct tinyui_runtime_host_state *state)
{
    double elapsed_ms;

    if (state == NULL || state->benchmark_first_frame_logged ||
        !tinyui_runtime_host_benchmark_log_enabled()) {
        return;
    }

    if (state->capture_written == 0) {
        return;
    }

    elapsed_ms = (double)(SDL_GetTicks() - state->start_ticks);
    printf("PICOUI_BENCHMARK_CAPTURE_READY_MS=%.3f\n", elapsed_ms);
    printf("PICOUI_BENCHMARK_FIRST_FRAME_MS=%.3f\n", elapsed_ms);
    fflush(stdout);
    state->benchmark_first_frame_logged = 1;
}

static const ldPageFuncGroup_t g_tinyui_runtime_host_page = {
    .init = tinyui_runtime_host_runtime_page_init,
    .loop = NULL,
    .quit = tinyui_runtime_host_runtime_page_quit,
    .draw = NULL,
    .frameStart = NULL,
    .frameComplete = NULL,
#if (USE_LOG_LEVEL>=LOG_LEVEL_INFO)
    .pageName = "picoui_runtime",
#endif
    .pointer = NULL,
};

static int tinyui_runtime_host_widget_is_supported_real(const struct picoui_backend_widget *widget)
{
    if (widget == NULL) {
        return 1;
    }

    switch (widget->kind) {
    case PICOUI_BACKEND_WIDGET_BACKGROUND:
    case PICOUI_BACKEND_WIDGET_WINDOW:
    case PICOUI_BACKEND_WIDGET_LABEL:
    case PICOUI_BACKEND_WIDGET_BUTTON:
    case PICOUI_BACKEND_WIDGET_CHECKBOX:
    case PICOUI_BACKEND_WIDGET_TEXT:
    case PICOUI_BACKEND_WIDGET_IMAGE:
    case PICOUI_BACKEND_WIDGET_SWITCH:
    case PICOUI_BACKEND_WIDGET_SLIDER:
    case PICOUI_BACKEND_WIDGET_ARC:
    case PICOUI_BACKEND_WIDGET_GAUGE:
    case PICOUI_BACKEND_WIDGET_ICON_SLIDER:
    case PICOUI_BACKEND_WIDGET_RADIAL_MENU:
    case PICOUI_BACKEND_WIDGET_PROGRESS_BAR:
    case PICOUI_BACKEND_WIDGET_QRCODE:
    case PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL:
    case PICOUI_BACKEND_WIDGET_ANIMATION:
    case PICOUI_BACKEND_WIDGET_LIST:
    case PICOUI_BACKEND_WIDGET_COMBO_BOX:
    case PICOUI_BACKEND_WIDGET_SCROLL_SELECTER:
    case PICOUI_BACKEND_WIDGET_TABLE:
    case PICOUI_BACKEND_WIDGET_GRAPH:
    case PICOUI_BACKEND_WIDGET_CALENDAR:
    case PICOUI_BACKEND_WIDGET_DATE_TIME:
    case PICOUI_BACKEND_WIDGET_MESSAGE_BOX:
    case PICOUI_BACKEND_WIDGET_CLOCK:
    case PICOUI_BACKEND_WIDGET_KEYBOARD:
        return 1;
    default:
        return 0;
    }
}

static int tinyui_runtime_host_widget_is_real_mapped(const struct picoui_backend_widget *widget)
{
    return widget != NULL &&
           widget->kind != PICOUI_BACKEND_WIDGET_WINDOW &&
           widget->kind != PICOUI_BACKEND_WIDGET_BACKGROUND &&
           tinyui_runtime_host_widget_is_supported_real(widget) &&
           widget->ld_widget != NULL;
}

static void tinyui_runtime_host_append_id(const char *id,
                                     char *buffer,
                                     size_t buffer_size,
                                     size_t *used)
{
    int written;

    if (id == NULL || id[0] == '\0') {
        return;
    }

    if (*used > 0 && *used + 1 < buffer_size) {
        buffer[*used] = ',';
        *used += 1;
        buffer[*used] = '\0';
    }

    if (*used + 1 >= buffer_size) {
        return;
    }

    written = snprintf(buffer + *used, buffer_size - *used, "%s", id);
    if (written > 0) {
        size_t advance = (size_t)written;
        if (advance >= buffer_size - *used) {
            *used = buffer_size - 1;
        } else {
            *used += advance;
        }
    }
}

static int tinyui_runtime_host_widget_needs_fallback(const struct picoui_backend_widget *widget)
{
    return widget != NULL &&
           widget->kind != PICOUI_BACKEND_WIDGET_WINDOW &&
           widget->kind != PICOUI_BACKEND_WIDGET_BACKGROUND &&
           (!tinyui_runtime_host_widget_is_supported_real(widget) || widget->ld_widget == NULL);
}

static int tinyui_runtime_host_window_has_real_layout(const struct picoui_backend_widget *widget)
{
    ldWindow_t *ld_window;

    if (widget == NULL
        || (widget->kind != PICOUI_BACKEND_WIDGET_WINDOW
            && widget->kind != PICOUI_BACKEND_WIDGET_BACKGROUND)
        || widget->ld_widget == NULL) {
        return 0;
    }

    ld_window = (ldWindow_t *)widget->ld_widget;
    return ld_window->layoutTpye == layoutFlex || ld_window->layoutTpye == layoutGrid;
}

static int tinyui_runtime_host_widget_excludes_formal_mapping(const struct picoui_backend_widget *widget)
{
    while (widget != NULL) {
        if ((widget->runtime_evidence_flags & PICOUI_BACKEND_EVIDENCE_EXCLUDE_FORMAL_MAPPING) != 0U) {
            return 1;
        }
        if (widget->first_child != NULL && tinyui_runtime_host_widget_excludes_formal_mapping(widget->first_child)) {
            return 1;
        }
        widget = widget->next_sibling;
    }
    return 0;
}

static int tinyui_runtime_host_widget_allows_smoke_layout(const struct picoui_backend_widget *widget)
{
    while (widget != NULL) {
        if ((widget->runtime_evidence_flags & PICOUI_BACKEND_EVIDENCE_ALLOW_SMOKE_LAYOUT) != 0U) {
            return 1;
        }
        if (widget->first_child != NULL && tinyui_runtime_host_widget_allows_smoke_layout(widget->first_child)) {
            return 1;
        }
        widget = widget->next_sibling;
    }
    return 0;
}

static void tinyui_runtime_host_append_widget_ids(const struct picoui_backend_widget *widget,
                                             int (*predicate)(const struct picoui_backend_widget *widget),
                                             char *buffer,
                                             size_t buffer_size,
                                             size_t *used)
{
    while (widget != NULL) {
        if (predicate(widget) && widget->id != NULL && widget->id[0] != '\0') {
            tinyui_runtime_host_append_id(widget->id, buffer, buffer_size, used);
        }

        if (widget->first_child != NULL) {
            tinyui_runtime_host_append_widget_ids(widget->first_child,
                                             predicate,
                                             buffer,
                                             buffer_size,
                                             used);
        }

        widget = widget->next_sibling;
    }
}

static void tinyui_runtime_host_log_mapping_markers(struct tinyui_runtime_host_state *state,
                                               const struct picoui_backend_widget *root)
{
    char real_ids[256] = {0};
    char fallback_ids[256] = {0};
    size_t real_used = 0;
    size_t fallback_used = 0;

    if (state == NULL || root == NULL) {
        return;
    }

    tinyui_runtime_host_append_widget_ids(root->first_child,
                                     tinyui_runtime_host_widget_is_real_mapped,
                                     real_ids,
                                     sizeof(real_ids),
                                     &real_used);
    tinyui_runtime_host_append_widget_ids(root->first_child,
                                     tinyui_runtime_host_widget_needs_fallback,
                                     fallback_ids,
                                     sizeof(fallback_ids),
                                     &fallback_used);

    if (real_used > 0 &&
        !tinyui_runtime_host_widget_excludes_formal_mapping(root->first_child) &&
        !state->static_mapping_logged) {
        printf("PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI\n");
        printf("PICOUI_BACKEND_REAL_WIDGET_IDS=%s\n", real_ids);
        fflush(stdout);
        state->static_mapping_logged = 1;
    }

    if (tinyui_runtime_host_widget_excludes_formal_mapping(root->first_child) &&
        !state->temporary_smoke_logged) {
        printf("PICOUI_BACKEND_TEMPORARY_SMOKE_PATH=EXCLUDED_FORMAL_MAPPING\n");
        fflush(stdout);
        state->temporary_smoke_logged = 1;
    }

    if (fallback_used > 0 && !state->fallback_boundary_logged) {
        printf("PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK\n");
        printf("PICOUI_BACKEND_FALLBACK_WIDGET_IDS=%s\n", fallback_ids);
        fflush(stdout);
        state->fallback_boundary_logged = 1;
    }
}

static void tinyui_runtime_host_log_image_source_marker(const struct picoui_backend_widget *widget)
{
    while (widget != NULL) {
        if (widget->kind == PICOUI_BACKEND_WIDGET_IMAGE && widget->id != NULL && widget->ld_widget != NULL) {
            ldImage_t *ld_image = (ldImage_t *)widget->ld_widget;

            printf("PICOUI_BACKEND_IMAGE_SOURCE=%s:img=%s,mask=%s\n",
                   widget->id,
                   ld_image->ptImgTile != NULL ? "set" : "null",
                   ld_image->ptMaskTile != NULL ? "set" : "null");
        }

        if (widget->first_child != NULL) {
            tinyui_runtime_host_log_image_source_marker(widget->first_child);
        }

        widget = widget->next_sibling;
    }
}

static Uint32 tinyui_runtime_host_parse_auto_quit_ms(void)
{
    const char *value = getenv("PICOUI_DEMO_AUTO_QUIT_MS");
    char *end = NULL;
    unsigned long parsed;

    if (value == NULL || value[0] == '\0') {
        return 0U;
    }

    parsed = strtoul(value, &end, 10);
    if (end == value || (end != NULL && *end != '\0')) {
        return 0U;
    }

    if (parsed > 60000UL) {
        parsed = 60000UL;
    }
    return (Uint32)parsed;
}

static uint32_t tinyui_runtime_host_pixel_to_rgb888(COLOUR_INT pixel)
{
#if __DISP0_CFG_COLOUR_DEPTH__ == 16
    uint32_t red = ((uint32_t)pixel >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)pixel >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)pixel & 0x1FU;

    red = (red << 3) | (red >> 2);
    green = (green << 2) | (green >> 4);
    blue = (blue << 3) | (blue >> 2);
    return (red << 16) | (green << 8) | blue;
#elif __DISP0_CFG_COLOUR_DEPTH__ == 32
    return (uint32_t)pixel & 0x00FFFFFFU;
#elif __DISP0_CFG_COLOUR_DEPTH__ == 8
    return ((uint32_t)pixel << 16) | ((uint32_t)pixel << 8) | (uint32_t)pixel;
#else
    return 0U;
#endif
}

static uint32_t tinyui_runtime_host_pixel_to_argb8888(COLOUR_INT pixel)
{
    return 0xFF000000U | tinyui_runtime_host_pixel_to_rgb888(pixel);
}

static int tinyui_runtime_host_write_capture(struct tinyui_runtime_host_state *state)
{
    const char *path = getenv("PICOUI_CAPTURE_FILE");
    FILE *fp;
    int x;
    int y;

    if (state == NULL || state->real_pixels == NULL || state->capture_written) {
        return 0;
    }

    if (path == NULL || path[0] == '\0') {
        return 0;
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "P6\n%d %d\n255\n", state->display_width, state->display_height);
    for (y = 0; y < state->display_height; ++y) {
        for (x = 0; x < state->display_width; ++x) {
            COLOUR_INT pixel =
                state->real_pixels[(size_t)y * (size_t)state->display_width + (size_t)x];
            uint32_t rgb888 = tinyui_runtime_host_pixel_to_rgb888(pixel);
            unsigned char rgb[3];

            rgb[0] = (unsigned char)((rgb888 >> 16) & 0xFFU);
            rgb[1] = (unsigned char)((rgb888 >> 8) & 0xFFU);
            rgb[2] = (unsigned char)(rgb888 & 0xFFU);
            fwrite(rgb, 1, 3, fp);
        }
    }

    fclose(fp);
    state->capture_written = 1;
    tinyui_runtime_host_log_first_frame_benchmark(state);
    return 0;
}

static struct tinyui_runtime_host_state *tinyui_runtime_host_state_from_app(struct picoui_app *app)
{
    struct picoui_backend_app_state *app_state;

    app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state == NULL) {
        return NULL;
    }
    return (struct tinyui_runtime_host_state *)app_state->runtime_state;
}

static struct picoui_backend_app_state *tinyui_runtime_host_app_state_from_window(struct picoui_window *window)
{
    return tinyui_runtime_bridge_backend_state_from_window(window);
}

/**
 * @brief Initialize app backend
 *
 * @param[in] app Application instance
 * @return 0 on success, -1 on failure
 */

static void tinyui_runtime_host_present_real_frame(struct tinyui_runtime_host_state *state)
{
    int x;
    int y;

    if (state == NULL || state->renderer == NULL || state->texture == NULL || state->real_pixels == NULL
        || state->present_pixels == NULL) {
        return;
    }

    for (y = 0; y < state->display_height; ++y) {
        for (x = 0; x < state->display_width; ++x) {
            size_t index = (size_t)y * (size_t)state->display_width + (size_t)x;
            state->present_pixels[index] = tinyui_runtime_host_pixel_to_argb8888(state->real_pixels[index]);
        }
    }

    SDL_UpdateTexture(state->texture,
                      NULL,
                      state->present_pixels,
                      (int)(state->display_width * (int)sizeof(*state->present_pixels)));
    SDL_RenderCopy(state->renderer, state->texture, NULL, NULL);
}

static void tinyui_runtime_host_apply_real_widget_layout(struct tinyui_runtime_host_state *state,
                                                    const struct picoui_backend_widget *widget,
                                                    int x,
                                                    int *cursor_y)
{
    while (widget != NULL) {
        if (widget->kind != PICOUI_BACKEND_WIDGET_WINDOW
            && widget->kind != PICOUI_BACKEND_WIDGET_BACKGROUND) {
            int height = PICOUI_RUNTIME_ROW_HEIGHT;

            if (widget->kind == PICOUI_BACKEND_WIDGET_IMAGE) {
                height = 56;
            }

            if (tinyui_runtime_host_widget_is_supported_real(widget) && widget->ld_widget != NULL) {
                arm_2d_region_t region = ldBaseGetRegion((ldBase_t *)widget->ld_widget);
                region.tLocation.iX = (int16_t)x;
                region.tLocation.iY = (int16_t)(*cursor_y);
                ldBaseSetRegion((ldBase_t *)widget->ld_widget, region);
                height = region.tSize.iHeight > 0 ? region.tSize.iHeight : height;
            }

            *cursor_y += height + PICOUI_RUNTIME_ROW_GAP;
        }

        if (widget->first_child != NULL) {
            tinyui_runtime_host_apply_real_widget_layout(state, widget->first_child, x, cursor_y);
        }

        widget = widget->next_sibling;
    }
}

static void tinyui_runtime_host_apply_smoke_cursor_layout(struct tinyui_runtime_host_state *state,
                                                     const struct picoui_backend_widget *root,
                                                     int x,
                                                     int *cursor_y)
{
    if (root == NULL || root->first_child == NULL || tinyui_runtime_host_window_has_real_layout(root) ||
        !tinyui_runtime_host_widget_allows_smoke_layout(root->first_child)) {
        return;
    }

    /* Explicit opt-in only: keep temporary smoke layout out of generic non-layout demos. */
    state->smoke_layout_used = 1;
    tinyui_runtime_host_apply_real_widget_layout(state, root->first_child, x, cursor_y);
}

static void tinyui_runtime_host_render(struct tinyui_runtime_host_state *state, struct picoui_window *window)
{
    const struct picoui_backend_widget *root;
    const struct picoui_backend_widget *root_widget;
    struct picoui_backend_app_state *app_state;
    int x = PICOUI_RUNTIME_PADDING;
    int y = PICOUI_RUNTIME_PADDING + 20;

    SDL_SetRenderDrawColor(state->renderer, 0x2E, 0x34, 0x40, 0xFF);
    SDL_RenderClear(state->renderer);

    root = (const struct picoui_backend_widget *)window->widget.backend_widget;
    if (root != NULL && root->first_child != NULL) {
        root_widget = (const struct picoui_backend_widget *)window->widget.backend_widget;
        app_state = tinyui_runtime_host_app_state_from_window(window);
        tinyui_runtime_host_log_mapping_markers(state, root);
        if (app_state != NULL && app_state->ld_scene != NULL && state->real_pixels != NULL) {
            state->smoke_layout_used = 0;
            memset(state->real_pixels,
                   0,
                   (size_t)state->display_width * (size_t)state->display_height *
                       sizeof(*state->real_pixels));
            tinyui_runtime_host_apply_smoke_cursor_layout(state, root_widget, x, &y);
            if (!state->smoke_layout_marker_logged) {
                printf("PICOUI_SMOKE_LAYOUT_USED=%d\n", state->smoke_layout_used ? 1 : 0);
                fflush(stdout);
                state->smoke_layout_marker_logged = 1;
            }
            ldGuiFrameStart(app_state->ld_scene);
            ldGuiTouchProcess(app_state->ld_scene);
            ldMsgProcess(app_state->ld_scene);
            ldGuiDraw(app_state->ld_scene, &state->real_tile, true);
            ldGuiFrameComplete(app_state->ld_scene);
            tinyui_runtime_host_log_image_source_marker(root->first_child);
            tinyui_runtime_host_present_real_frame(state);
        }
    }

    SDL_RenderPresent(state->renderer);
    (void)tinyui_runtime_host_write_capture(state);
}

static int tinyui_runtime_host_prepare_runtime_state(struct picoui_app *app,
                                                     struct tinyui_runtime_host_state **state_out)
{
    struct tinyui_runtime_host_state *state;

    if (app == NULL || state_out == NULL) {
        return -1;
    }

    state = tinyui_runtime_host_state_from_app(app);
    if (state == NULL && tinyui_runtime_bridge_init_app(app) != 0) {
        return -1;
    }
    state = tinyui_runtime_host_state_from_app(app);
    if (state == NULL) {
        return -1;
    }

    *state_out = state;
    return 0;
}

static int tinyui_runtime_host_prepare_runtime_scene(struct picoui_app *app,
                                                     struct picoui_window *window,
                                                     struct picoui_backend_app_state **app_state_out)
{
    struct picoui_backend_app_state *app_state;

    if (app == NULL || window == NULL || app_state_out == NULL) {
        return -1;
    }

    app_state = tinyui_runtime_host_app_state_from_window(window);
    if (app_state == NULL || app_state->ld_scene == NULL) {
        return -1;
    }

    tinyui_runtime_host_runtime_bootstrap();
    app_state->ld_scene->ldGuiFuncGroup = &g_tinyui_runtime_host_page;
    *app_state_out = app_state;
    return 0;
}

static void tinyui_runtime_host_log_runtime_ready(struct picoui_app *app,
                                                  struct tinyui_runtime_host_state *state)
{
    if (app == NULL || state == NULL || state->ready_logged) {
        return;
    }

    if (app->focus_owner == NULL) {
        printf("PICOUI_FOCUS_RUNTIME_READY=1\n");
        fflush(stdout);
    }
    printf("PICOUI_RUNTIME_READY\n");
    fflush(stdout);
    state->ready_logged = 1;
    state->screen_create_end_ticks = SDL_GetTicks();
    tinyui_runtime_host_log_screen_create_benchmark(state);
}

static int tinyui_runtime_host_prepare_runtime(struct picoui_app *app, struct picoui_window *window)
{
    struct tinyui_runtime_host_state *state;
    struct picoui_backend_app_state *app_state;

    if (tinyui_runtime_host_prepare_runtime_state(app, &state) != 0) {
        return -1;
    }

    if (tinyui_runtime_host_prepare_runtime_scene(app, window, &app_state) != 0) {
        return -1;
    }

    state->auto_quit_ms = tinyui_runtime_host_parse_auto_quit_ms();

    if (tinyui_runtime_bridge_ensure_window(app) != 0) {
        return -1;
    }
    if (app_state->ld_scene->ptMsgQueue == NULL) {
        ldGuiSceneInit(app_state->ld_scene);
    }

    tinyui_runtime_host_log_runtime_ready(app, state);

    return 0;
}

static int tinyui_runtime_host_pump_sdl_events(struct picoui_app *app,
                                               struct tinyui_runtime_host_state *state)
{
    SDL_Event event;

    if (app == NULL || state == NULL) {
        return -1;
    }

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return 1;
        } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                   event.button.button == SDL_BUTTON_LEFT) {
            if (tinyui_runtime_host_touch_log_enabled()) {
                printf("[PICOUI_TOUCH][SDL] type=down button=%u pos=(%d,%d)\n",
                       (unsigned int)event.button.button,
                       event.button.x,
                       event.button.y);
                fflush(stdout);
            }
            (void)tinyui_runtime_bridge_commit_pointer_event(app,
                                                             state->display_width,
                                                             state->display_height,
                                                             event.button.x,
                                                             event.button.y,
                                                             1);
        } else if (event.type == SDL_MOUSEBUTTONUP &&
                   event.button.button == SDL_BUTTON_LEFT) {
            if (tinyui_runtime_host_touch_log_enabled()) {
                printf("[PICOUI_TOUCH][SDL] type=up button=%u pos=(%d,%d)\n",
                       (unsigned int)event.button.button,
                       event.button.x,
                       event.button.y);
                fflush(stdout);
            }
            (void)tinyui_runtime_bridge_commit_pointer_event(app,
                                                             state->display_width,
                                                             state->display_height,
                                                             event.button.x,
                                                             event.button.y,
                                                             0);
        } else if (event.type == SDL_MOUSEMOTION) {
            if (tinyui_runtime_host_touch_log_enabled()) {
                printf("[PICOUI_TOUCH][SDL] type=motion buttons=0x%x pos=(%d,%d)\n",
                       (unsigned int)event.motion.state,
                       event.motion.x,
                       event.motion.y);
                fflush(stdout);
            }
            (void)tinyui_runtime_bridge_commit_pointer_event(app,
                                                             state->display_width,
                                                             state->display_height,
                                                             event.motion.x,
                                                             event.motion.y,
                                                             (event.motion.state & SDL_BUTTON_LMASK) != 0U);
        } else if (event.type == SDL_WINDOWEVENT &&
                   event.window.event == SDL_WINDOWEVENT_EXPOSED) {
            SDL_RenderPresent(state->renderer);
        }
    }

    return 0;
}

int tinyui_runtime_host_step_app(struct picoui_app *app)
{
    struct tinyui_runtime_host_state *state;
    struct picoui_window *active_window;
    int event_result;

    if (app == NULL || app->root_window == NULL) {
        return -1;
    }

    if (tinyui_runtime_host_prepare_runtime(app, app->root_window) != 0) {
        return -1;
    }

    state = tinyui_runtime_host_state_from_app(app);
    if (state == NULL) {
        return -1;
    }

    event_result = tinyui_runtime_host_pump_sdl_events(app, state);
    if (event_result != 0) {
        return event_result;
    }

    active_window = app->root_window;
    if (active_window == NULL) {
        return -1;
    }

    tinyui_app_pump_timers(app, picoui_tick_get(app));
    tinyui_runtime_host_render(state, active_window);
    picoui_os_delay(app, 16);

    if (state->auto_quit_ms > 0 &&
        picoui_tick_get(app) - state->start_ticks >= state->auto_quit_ms) {
        return 1;
    }

    return 0;
}

/**
 * @brief Run app backend
 *
 * @param[in] app Application instance
 * @param[in] window Window instance
 * @return 0 on success, -1 on failure
 */
