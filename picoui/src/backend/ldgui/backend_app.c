#include "backend.h"
#include "internal.h"

#include <SDL.h>
#include "arm_2d.h"
#include "ldBase.h"
#include "ldGui.h"
#include "arm_2d_disp_adapter_0.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PICOUI_RUNTIME_WIDTH 480
#define PICOUI_RUNTIME_HEIGHT 320
#define PICOUI_RUNTIME_PADDING 16
#define PICOUI_RUNTIME_ROW_HEIGHT 34
#define PICOUI_RUNTIME_ROW_GAP 10

__attribute__((weak)) void VT_enter_global_mutex(void) {}
__attribute__((weak)) void VT_leave_global_mutex(void) {}

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
    int ready_logged;
    int capture_written;
    int static_mapping_logged;
    int fallback_boundary_logged;
};

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
    case PICOUI_BACKEND_WIDGET_WINDOW:
    case PICOUI_BACKEND_WIDGET_LABEL:
    case PICOUI_BACKEND_WIDGET_BUTTON:
    case PICOUI_BACKEND_WIDGET_CHECKBOX:
    case PICOUI_BACKEND_WIDGET_TEXT:
    case PICOUI_BACKEND_WIDGET_IMAGE:
    case PICOUI_BACKEND_WIDGET_SWITCH:
    case PICOUI_BACKEND_WIDGET_SLIDER:
        return 1;
    default:
        return 0;
    }
}

static int picoui_backend_widget_is_real_mapped(const struct picoui_backend_widget *widget)
{
    return widget != NULL &&
           widget->kind != PICOUI_BACKEND_WIDGET_WINDOW &&
           picoui_backend_widget_is_supported_real(widget) &&
           widget->ld_widget != NULL;
}

static int picoui_backend_widget_needs_fallback(const struct picoui_backend_widget *widget)
{
    return widget != NULL &&
           widget->kind != PICOUI_BACKEND_WIDGET_WINDOW &&
           (!picoui_backend_widget_is_supported_real(widget) || widget->ld_widget == NULL);
}

static int picoui_backend_window_has_real_layout(const struct picoui_backend_widget *widget)
{
    ldWindow_t *ld_window;

    if (widget == NULL || widget->kind != PICOUI_BACKEND_WIDGET_WINDOW || widget->ld_widget == NULL) {
        return 0;
    }

    ld_window = (ldWindow_t *)widget->ld_widget;
    return ld_window->layoutTpye == layoutFlex || ld_window->layoutTpye == layoutGrid;
}

