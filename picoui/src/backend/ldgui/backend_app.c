#include "backend.h"
#include "internal.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PICOUI_RUNTIME_WIDTH 480
#define PICOUI_RUNTIME_HEIGHT 320
#define PICOUI_RUNTIME_PADDING 16
#define PICOUI_RUNTIME_ROW_HEIGHT 34
#define PICOUI_RUNTIME_ROW_GAP 10

static void picoui_backend_set_color(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
}

struct picoui_backend_runtime_widget {
    struct picoui_backend_widget *widget;
    struct picoui_backend_runtime_widget *next;
};

struct picoui_backend_runtime_state {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Uint32 start_ticks;
    Uint32 auto_quit_ms;
    int ready_logged;
    int capture_written;
    struct picoui_backend_runtime_widget *widgets;
};

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

static int picoui_backend_write_capture(struct picoui_backend_runtime_state *state)
{
    const char *path = getenv("PICOUI_CAPTURE_FILE");
    FILE *fp;
    unsigned char *pixels;
    int pitch;
    int x;
    int y;

    if (state == NULL || state->renderer == NULL || state->capture_written) {
        return 0;
    }

    if (path == NULL || path[0] == '\0') {
        return 0;
    }

    pitch = PICOUI_RUNTIME_WIDTH * 4;
    pixels = malloc((size_t)pitch * (size_t)PICOUI_RUNTIME_HEIGHT);
    if (pixels == NULL) {
        return -1;
    }

    if (SDL_RenderReadPixels(state->renderer,
                             NULL,
                             SDL_PIXELFORMAT_ARGB8888,
                             pixels,
                             pitch) != 0) {
        free(pixels);
        return -1;
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        free(pixels);
        return -1;
    }

    fprintf(fp, "P6\n%d %d\n255\n", PICOUI_RUNTIME_WIDTH, PICOUI_RUNTIME_HEIGHT);
    for (y = 0; y < PICOUI_RUNTIME_HEIGHT; ++y) {
        for (x = 0; x < PICOUI_RUNTIME_WIDTH; ++x) {
            const unsigned char *src = pixels + (size_t)y * (size_t)pitch + (size_t)x * 4u;
            unsigned char rgb[3];

            rgb[0] = src[1];
            rgb[1] = src[2];
            rgb[2] = src[3];
            fwrite(rgb, 1, 3, fp);
        }
    }

    fclose(fp);
    free(pixels);
    state->capture_written = 1;
    return 0;
}

static struct picoui_backend_runtime_state *picoui_backend_runtime_state_from_app(struct picoui_app *app)
{
    return (struct picoui_backend_runtime_state *)app->backend_app;
}

static int picoui_backend_runtime_push_widget(struct picoui_backend_runtime_state *state,
                                              struct picoui_backend_widget *widget)
{
    struct picoui_backend_runtime_widget *entry;

    if (state == NULL || widget == NULL) {
        return -1;
    }

    entry = calloc(1, sizeof(*entry));
    if (entry == NULL) {
        return -1;
    }

    entry->widget = widget;
    if (state->widgets == NULL) {
        state->widgets = entry;
        return 0;
    }

    {
        struct picoui_backend_runtime_widget *tail = state->widgets;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = entry;
    }

    return 0;
}

static void picoui_backend_runtime_collect_widgets(struct picoui_backend_runtime_state *state,
                                                   struct picoui_backend_widget *root)
{
    struct picoui_backend_runtime_widget *entry = state->widgets;
    struct picoui_backend_widget *child;

    while (entry != NULL) {
        struct picoui_backend_runtime_widget *next = entry->next;
        free(entry);
        entry = next;
    }
    state->widgets = NULL;

    if (root == NULL) {
        return;
    }

    for (child = root->first_child; child != NULL; child = child->next_sibling) {
        if (picoui_backend_runtime_push_widget(state, child) != 0) {
            break;
        }
    }
}

static int picoui_backend_widget_value(const struct picoui_backend_widget *widget)
{
    if (widget == NULL) {
        return 0;
    }
    return widget->value;
}

static const char *picoui_backend_widget_title(const struct picoui_backend_widget *widget)
{
    if (widget == NULL) {
        return "";
    }
    if (widget->text != NULL && widget->text[0] != '\0') {
        return widget->text;
    }
    if (widget->id != NULL) {
        return widget->id;
    }
    return "";
}

