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

#include <SDL.h>
#include "arm_2d.h"
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

int picoui_native_render_once(void *root_backend_widget);

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

static int picoui_backend_touch_log_enabled(void)
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

static int16_t picoui_backend_map_pointer_axis(int value, int window_extent, int target_extent)
{
    (void)window_extent;
    (void)target_extent;

    if (value < 0) {
        return 0;
    }
    if (value > 32767) {
        return 32767;
    }

    return (int16_t)value;
}

static void picoui_backend_runtime_page_init(ld_scene_t *scene)
{
    (void)scene;
}

static void picoui_backend_runtime_page_quit(ld_scene_t *scene)
{
    (void)scene;
}

struct picoui_backend_runtime_state {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    COLOUR_INT *real_pixels;
    uint32_t *present_pixels;
    arm_2d_tile_t real_tile;
    Uint32 start_ticks;
    Uint32 auto_quit_ms;
    int display_width;
    int display_height;
    int ready_logged;
    int capture_written;
    int static_mapping_logged;
    int fallback_boundary_logged;
    int image_source_logged;
    int temporary_smoke_logged;
    int smoke_layout_used;
    int smoke_layout_marker_logged;
};

static unsigned int picoui_backend_default_tick_source(void *user_data)
{
    (void)user_data;
    return (unsigned int)SDL_GetTicks();
}

static void picoui_backend_default_delay(unsigned int ms, void *user_data)
{
    (void)user_data;
    SDL_Delay((Uint32)ms);
}

static int picoui_backend_get_display_config(const struct picoui_app *app,
                                             struct picoui_display_config *config)
{
    return picoui_display_get_config(app, config);
}

static void picoui_backend_push_pointer_to_port(struct picoui_app *app, int x, int y, int pressed)
{
    if (app == NULL) {
        return;
    }

    (void)picoui_input_push_pointer(app, x, y, pressed);
}

static void picoui_backend_bridge_pointer_from_port(struct picoui_app *app,
                                                    struct picoui_backend_runtime_state *state)
{
    struct picoui_display_config display = {0};
    int pointer_x = 0;
    int pointer_y = 0;
    int pointer_pressed = 0;
    int window_width;
    int window_height;
    int16_t mapped_x;
    int16_t mapped_y;

    if (app == NULL || state == NULL || state->window == NULL) {
        return;
    }

    if (picoui_input_get_pointer(app, &pointer_x, &pointer_y, &pointer_pressed) != 0) {
        return;
    }

    if (picoui_backend_get_display_config(app, &display) != 0) {
        return;
    }

    window_width = display.width;
    window_height = display.height;
    SDL_GetWindowSize(state->window, &window_width, &window_height);
    mapped_x = picoui_backend_map_pointer_axis(pointer_x, window_width, display.width);
    mapped_y = picoui_backend_map_pointer_axis(pointer_y, window_height, display.height);
    if (picoui_backend_touch_log_enabled()) {
        printf("[PICOUI_TOUCH][PORT->LD] raw=(%d,%d) window=(%d,%d) mapped=(%d,%d) pressed=%d\n",
               pointer_x,
               pointer_y,
               window_width,
               window_height,
               mapped_x,
               mapped_y,
               pointer_pressed ? 1 : 0);
        fflush(stdout);
    }
    ldCfgTouchSetPoint(mapped_x, mapped_y, pointer_pressed != 0);
}

static void picoui_backend_commit_pointer_event(struct picoui_backend_runtime_state *state,
                                                struct picoui_app *app,
                                                int x,
                                                int y,
                                                int pressed)
{
    if (state == NULL || state->window == NULL || app == NULL) {
        return;
    }

    picoui_backend_push_pointer_to_port(app, x, y, pressed);
    picoui_backend_bridge_pointer_from_port(app, state);
}

static const ldPageFuncGroup_t g_picoui_backend_runtime_page = {
    .init = picoui_backend_runtime_page_init,
    .loop = NULL,
    .quit = picoui_backend_runtime_page_quit,
    .draw = NULL,
    .frameStart = NULL,
    .frameComplete = NULL,
#if (USE_LOG_LEVEL>=LOG_LEVEL_INFO)
    .pageName = "picoui_runtime",
#endif
    .pointer = NULL,
};

