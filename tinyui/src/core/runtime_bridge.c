#include "internal.h"
#include "runtime_bridge.h"
#include "../../../src/porting/ldConfig.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/gui/ldSlider.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/gui/ldGui.h"
#include "../../../src/misc/ldMsg.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
uint32_t SDL_GetTicks(void);
void SDL_Delay(uint32_t ms);
void SDL_DestroyRenderer(SDL_Renderer *renderer);
void SDL_DestroyTexture(SDL_Texture *texture);
void SDL_DestroyWindow(SDL_Window *window);
void SDL_Quit(void);
int SDL_Init(uint32_t flags);
const char *SDL_GetError(void);
SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, uint32_t flags);
SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, uint32_t flags);
SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer,
                               uint32_t format,
                               int access,
                               int w,
                               int h);

struct tinyui_runtime_bridge_backend_runtime_state {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    COLOUR_INT *real_pixels;
    uint32_t *present_pixels;
    arm_2d_tile_t real_tile;
    uint32_t start_ticks;
    uint32_t screen_create_start_ticks;
    uint32_t screen_create_end_ticks;
    uint32_t auto_quit_ms;
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

#ifndef SDL_INIT_VIDEO
#define SDL_INIT_VIDEO 0x00000020u
#endif

#ifndef SDL_INIT_EVENTS
#define SDL_INIT_EVENTS 0x00004000u
#endif

#ifndef SDL_WINDOWPOS_CENTERED
#define SDL_WINDOWPOS_CENTERED 0x2FFF0000u
#endif

#ifndef SDL_WINDOW_SHOWN
#define SDL_WINDOW_SHOWN 0x00000004u
#endif

#ifndef SDL_RENDERER_SOFTWARE
#define SDL_RENDERER_SOFTWARE 0x00000001u
#endif

#ifndef SDL_RENDERER_ACCELERATED
#define SDL_RENDERER_ACCELERATED 0x00000002u
#endif

#ifndef SDL_PIXELFORMAT_ARGB8888
#define SDL_PIXELFORMAT_ARGB8888 372645892u
#endif

#ifndef SDL_TEXTUREACCESS_STREAMING
#define SDL_TEXTUREACCESS_STREAMING 1
#endif

static unsigned int tinyui_runtime_bridge_default_tick_source(void *user_data)
{
    (void)user_data;
    return (unsigned int)SDL_GetTicks();
}

static void tinyui_runtime_bridge_default_delay(unsigned int ms, void *user_data)
{
    (void)user_data;
    SDL_Delay((uint32_t)ms);
}

static int tinyui_runtime_bridge_ensure_window_from_state(
    struct picoui_app *app,
    struct tinyui_runtime_bridge_backend_runtime_state *state)
{
    struct picoui_display_config display = {0};

    if (state == NULL) {
        return -1;
    }

    if (picoui_display_get_config(app, &display) != 0) {
        return -1;
    }

    state->display_width = display.width;
    state->display_height = display.height;

    if (state->window != NULL && state->renderer != NULL && state->texture != NULL
        && state->real_pixels != NULL && state->present_pixels != NULL) {
        return 0;
    }

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

void ldBaseNodeRemove(arm_2d_control_node_t *ptNode);
int tinyui_runtime_bridge_bind_ld_event_bridge(void *backend_widget,
                                               struct ld_scene_t *scene,
                                               void *sender);

static bool tinyui_runtime_bridge_ld_event_bridge_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend = NULL;

    (void)scene;

    if (msg.ptSender == NULL) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL) {
        return false;
    }

    tinyui_widget_dispatch_native_signal(backend, msg.signal, msg.value);
    return false;
}