static void picoui_backend_draw_widget(SDL_Renderer *renderer,
                                       const struct picoui_backend_widget *widget,
                                       int x,
                                       int y,
                                       int width,
                                       int height)
{
    SDL_Rect rect = {x, y, width, height};
    int knob_x;
    int knob_y;
    int knob_radius;

    if (widget == NULL) {
        return;
    }

    switch (widget->kind) {
    case PICOUI_BACKEND_WIDGET_BUTTON:
        rect.w = 160;
        picoui_backend_set_color(renderer, 0x58, 0x7C, 0xAA);
        SDL_RenderFillRect(renderer, &rect);
        picoui_backend_set_color(renderer, 0xA9, 0xC0, 0xE1);
        SDL_RenderDrawRect(renderer, &rect);
        picoui_backend_set_color(renderer, 0xE8, 0xF0, 0xFA);
        SDL_Rect button_text = {x + 18, y + 12, 56, 8};
        SDL_RenderFillRect(renderer, &button_text);
        break;
    case PICOUI_BACKEND_WIDGET_SWITCH: {
        SDL_Rect track = {x + 22, y + 8, 84, height - 16};
        picoui_backend_set_color(renderer,
                                 picoui_backend_widget_value(widget) ? 0x68 : 0xA5,
                                 picoui_backend_widget_value(widget) ? 0xB7 : 0x6D,
                                 picoui_backend_widget_value(widget) ? 0x7A : 0x7A);
        SDL_RenderFillRect(renderer, &track);
        picoui_backend_set_color(renderer, 0xE8, 0xEC, 0xF1);
        knob_radius = 11;
        knob_x = picoui_backend_widget_value(widget) ? (track.x + track.w - 18) : (track.x + 18);
        knob_y = y + height / 2;
        for (int dy = -knob_radius; dy <= knob_radius; ++dy) {
            for (int dx = -knob_radius; dx <= knob_radius; ++dx) {
                if (dx * dx + dy * dy <= knob_radius * knob_radius) {
                    SDL_RenderDrawPoint(renderer, knob_x + dx, knob_y + dy);
                }
            }
        }
        picoui_backend_set_color(renderer, 0xCE, 0xD6, 0xE0);
        SDL_Rect switch_label = {x + 108, y + 14, 92, 6};
        SDL_RenderFillRect(renderer, &switch_label);
        break;
    }
    case PICOUI_BACKEND_WIDGET_CHECKBOX: {
        SDL_Rect box = {x + 22, y + 4, 24, 24};
        picoui_backend_set_color(renderer, 0xE9, 0xEE, 0xF3);
        SDL_RenderFillRect(renderer, &box);
        picoui_backend_set_color(renderer, 0x6E, 0x7D, 0x91);
        SDL_RenderDrawRect(renderer, &box);
        if (picoui_backend_widget_value(widget)) {
            SDL_Rect fill = {x + 27, y + 9, 14, 14};
            picoui_backend_set_color(renderer, 0x6F, 0xC2, 0x7A);
            SDL_RenderFillRect(renderer, &fill);
        }
        picoui_backend_set_color(renderer, 0xD7, 0xDF, 0xE8);
        SDL_Rect checkbox_text = {x + 62, y + 11, 120, 7};
        SDL_RenderFillRect(renderer, &checkbox_text);
        break;
    }
    case PICOUI_BACKEND_WIDGET_SLIDER:
        rect.w = 220;
        picoui_backend_set_color(renderer, 0x73, 0x82, 0x95);
        SDL_Rect slider_track = {x + 22, y + height / 2 - 2, 168, 4};
        SDL_RenderFillRect(renderer, &slider_track);
        picoui_backend_set_color(renderer, 0x7F, 0xD0, 0xDB);
        knob_x = x + 22 + (168 * picoui_backend_widget_value(widget)) / 100;
        knob_y = y + height / 2;
        knob_radius = 10;
        for (int dy = -knob_radius; dy <= knob_radius; ++dy) {
            for (int dx = -knob_radius; dx <= knob_radius; ++dx) {
                if (dx * dx + dy * dy <= knob_radius * knob_radius) {
                    SDL_RenderDrawPoint(renderer, knob_x + dx, knob_y + dy);
                }
            }
        }
        break;
    case PICOUI_BACKEND_WIDGET_IMAGE:
        rect.w = 220;
        picoui_backend_set_color(renderer, 0xF2, 0xD3, 0x85);
        SDL_RenderFillRect(renderer, &rect);
        picoui_backend_set_color(renderer, 0xA8, 0x7F, 0x2D);
        SDL_RenderDrawRect(renderer, &rect);
        picoui_backend_set_color(renderer, 0xF8, 0xE7, 0xBA);
        SDL_RenderDrawLine(renderer, x + 18, y + height - 12, x + 96, y + 14);
        SDL_RenderDrawLine(renderer, x + 96, y + 14, x + 180, y + height - 18);
        break;
    case PICOUI_BACKEND_WIDGET_TEXT:
        rect.w = 220;
        picoui_backend_set_color(renderer, 0xEA, 0xEF, 0xF4);
        SDL_RenderFillRect(renderer, &rect);
        picoui_backend_set_color(renderer, 0xCE, 0xD6, 0xE0);
        SDL_Rect text_line = {x + 16, y + 10, 116, 7};
        SDL_RenderFillRect(renderer, &text_line);
        break;
    case PICOUI_BACKEND_WIDGET_LABEL:
    default:
        rect.w = 220;
        picoui_backend_set_color(renderer, 0xE2, 0xE8, 0xF2);
        SDL_RenderFillRect(renderer, &rect);
        picoui_backend_set_color(renderer, 0xA9, 0xB8, 0xCB);
        SDL_RenderDrawRect(renderer, &rect);
        picoui_backend_set_color(renderer, 0xFA, 0xFC, 0xFF);
        SDL_Rect label_line = {x + 16, y + 11, 104, 7};
        SDL_RenderFillRect(renderer, &label_line);
        break;
    }
}