static int picoui_backend_widget_is_supported_real(const struct picoui_backend_widget *widget)
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

static int picoui_backend_widget_is_real_mapped(const struct picoui_backend_widget *widget)
{
    return widget != NULL &&
           widget->kind != PICOUI_BACKEND_WIDGET_WINDOW &&
           widget->kind != PICOUI_BACKEND_WIDGET_BACKGROUND &&
           picoui_backend_widget_is_supported_real(widget) &&
           widget->ld_widget != NULL;
}

static void picoui_backend_append_id(const char *id,
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

static int picoui_backend_widget_needs_fallback(const struct picoui_backend_widget *widget)
{
    return widget != NULL &&
           widget->kind != PICOUI_BACKEND_WIDGET_WINDOW &&
           widget->kind != PICOUI_BACKEND_WIDGET_BACKGROUND &&
           (!picoui_backend_widget_is_supported_real(widget) || widget->ld_widget == NULL);
}

static int picoui_backend_window_has_real_layout(const struct picoui_backend_widget *widget)
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

static int picoui_backend_widget_excludes_formal_mapping(const struct picoui_backend_widget *widget)
{
    while (widget != NULL) {
        if ((widget->runtime_evidence_flags & PICOUI_BACKEND_EVIDENCE_EXCLUDE_FORMAL_MAPPING) != 0U) {
            return 1;
        }
        if (widget->first_child != NULL && picoui_backend_widget_excludes_formal_mapping(widget->first_child)) {
            return 1;
        }
        widget = widget->next_sibling;
    }
    return 0;
}

static int picoui_backend_widget_allows_smoke_layout(const struct picoui_backend_widget *widget)
{
    while (widget != NULL) {
        if ((widget->runtime_evidence_flags & PICOUI_BACKEND_EVIDENCE_ALLOW_SMOKE_LAYOUT) != 0U) {
            return 1;
        }
        if (widget->first_child != NULL && picoui_backend_widget_allows_smoke_layout(widget->first_child)) {
            return 1;
        }
        widget = widget->next_sibling;
    }
    return 0;
}

static void picoui_backend_append_widget_ids(const struct picoui_backend_widget *widget,
                                             int (*predicate)(const struct picoui_backend_widget *widget),
                                             char *buffer,
                                             size_t buffer_size,
                                             size_t *used)
{
    while (widget != NULL) {
        if (predicate(widget) && widget->id != NULL && widget->id[0] != '\0') {
            picoui_backend_append_id(widget->id, buffer, buffer_size, used);
        }

        if (widget->first_child != NULL) {
            picoui_backend_append_widget_ids(widget->first_child,
                                             predicate,
                                             buffer,
                                             buffer_size,
                                             used);
        }

        widget = widget->next_sibling;
    }
}

static void picoui_backend_log_mapping_markers(struct picoui_backend_runtime_state *state,
                                               const struct picoui_backend_widget *root)
{
    char real_ids[256] = {0};
    char fallback_ids[256] = {0};
    size_t real_used = 0;
    size_t fallback_used = 0;

    if (state == NULL || root == NULL) {
        return;
    }

    picoui_backend_append_widget_ids(root->first_child,
                                     picoui_backend_widget_is_real_mapped,
                                     real_ids,
                                     sizeof(real_ids),
                                     &real_used);
    picoui_backend_append_widget_ids(root->first_child,
                                     picoui_backend_widget_needs_fallback,
                                     fallback_ids,
                                     sizeof(fallback_ids),
                                     &fallback_used);

    if (real_used > 0 &&
        !picoui_backend_widget_excludes_formal_mapping(root->first_child) &&
        !state->static_mapping_logged) {
        printf("PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI\n");
        printf("PICOUI_BACKEND_REAL_WIDGET_IDS=%s\n", real_ids);
        fflush(stdout);
        state->static_mapping_logged = 1;
    }

    if (picoui_backend_widget_excludes_formal_mapping(root->first_child) &&
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

static const struct picoui_backend_widget *picoui_backend_find_first_image_widget(const struct picoui_backend_widget *widget)
{
    while (widget != NULL) {
        if (widget->kind == PICOUI_BACKEND_WIDGET_IMAGE && widget->id != NULL && widget->ld_widget != NULL) {
            return widget;
        }

        if (widget->first_child != NULL) {
            const struct picoui_backend_widget *found =
                picoui_backend_find_first_image_widget(widget->first_child);
            if (found != NULL) {
                return found;
            }
        }

        widget = widget->next_sibling;
    }

    return NULL;
}

static void picoui_backend_log_image_source_marker(struct picoui_backend_runtime_state *state,
                                                   const struct picoui_backend_widget *widget)
{
    const struct picoui_backend_widget *image_widget;
    ldImage_t *ld_image;

    if (state == NULL || state->image_source_logged) {
        return;
    }

    image_widget = picoui_backend_find_first_image_widget(widget);
    if (image_widget == NULL) {
        return;
    }

    ld_image = (ldImage_t *)image_widget->ld_widget;
    printf("PICOUI_BACKEND_IMAGE_SOURCE=%s:img=%s,mask=%s\n",
           image_widget->id,
           ld_image->ptImgTile != NULL ? "set" : "null",
           ld_image->ptMaskTile != NULL ? "set" : "null");
    fflush(stdout);
    state->image_source_logged = 1;
}

static Uint32 picoui_backend_parse_auto_quit_ms(void)
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

static uint32_t picoui_backend_pixel_to_rgb888(COLOUR_INT pixel)
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

static uint32_t picoui_backend_pixel_to_argb8888(COLOUR_INT pixel)
{
    return 0xFF000000U | picoui_backend_pixel_to_rgb888(pixel);
}

static int picoui_backend_write_capture(struct picoui_backend_runtime_state *state)
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
            uint32_t rgb888 = picoui_backend_pixel_to_rgb888(pixel);
            unsigned char rgb[3];

            rgb[0] = (unsigned char)((rgb888 >> 16) & 0xFFU);
            rgb[1] = (unsigned char)((rgb888 >> 8) & 0xFFU);
            rgb[2] = (unsigned char)(rgb888 & 0xFFU);
            fwrite(rgb, 1, 3, fp);
        }
    }

    fclose(fp);
    state->capture_written = 1;
    return 0;
}