static int tinyui_runtime_bridge_connect_native_events(struct picoui_backend_widget *backend)
{
    uint8_t primary_signal = SIGNAL_NO_OPERATION;
    uint8_t secondary_signal = SIGNAL_NO_OPERATION;
    uint8_t tertiary_signal = SIGNAL_NO_OPERATION;
    ldBase_t *sender = NULL;
    ldAssn_t *assn = NULL;

    if (backend == NULL || backend->ld_widget == NULL) {
        return -1;
    }

    sender = (ldBase_t *)backend->ld_widget;
    sender->pInfo = backend;

    switch (backend->kind) {
    case PICOUI_BACKEND_WIDGET_BUTTON:
        primary_signal = SIGNAL_PRESS;
        secondary_signal = SIGNAL_RELEASE;
        tertiary_signal = SIGNAL_HOLD_DOWN;
        break;
    case PICOUI_BACKEND_WIDGET_LIST:
        primary_signal = SIGNAL_CLICKED_ITEM;
        break;
    case PICOUI_BACKEND_WIDGET_CHECKBOX:
    case PICOUI_BACKEND_WIDGET_SWITCH:
    case PICOUI_BACKEND_WIDGET_SLIDER:
        primary_signal = SIGNAL_VALUE_CHANGED;
        break;
    default:
        return 0;
    }

    assn = sender->ptAssn;
    while (assn != NULL) {
        if (assn->signal == primary_signal && assn->pFunc == tinyui_runtime_bridge_ld_event_bridge_slot) {
            primary_signal = SIGNAL_NO_OPERATION;
            break;
        }
        assn = assn->ptNext;
    }
    if (primary_signal != SIGNAL_NO_OPERATION
        && !ldMsgConnect(sender, primary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
        return -1;
    }
    if (secondary_signal != SIGNAL_NO_OPERATION) {
        assn = sender->ptAssn;
        while (assn != NULL) {
            if (assn->signal == secondary_signal
                && assn->pFunc == tinyui_runtime_bridge_ld_event_bridge_slot) {
                secondary_signal = SIGNAL_NO_OPERATION;
                break;
            }
            assn = assn->ptNext;
        }
        if (secondary_signal != SIGNAL_NO_OPERATION
            && !ldMsgConnect(sender, secondary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
            return -1;
        }
    }
    if (tertiary_signal != SIGNAL_NO_OPERATION) {
        assn = sender->ptAssn;
        while (assn != NULL) {
            if (assn->signal == tertiary_signal
                && assn->pFunc == tinyui_runtime_bridge_ld_event_bridge_slot) {
                tertiary_signal = SIGNAL_NO_OPERATION;
                break;
            }
            assn = assn->ptNext;
        }
        if (tertiary_signal != SIGNAL_NO_OPERATION
            && !ldMsgConnect(sender, tertiary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
            return -1;
        }
    }

    return 0;
}

struct picoui_backend_app_state *tinyui_runtime_bridge_backend_state_from_parent(void *backend_widget)
{
    struct picoui_backend_widget *parent_widget = backend_widget;

    if (parent_widget == 0 || parent_widget->owner == 0) {
        return 0;
    }

    return tinyui_runtime_bridge_backend_state(parent_widget->owner);
}

struct ld_scene_t *tinyui_runtime_bridge_scene_from_parent(void *backend_widget)
{
    struct picoui_backend_app_state *app_state =
        tinyui_runtime_bridge_backend_state_from_parent(backend_widget);

    if (app_state == 0) {
        return 0;
    }

    return app_state->ld_scene;
}

uint16_t tinyui_runtime_bridge_next_name_id(void *backend_widget)
{
    struct picoui_backend_app_state *app_state =
        tinyui_runtime_bridge_backend_state_from_parent(backend_widget);

    if (app_state == 0) {
        return 0;
    }

    return ++app_state->next_ld_name_id;
}

int tinyui_runtime_bridge_bind_theme(struct picoui_app *app, struct picoui_theme *theme)
{
    struct picoui_backend_app_state *app_state = tinyui_runtime_bridge_backend_state(app);

    if (app == 0 || theme == 0 || app_state == 0) {
        return -1;
    }

    app->theme = theme;
    app_state->theme = theme;
    return 0;
}

int tinyui_runtime_bridge_init_app(struct picoui_app *app)
{
    struct tinyui_runtime_bridge_backend_runtime_state *state;
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
    app_state->runtime_state = state;
    (void)picoui_tick_set_source(app, tinyui_runtime_bridge_default_tick_source, NULL);
    (void)picoui_os_set_delay_callback(app, tinyui_runtime_bridge_default_delay, NULL);
    app->backend_app = app_state;
    return 0;
}

int tinyui_runtime_bridge_run_app(struct picoui_app *app, struct picoui_window *window)
{
    int running = 1;

    if (app == NULL || window == NULL) {
        return -1;
    }

    app->root_window = window;
    while (running) {
        int step = tinyui_runtime_bridge_step_app(app);

        if (step < 0) {
            return -1;
        }
        if (step > 0) {
            running = 0;
        }
    }

    return 0;
}

int tinyui_runtime_bridge_step_app(struct picoui_app *app)
{
    return tinyui_runtime_host_step_app(app);
}

void tinyui_runtime_bridge_shutdown_app(struct picoui_app *app)
{
    struct picoui_backend_app_state *app_state;
    struct tinyui_runtime_bridge_backend_runtime_state *state;

    if (app == NULL) {
        return;
    }

    app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state == NULL) {
        return;
    }

    state = (struct tinyui_runtime_bridge_backend_runtime_state *)app_state->runtime_state;
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
    if (app_state->ld_scene != NULL) {
        ldGuiDespose(app_state->ld_scene);
    }
    free(app_state->ld_scene);
    free(app_state);
    app->backend_app = NULL;
}

int tinyui_runtime_bridge_ensure_window(struct picoui_app *app)
{
    struct picoui_backend_app_state *app_state;

    if (app == NULL) {
        return -1;
    }

    app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state == NULL || app_state->runtime_state == NULL) {
        return -1;
    }

    return tinyui_runtime_bridge_ensure_window_from_state(
        app,
        (struct tinyui_runtime_bridge_backend_runtime_state *)app_state->runtime_state);
}

void tinyui_runtime_bridge_begin_screen_create(struct picoui_app *app)
{
    struct picoui_backend_app_state *app_state;
    struct tinyui_runtime_bridge_backend_runtime_state *state;

    app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state == NULL || app_state->runtime_state == NULL) {
        return;
    }

    state = (struct tinyui_runtime_bridge_backend_runtime_state *)app_state->runtime_state;
    state->screen_create_start_ticks = picoui_tick_get(app);
    state->screen_create_end_ticks = state->screen_create_start_ticks;
    state->benchmark_screen_create_logged = 0;
}