static void picoui_backend_append_widget_ids(const struct picoui_backend_widget *widget,
                                             int (*predicate)(const struct picoui_backend_widget *widget),
                                             char *buffer,
                                             size_t buffer_size,
                                             size_t *used)
{
    while (widget != NULL) {
        if (predicate(widget) && widget->id != NULL && widget->id[0] != '\0') {
            int written;

            if (*used > 0 && *used + 1 < buffer_size) {
                buffer[*used] = ',';
                *used += 1;
                buffer[*used] = '\0';
            }

            if (*used + 1 < buffer_size) {
                written = snprintf(buffer + *used, buffer_size - *used, "%s", widget->id);
                if (written > 0) {
                    size_t advance = (size_t)written;
                    if (advance >= buffer_size - *used) {
                        *used = buffer_size - 1;
                    } else {
                        *used += advance;
                    }
                }
            }
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

    if (real_used > 0 && !state->static_mapping_logged) {
        printf("PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI\n");
        printf("PICOUI_BACKEND_REAL_WIDGET_IDS=%s\n", real_ids);
        fflush(stdout);
        state->static_mapping_logged = 1;
    }

    if (fallback_used > 0 && !state->fallback_boundary_logged) {
        printf("PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK\n");
        printf("PICOUI_BACKEND_FALLBACK_WIDGET_IDS=%s\n", fallback_ids);
        fflush(stdout);
        state->fallback_boundary_logged = 1;
    }
}

static void picoui_backend_log_image_source_marker(const struct picoui_backend_widget *widget)
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
            picoui_backend_log_image_source_marker(widget->first_child);
        }

        widget = widget->next_sibling;
    }
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

    fprintf(fp, "P6\n%d %d\n255\n", PICOUI_RUNTIME_WIDTH, PICOUI_RUNTIME_HEIGHT);
    for (y = 0; y < PICOUI_RUNTIME_HEIGHT; ++y) {
        for (x = 0; x < PICOUI_RUNTIME_WIDTH; ++x) {
            COLOUR_INT pixel = state->real_pixels[(size_t)y * (size_t)PICOUI_RUNTIME_WIDTH + (size_t)x];
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
    app->backend_app = app_state;
    return 0;
}

static int picoui_backend_ensure_window(struct picoui_backend_runtime_state *state)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "PicoUI runtime SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    state->window = SDL_CreateWindow("PicoUI Demo",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     PICOUI_RUNTIME_WIDTH,
                                     PICOUI_RUNTIME_HEIGHT,
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
                                       PICOUI_RUNTIME_WIDTH,
                                       PICOUI_RUNTIME_HEIGHT);
    if (state->texture == NULL) {
        fprintf(stderr, "PicoUI runtime SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        state->renderer = NULL;
        state->window = NULL;
        SDL_Quit();
        return -1;
    }

    state->real_pixels = calloc((size_t)PICOUI_RUNTIME_WIDTH * (size_t)PICOUI_RUNTIME_HEIGHT,
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

    state->present_pixels = calloc((size_t)PICOUI_RUNTIME_WIDTH * (size_t)PICOUI_RUNTIME_HEIGHT,
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
                .iWidth = PICOUI_RUNTIME_WIDTH,
                .iHeight = PICOUI_RUNTIME_HEIGHT,
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

    state->start_ticks = SDL_GetTicks();
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

    for (y = 0; y < PICOUI_RUNTIME_HEIGHT; ++y) {
        for (x = 0; x < PICOUI_RUNTIME_WIDTH; ++x) {
            size_t index = (size_t)y * (size_t)PICOUI_RUNTIME_WIDTH + (size_t)x;
            state->present_pixels[index] = picoui_backend_pixel_to_argb8888(state->real_pixels[index]);
        }
    }

    SDL_UpdateTexture(state->texture,
                      NULL,
                      state->present_pixels,
                      (int)(PICOUI_RUNTIME_WIDTH * sizeof(*state->present_pixels)));
    SDL_RenderCopy(state->renderer, state->texture, NULL, NULL);
}

static void picoui_backend_apply_real_widget_layout(struct picoui_backend_runtime_state *state,
                                                    const struct picoui_backend_widget *widget,
                                                    int x,
                                                    int *cursor_y)
{
    while (widget != NULL) {
        if (widget->kind != PICOUI_BACKEND_WIDGET_WINDOW) {
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
    if (root == NULL || root->first_child == NULL || picoui_backend_window_has_real_layout(root)) {
        return;
    }

    /* temporary smoke path: non-layout demos still need default root-child placement. */
    picoui_backend_apply_real_widget_layout(state, root->first_child, x, cursor_y);
}

static void picoui_backend_render(struct picoui_backend_runtime_state *state, struct picoui_window *window)
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
            memset(state->real_pixels,
                   0,
                   (size_t)PICOUI_RUNTIME_WIDTH * (size_t)PICOUI_RUNTIME_HEIGHT * sizeof(*state->real_pixels));
            picoui_backend_apply_smoke_cursor_layout(state, root_widget, x, &y);
            ldGuiFrameStart(app_state->ld_scene);
            ldMsgProcess(app_state->ld_scene);
            ldGuiDraw(app_state->ld_scene, &state->real_tile, true);
            ldGuiFrameComplete(app_state->ld_scene);
            picoui_backend_log_image_source_marker(root->first_child);
            picoui_backend_present_real_frame(state);
        }
    }

    SDL_RenderPresent(state->renderer);
    (void)picoui_backend_write_capture(state);
}

int picoui_backend_app_run(struct picoui_app *app, struct picoui_window *window)
{
    struct picoui_backend_runtime_state *state;
    struct picoui_backend_app_state *app_state;
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

    if (picoui_backend_ensure_window(state) != 0) {
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
        printf("PICOUI_RUNTIME_READY\n");
        fflush(stdout);
        state->ready_logged = 1;
    }

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        picoui_backend_render(state, window);
        SDL_Delay(16);

        if (state->auto_quit_ms > 0 &&
            SDL_GetTicks() - state->start_ticks >= state->auto_quit_ms) {
            running = 0;
        }
    }

    return 0;
}

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
        free(app_state->ld_scene);
    }
    free(app_state);
    app->backend_app = NULL;
}