static struct picoui_backend_runtime_state *picoui_backend_runtime_state_from_app(struct picoui_app *app)
{
    struct picoui_backend_app_state *app_state;

    if (app == NULL || app->backend_app == NULL) {
        return NULL;
    }
    app_state = (struct picoui_backend_app_state *)app->backend_app;
    return (struct picoui_backend_runtime_state *)app_state->runtime_state;
}

static struct picoui_backend_app_state *picoui_backend_app_state_from_window(struct picoui_window *window)
{
    const struct picoui_backend_widget *root_widget;

    if (window == NULL || window->widget.backend_widget == NULL) {
        return NULL;
    }

    root_widget = (const struct picoui_backend_widget *)window->widget.backend_widget;
    if (root_widget->owner == NULL || root_widget->owner->backend_app == NULL) {
        return NULL;
    }

    return (struct picoui_backend_app_state *)root_widget->owner->backend_app;
}

/**
 * @brief Initialize app backend
 *
 * @param[in] app Application instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_app_init(struct picoui_app *app)
{
    struct picoui_backend_runtime_state *state;
    struct picoui_backend_app_state *app_state;

    if (app == NULL) {
        return -1;
    }

    if (app->backend_app != NULL) {
        return 0;
    }

    state = calloc(1, sizeof(*state));
    if (state == NULL) {
        return -1;
    }

    app_state = calloc(1, sizeof(*app_state));
    if (app_state == NULL) {
        free(state);
        return -1;
    }

    state->auto_quit_ms = picoui_backend_parse_auto_quit_ms();
    state->display_width = 480;
    state->display_height = 320;
    app_state->ld_scene = calloc(1, sizeof(*app_state->ld_scene));
    if (app_state->ld_scene == NULL) {
        free(app_state);
        free(state);
        return -1;
    }

    app_state->theme = app->theme;
    app_state->next_ld_name_id = 0;
    app_state->ld_scene->bUserAllocated = true;
    app_state->ld_scene->ldGuiFuncGroup = &g_picoui_backend_runtime_page;
    app_state->ld_scene->ptNodeRoot = NULL;
    app_state->ld_scene->ptMsgQueue = NULL;
    app_state->runtime_state = state;
    state->capture_written = 0;
    state->ready_logged = 0;
    state->static_mapping_logged = 0;
    state->fallback_boundary_logged = 0;
    state->image_source_logged = 0;
    state->temporary_smoke_logged = 0;
    state->smoke_layout_used = 0;
    state->smoke_layout_marker_logged = 0;
    (void)picoui_tick_set_source(app, picoui_backend_default_tick_source, NULL);
    (void)picoui_os_set_delay_callback(app, picoui_backend_default_delay, NULL);
    app->backend_app = app_state;
    return 0;
}

static int picoui_backend_ensure_window(struct picoui_app *app,
                                        struct picoui_backend_runtime_state *state)
{
    struct picoui_display_config display = {0};

    if (picoui_backend_get_display_config(app, &display) != 0) {
        return -1;
    }

    state->display_width = display.width;
    state->display_height = display.height;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "PicoUI runtime SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    state->window = SDL_CreateWindow("PicoUI Demo",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     state->display_width,
                                     state->display_height,
                                     SDL_WINDOW_SHOWN);
    if (state->window == NULL) {
        fprintf(stderr, "PicoUI runtime SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_ACCELERATED);
    if (state->renderer == NULL) {
        state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (state->renderer == NULL) {
        fprintf(stderr, "PicoUI runtime SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(state->window);
        state->window = NULL;
        SDL_Quit();
        return -1;
    }

    state->texture = SDL_CreateTexture(state->renderer,
                                       SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_STREAMING,
                                       state->display_width,
                                       state->display_height);
    if (state->texture == NULL) {
        fprintf(stderr, "PicoUI runtime SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        state->renderer = NULL;
        state->window = NULL;
        SDL_Quit();
        return -1;
    }

    state->real_pixels = calloc((size_t)state->display_width * (size_t)state->display_height,
                                sizeof(*state->real_pixels));
    if (state->real_pixels == NULL) {
        SDL_DestroyTexture(state->texture);
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        state->texture = NULL;
        state->renderer = NULL;
        state->window = NULL;
        SDL_Quit();
        return -1;
    }

    state->present_pixels = calloc((size_t)state->display_width * (size_t)state->display_height,
                                   sizeof(*state->present_pixels));
    if (state->present_pixels == NULL) {
        free(state->real_pixels);
        SDL_DestroyTexture(state->texture);
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        state->real_pixels = NULL;
        state->texture = NULL;
        state->renderer = NULL;
        state->window = NULL;
        SDL_Quit();
        return -1;
    }

    state->real_tile = (arm_2d_tile_t) {
        .tRegion = {
            .tLocation = {
                .iX = 0,
                .iY = 0,
            },
            .tSize = {
                .iWidth = state->display_width,
                .iHeight = state->display_height,
            },
        },
        .tInfo = {
            .bIsRoot = true,
            .bHasEnforcedColour = true,
            .tColourInfo = {
                .chScheme = __DISP0_COLOUR_FORMAT__,
            },
        },
        .pchBuffer = (uint8_t *)state->real_pixels,
    };

    state->start_ticks = picoui_tick_get(app);
    return 0;
}

static void picoui_backend_present_real_frame(struct picoui_backend_runtime_state *state)
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
            state->present_pixels[index] = picoui_backend_pixel_to_argb8888(state->real_pixels[index]);
        }
    }

    SDL_UpdateTexture(state->texture,
                      NULL,
                      state->present_pixels,
                      (int)(state->display_width * (int)sizeof(*state->present_pixels)));
    SDL_RenderCopy(state->renderer, state->texture, NULL, NULL);
}

static void picoui_backend_apply_real_widget_layout(struct picoui_backend_runtime_state *state,
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

            if (picoui_backend_widget_is_supported_real(widget) && widget->ld_widget != NULL) {
                arm_2d_region_t region = ldBaseGetRegion((ldBase_t *)widget->ld_widget);
                region.tLocation.iX = (int16_t)x;
                region.tLocation.iY = (int16_t)(*cursor_y);
                ldBaseSetRegion((ldBase_t *)widget->ld_widget, region);
                height = region.tSize.iHeight > 0 ? region.tSize.iHeight : height;
            }

            *cursor_y += height + PICOUI_RUNTIME_ROW_GAP;
        }

        if (widget->first_child != NULL) {
            picoui_backend_apply_real_widget_layout(state, widget->first_child, x, cursor_y);
        }

        widget = widget->next_sibling;
    }
}

static void picoui_backend_apply_smoke_cursor_layout(struct picoui_backend_runtime_state *state,
                                                     const struct picoui_backend_widget *root,
                                                     int x,
                                                     int *cursor_y)
{
    if (root == NULL || root->first_child == NULL || picoui_backend_window_has_real_layout(root) ||
        !picoui_backend_widget_allows_smoke_layout(root->first_child)) {
        return;
    }

    /* Explicit opt-in only: keep temporary smoke layout out of generic non-layout demos. */
    state->smoke_layout_used = 1;
    picoui_backend_apply_real_widget_layout(state, root->first_child, x, cursor_y);
}