int16_t tinyui_runtime_bridge_map_pointer_axis(int value, int window_extent, int target_extent)
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

static int tinyui_runtime_bridge_touch_log_enabled(void)
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

int tinyui_runtime_bridge_bridge_pointer_from_port(struct picoui_app *app,
                                                   int window_width,
                                                   int window_height)
{
    struct picoui_display_config display = {0};
    int pointer_x = 0;
    int pointer_y = 0;
    int pointer_pressed = 0;
    int16_t mapped_x;
    int16_t mapped_y;

    if (app == NULL) {
        return -1;
    }

    if (picoui_input_get_pointer(app, &pointer_x, &pointer_y, &pointer_pressed) != 0) {
        return -1;
    }

    if (picoui_display_get_config(app, &display) != 0) {
        return -1;
    }

    mapped_x = tinyui_runtime_bridge_map_pointer_axis(pointer_x, window_width, display.width);
    mapped_y = tinyui_runtime_bridge_map_pointer_axis(pointer_y, window_height, display.height);
    if (tinyui_runtime_bridge_touch_log_enabled()) {
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
    return 0;
}

int tinyui_runtime_bridge_commit_pointer_event(struct picoui_app *app,
                                               int window_width,
                                               int window_height,
                                               int x,
                                               int y,
                                               int pressed)
{
    if (app == NULL) {
        return -1;
    }

    if (picoui_input_push_pointer(app, x, y, pressed) != 0) {
        return -1;
    }

    return tinyui_runtime_bridge_bridge_pointer_from_port(app, window_width, window_height);
}

int tinyui_runtime_bridge_bind_host(void *backend_widget, struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_backend_app_state *app_state = NULL;

    if (backend == 0 || widget == 0) {
        return -1;
    }

    if (tinyui_widget_bind_backend_host(widget, backend) != 0) {
        return -1;
    }
    backend->edit_result_on_finish = PICOUI_EDIT_RESULT_NONE;
    tinyui_widget_init_data_model(backend);
    app_state = tinyui_runtime_bridge_backend_state(backend->owner);
    if (app_state != NULL && app_state->ld_scene != NULL && backend->ld_widget != NULL) {
        if (tinyui_runtime_bridge_bind_ld_event_bridge(backend,
                                                       app_state->ld_scene,
                                                       backend->ld_widget) != 0) {
            return -1;
        }
    }
    return 0;
}

int tinyui_runtime_bridge_bind_ld_event_bridge(void *backend_widget,
                                               struct ld_scene_t *scene,
                                               void *sender)
{
    struct picoui_backend_widget *backend = backend_widget;

    if (backend == 0 || scene == 0 || sender == 0) {
        return -1;
    }

    if (tinyui_runtime_bridge_connect_native_events(backend) != 0) {
        return -1;
    }

    backend->ld_event_bridge_scene = scene;
    backend->ld_event_bridge_sender = sender;
    return 0;
}

int tinyui_runtime_bridge_unbind_host(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0) {
        return -1;
    }

    if (widget->ld_widget != 0) {
        ((ldBase_t *)widget->ld_widget)->pInfo = 0;
    }
    widget->host_widget = 0;
    widget->ld_event_bridge_scene = 0;
    widget->ld_event_bridge_sender = 0;
    widget->ld_event_bridge_next = 0;
    return 0;
}