static void picoui_backend_draw_layout_hint(SDL_Renderer *renderer, struct picoui_window *window)
{
    SDL_Rect rect;

    if (renderer == NULL || window == NULL) {
        return;
    }

    rect.x = PICOUI_RUNTIME_PADDING / 2;
    rect.y = PICOUI_RUNTIME_PADDING / 2;
    rect.w = PICOUI_RUNTIME_WIDTH - PICOUI_RUNTIME_PADDING;
    rect.h = PICOUI_RUNTIME_HEIGHT - PICOUI_RUNTIME_PADDING;

    if (window->grid_col_count > 0 || window->grid_row_count > 0) {
        SDL_SetRenderDrawColor(renderer, 0x81, 0xA1, 0xC1, 0xFF);
        SDL_RenderDrawRect(renderer, &rect);
        if (window->grid_col_count > 1) {
            int col_width = rect.w / window->grid_col_count;
            int i;
            for (i = 1; i < window->grid_col_count; ++i) {
                SDL_RenderDrawLine(renderer,
                                   rect.x + i * col_width,
                                   rect.y,
                                   rect.x + i * col_width,
                                   rect.y + rect.h);
            }
        }
        if (window->grid_row_count > 1) {
            int row_height = rect.h / window->grid_row_count;
            int i;
            for (i = 1; i < window->grid_row_count; ++i) {
                SDL_RenderDrawLine(renderer,
                                   rect.x,
                                   rect.y + i * row_height,
                                   rect.x + rect.w,
                                   rect.y + i * row_height);
            }
        }
    } else if (window->flex_flow != PICOUI_FLEX_FLOW_ROW) {
        SDL_SetRenderDrawColor(renderer, 0x8F, 0xBC, 0xBB, 0xFF);
        SDL_RenderDrawRect(renderer, &rect);
    }
}

int picoui_backend_app_init(struct picoui_app *app)
{
    struct picoui_backend_runtime_state *state;

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

    state->auto_quit_ms = picoui_backend_parse_auto_quit_ms();
    app->backend_app = state;
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

    state->start_ticks = SDL_GetTicks();
    return 0;
}

static void picoui_backend_render(struct picoui_backend_runtime_state *state, struct picoui_window *window)
{
    struct picoui_backend_runtime_widget *entry;
    int x = PICOUI_RUNTIME_PADDING;
    int y = PICOUI_RUNTIME_PADDING + 20;
    int width = PICOUI_RUNTIME_WIDTH - PICOUI_RUNTIME_PADDING * 2;

    SDL_SetRenderDrawColor(state->renderer, 0x2E, 0x34, 0x40, 0xFF);
    SDL_RenderClear(state->renderer);

    picoui_backend_draw_layout_hint(state->renderer, window);

    entry = state->widgets;
    while (entry != NULL) {
        int height = PICOUI_RUNTIME_ROW_HEIGHT;
        const struct picoui_backend_widget *widget = entry->widget;
        int draw_width = 220;

        if (widget->kind == PICOUI_BACKEND_WIDGET_IMAGE) {
            height = 56;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_BUTTON) {
            draw_width = 160;
        }

        picoui_backend_draw_widget(state->renderer, widget, x, y, draw_width, height);

        y += height + PICOUI_RUNTIME_ROW_GAP;
        entry = entry->next;
    }

    SDL_RenderPresent(state->renderer);
    (void)picoui_backend_write_capture(state);
}

int picoui_backend_app_run(struct picoui_app *app, struct picoui_window *window)
{
    struct picoui_backend_runtime_state *state;
    struct picoui_backend_widget *root;
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

    root = (struct picoui_backend_widget *)window->widget.backend_widget;
    picoui_backend_runtime_collect_widgets(state, root);

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
    struct picoui_backend_runtime_widget *entry;

    if (app == NULL || app->backend_app == NULL) {
        return;
    }

    state = picoui_backend_runtime_state_from_app(app);
    if (state == NULL) {
        app->backend_app = NULL;
        return;
    }

    entry = state->widgets;
    while (entry != NULL) {
        struct picoui_backend_runtime_widget *next = entry->next;
        free(entry);
        entry = next;
    }

    if (state->renderer != NULL) {
        SDL_DestroyRenderer(state->renderer);
    }
    if (state->window != NULL) {
        SDL_DestroyWindow(state->window);
    }
    SDL_Quit();
    free(state);
    app->backend_app = NULL;
}