static void picoui_backend_render(struct picoui_backend_runtime_state *state,
                                  struct picoui_window *window,
                                  int native_render_already_synced)
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
        app_state = picoui_backend_app_state_from_window(window);
        picoui_backend_log_mapping_markers(state, root);
        if (app_state != NULL && app_state->ld_scene != NULL && state->real_pixels != NULL) {
            state->smoke_layout_used = 0;
            memset(state->real_pixels,
                   0,
                   (size_t)state->display_width * (size_t)state->display_height *
                       sizeof(*state->real_pixels));
            picoui_backend_apply_smoke_cursor_layout(state, root_widget, x, &y);
            if (!native_render_already_synced
                && picoui_native_render_once(window->widget.backend_widget) != 0) {
                return;
            }
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
            picoui_backend_log_image_source_marker(state, root->first_child);
            picoui_backend_present_real_frame(state);
        }
    }

    SDL_RenderPresent(state->renderer);
    (void)picoui_backend_write_capture(state);
}

static void picoui_backend_pump_timers(struct picoui_app *app, unsigned int now_ticks)
{
    struct picoui_backend_timer_snapshot_entry {
        struct picoui_app_timer *timer;
        struct picoui_app_timer *expected_predecessor;
    };

    struct picoui_backend_timer_snapshot_entry *snapshot;
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

void picoui_backend_test_pump_timers(struct picoui_app *app, unsigned int now_ticks)
{
    picoui_backend_pump_timers(app, now_ticks);
}

/**
 * @brief Run app backend
 *
 * @param[in] app Application instance
 * @param[in] window Window instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_app_run(struct picoui_app *app, struct picoui_window *window)
{
    struct picoui_backend_runtime_state *state;
    struct picoui_backend_app_state *app_state;
    struct picoui_window *active_window;
    int running = 1;

    if (app == NULL || window == NULL) {
        return -1;
    }

    state = picoui_backend_runtime_state_from_app(app);
    if (state == NULL && picoui_backend_app_init(app) != 0) {
        return -1;
    }
    state = picoui_backend_runtime_state_from_app(app);
    if (state == NULL) {
        return -1;
    }

    if (picoui_backend_ensure_window(app, state) != 0) {
        return -1;
    }

    app_state = picoui_backend_app_state_from_window(window);
    if (app_state == NULL || app_state->ld_scene == NULL) {
        return -1;
    }
    if (app_state->ld_scene->ptMsgQueue == NULL) {
        ldGuiSceneInit(app_state->ld_scene);
    }

    if (!state->ready_logged) {
        if (app->focus_owner == NULL) {
            printf("PICOUI_FOCUS_RUNTIME_READY=1\n");
            fflush(stdout);
        }
        printf("PICOUI_RUNTIME_READY\n");
        fflush(stdout);
        state->ready_logged = 1;
    }

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                       event.button.button == SDL_BUTTON_LEFT) {
                if (picoui_backend_touch_log_enabled()) {
                    printf("[PICOUI_TOUCH][SDL] type=down button=%u pos=(%d,%d)\n",
                           (unsigned int)event.button.button,
                           event.button.x,
                           event.button.y);
                    fflush(stdout);
                }
                picoui_backend_commit_pointer_event(state,
                                                    app,
                                                    event.button.x,
                                                    event.button.y,
                                                    1);
            } else if (event.type == SDL_MOUSEBUTTONUP &&
                       event.button.button == SDL_BUTTON_LEFT) {
                if (picoui_backend_touch_log_enabled()) {
                    printf("[PICOUI_TOUCH][SDL] type=up button=%u pos=(%d,%d)\n",
                           (unsigned int)event.button.button,
                           event.button.x,
                           event.button.y);
                    fflush(stdout);
                }
                picoui_backend_commit_pointer_event(state,
                                                    app,
                                                    event.button.x,
                                                    event.button.y,
                                                    0);
            } else if (event.type == SDL_MOUSEMOTION) {
                if (picoui_backend_touch_log_enabled()) {
                    printf("[PICOUI_TOUCH][SDL] type=motion buttons=0x%x pos=(%d,%d)\n",
                           (unsigned int)event.motion.state,
                           event.motion.x,
                           event.motion.y);
                    fflush(stdout);
                }
                picoui_backend_commit_pointer_event(state,
                                                    app,
                                                    event.motion.x,
                                                    event.motion.y,
                                                    (event.motion.state & SDL_BUTTON_LMASK) != 0U);
            } else if (event.type == SDL_WINDOWEVENT &&
                       event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                SDL_RenderPresent(state->renderer);
            }
        }

        active_window = app->root_window != NULL ? app->root_window : window;
        if (active_window == NULL) {
            return -1;
        }
        picoui_backend_pump_timers(app, picoui_tick_get(app));
        picoui_backend_render(state, active_window, 0);
        picoui_os_delay(app, 16);

        if (state->auto_quit_ms > 0 &&
            picoui_tick_get(app) - state->start_ticks >= state->auto_quit_ms) {
            running = 0;
        }
    }

    return 0;
}

int picoui_backend_native_render_capture(struct picoui_window *window)
{
    struct picoui_backend_runtime_state *state;
    struct picoui_backend_app_state *app_state;
    struct picoui_app *app;

    if (window == NULL || window->widget.backend_widget == NULL) {
        return -1;
    }

    app_state = picoui_backend_app_state_from_window(window);
    if (app_state == NULL || app_state->ld_scene == NULL) {
        return -1;
    }
    app = picoui_backend_widget_get_owner(window->widget.backend_widget);
    if (app == NULL) {
        return -1;
    }

    state = picoui_backend_runtime_state_from_app(app);
    if (state == NULL) {
        return -1;
    }
    if (picoui_backend_ensure_window(app, state) != 0) {
        return -1;
    }

    if (app_state->ld_scene->ptMsgQueue == NULL) {
        ldGuiSceneInit(app_state->ld_scene);
    }

    if (!state->ready_logged) {
        if (app->focus_owner == NULL) {
            printf("PICOUI_FOCUS_RUNTIME_READY=1\n");
            fflush(stdout);
        }
        printf("PICOUI_RUNTIME_READY\n");
        fflush(stdout);
        state->ready_logged = 1;
    }

    picoui_backend_render(state, window, 1);
    return 0;
}

/**
 * @brief Shutdown app backend
 *
 * @param[in] app Application instance
 */

void picoui_backend_app_shutdown(struct picoui_app *app)
{
    struct picoui_backend_runtime_state *state;
    struct picoui_backend_app_state *app_state;

    if (app == NULL) {
        return;
    }

    state = picoui_backend_runtime_state_from_app(app);
    app_state = (struct picoui_backend_app_state *)app->backend_app;

    if (state != NULL && state->renderer != NULL) {
        SDL_DestroyRenderer(state->renderer);
    }
    if (state != NULL && state->texture != NULL) {
        SDL_DestroyTexture(state->texture);
    }
    if (state != NULL && state->window != NULL) {
        SDL_DestroyWindow(state->window);
    }
    SDL_Quit();
    if (state != NULL) {
        free(state->present_pixels);
        free(state->real_pixels);
        free(state);
    }
    if (app_state != NULL) {
        if (app_state->ld_scene != NULL) {
            ldGuiDespose(app_state->ld_scene);
        }
        free(app_state->ld_scene);
    }
    free(app_state);
    app->backend_app = NULL;
}