int tinyui_runtime_bridge_detach_from_parent(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0) {
        return -1;
    }

    if (widget->ld_widget != 0) {
        ldBaseNodeRemove((arm_2d_control_node_t *)widget->ld_widget);
    }

    return tinyui_widget_backend_detach(widget);
}

int tinyui_runtime_bridge_has_scene(const struct picoui_app *app)
{
    return app != 0 && app->backend_app != 0;
}

struct picoui_backend_app_state *tinyui_runtime_bridge_backend_state(struct picoui_app *app)
{
    if (app == 0 || app->backend_app == 0) {
        return 0;
    }

    return (struct picoui_backend_app_state *)app->backend_app;
}

struct picoui_backend_app_state *tinyui_runtime_bridge_backend_state_from_window(struct picoui_window *window)
{
    const struct picoui_backend_widget *backend = 0;

    if (window == 0 || window->widget.backend_widget == 0) {
        return 0;
    }

    backend = (const struct picoui_backend_widget *)window->widget.backend_widget;
    return tinyui_runtime_bridge_backend_state(backend->owner);
}

int tinyui_runtime_bridge_window_is_owned_by(const struct picoui_app *app,
                                             const struct picoui_window *window)
{
    const struct picoui_backend_widget *backend = 0;

    if (app == 0 || window == 0 || window->widget.backend_widget == 0) {
        return 0;
    }

    backend = (const struct picoui_backend_widget *)window->widget.backend_widget;
    return backend->owner == app;
}

void tinyui_runtime_bridge_reset_window_switch(struct picoui_app *app)
{
    struct picoui_backend_app_state *app_state = tinyui_runtime_bridge_backend_state(app);

    if (app_state == 0) {
        return;
    }

    app_state->last_window_switch_mode = 0;
    app_state->last_window_switch_duration_ms = 0;
}

void tinyui_runtime_bridge_set_window_switch(struct picoui_app *app,
                                             int mode,
                                             unsigned int duration_ms)
{
    struct picoui_backend_app_state *app_state = tinyui_runtime_bridge_backend_state(app);

    if (app_state == 0) {
        return;
    }

    app_state->last_window_switch_mode = mode;
    app_state->last_window_switch_duration_ms = duration_ms;
}
