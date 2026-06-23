/* tinyui/port/sdl/step.c
 * 帧调度层 — 对应 lv_port_pc_vscode 的 lv_timer_handler() 内部实现。
 * arm_2d 初始化、ldGui 帧循环、渲染、自动退出。
 * 对外只暴露两个函数：tinyui_runtime_host_step_app / shutdown_app。
 */
#include "host_internal.h"
#include "arm_2d_helper.h"
#include "ldConfig.h"
#include "ldBase.h"
#include "ldGui.h"
#include "arm_2d_disp_adapter_0.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static const ldPageFuncGroup_t g_tinyui_runtime_host_page = {
    .init = tinyui_runtime_host_runtime_page_init,
    .loop = NULL,
    .quit = tinyui_runtime_host_runtime_page_quit,
    .draw = NULL,
    .frameStart = NULL,
    .frameComplete = NULL,
#if (USE_LOG_LEVEL>=LOG_LEVEL_INFO)
    .pageName = "tinyui_runtime",
#endif
    .pointer = NULL,
};

static int tinyui_runtime_host_widget_is_supported_real(const struct tinyui_widget *widget)
{
    if (widget == NULL) {
        return 1;
    }

    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_BACKGROUND:
    case TINYUI_BACKEND_WIDGET_WINDOW:
    case TINYUI_BACKEND_WIDGET_LABEL:
    case TINYUI_BACKEND_WIDGET_BUTTON:
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
    case TINYUI_BACKEND_WIDGET_TEXT:
    case TINYUI_BACKEND_WIDGET_IMAGE:
    case TINYUI_BACKEND_WIDGET_SWITCH:
    case TINYUI_BACKEND_WIDGET_SLIDER:
    case TINYUI_BACKEND_WIDGET_ARC:
    case TINYUI_BACKEND_WIDGET_GAUGE:
    case TINYUI_BACKEND_WIDGET_ICON_SLIDER:
    case TINYUI_BACKEND_WIDGET_RADIAL_MENU:
    case TINYUI_BACKEND_WIDGET_PROGRESS_BAR:
    case TINYUI_BACKEND_WIDGET_QRCODE:
    case TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL:
    case TINYUI_BACKEND_WIDGET_ANIMATION:
    case TINYUI_BACKEND_WIDGET_LIST:
    case TINYUI_BACKEND_WIDGET_COMBO_BOX:
    case TINYUI_BACKEND_WIDGET_SCROLL_SELECTER:
    case TINYUI_BACKEND_WIDGET_TABLE:
    case TINYUI_BACKEND_WIDGET_GRAPH:
    case TINYUI_BACKEND_WIDGET_CALENDAR:
    case TINYUI_BACKEND_WIDGET_DATE_TIME:
    case TINYUI_BACKEND_WIDGET_MESSAGE_BOX:
    case TINYUI_BACKEND_WIDGET_CLOCK:
    case TINYUI_BACKEND_WIDGET_KEYBOARD:
        return 1;
    default:
        return 0;
    }
}

static struct tinyui_runtime_host_state *tinyui_runtime_host_state_from_app(struct tinyui_app *app)
{
    if (app == NULL) {
        return NULL;
    }
    return (struct tinyui_runtime_host_state *)app->runtime_state;
}

static struct tinyui_app *tinyui_runtime_host_app_state_from_window(struct tinyui_window *window)
{
    if (window == NULL) {
        return NULL;
    }

    return window->widget.owner;
}

static void tinyui_runtime_host_apply_smoke_cursor_layout(struct tinyui_runtime_host_state *state,
                                                     const struct tinyui_widget *root,
                                                     int x,
                                                     int *cursor_y)
{
    /* smoke layout permanently disabled */
    (void)state; (void)root; (void)x; (void)cursor_y;
}

static void tinyui_runtime_host_render(struct tinyui_runtime_host_state *state, struct tinyui_window *window)
{
    struct tinyui_app *app_state;

    SDL_SetRenderDrawColor(state->renderer, 0x2E, 0x34, 0x40, 0xFF);
    SDL_RenderClear(state->renderer);

    if (window->widget.ld_widget != NULL) {
        app_state = tinyui_runtime_host_app_state_from_window(window);
        tinyui_runtime_host_log_mapping_markers(state, &window->widget);
        if (app_state != NULL && app_state->ld_scene != NULL && state->real_pixels != NULL) {
            state->smoke_layout_used = 0;
            memset(state->real_pixels,
                   0,
                   (size_t)state->display_width * (size_t)state->display_height *
                       sizeof(*state->real_pixels));
            tinyui_runtime_host_log_smoke_layout_marker(state);
            ldGuiFrameStart(app_state->ld_scene);
            ldGuiTouchProcess(app_state->ld_scene);
            ldMsgProcess(app_state->ld_scene);
            ldGuiDraw(app_state->ld_scene, &state->real_tile, true);
            ldGuiFrameComplete(app_state->ld_scene);
            tinyui_runtime_host_log_image_source_marker(&window->widget);
            tinyui_runtime_host_present_real_frame(state);
            state->rendered_frames += 1U;
        }
    }

    SDL_RenderPresent(state->renderer);
    (void)tinyui_runtime_host_write_capture(state);
}

static int tinyui_runtime_host_prepare_runtime_state(struct tinyui_app *app,
                                                     struct tinyui_runtime_host_state **state_out)
{
    struct tinyui_runtime_host_state *state;
    struct tinyui_app *app_state;

    if (app == NULL || state_out == NULL) {
        return -1;
    }

    if (tinyui_runtime_bridge_init_app(app) != 0) {
        return -1;
    }

    if (app == NULL) {
        return -1;
    }
    app_state = app;

    state = (struct tinyui_runtime_host_state *)app_state->runtime_state;
    if (state == NULL) {
        state = calloc(1, sizeof(*state));
        if (state == NULL) {
            return -1;
        }
        state->display_width = 480;
        state->display_height = 320;
        app_state->runtime_state = state;
    }

    if (app->tick_port.callback == NULL) {
        (void)tinyui_tick_set_source(app, tinyui_runtime_host_default_tick_source, NULL);
    }
    if (app->os_port.delay == NULL) {
        (void)tinyui_os_set_delay_callback(app, tinyui_runtime_host_default_delay, NULL);
    }

    *state_out = state;
    return 0;
}

static int tinyui_runtime_host_prepare_runtime_scene(struct tinyui_app *app,
                                                     struct tinyui_window *window,
                                                     struct tinyui_app **app_state_out)
{
    struct tinyui_app *app_state;

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

static int tinyui_runtime_host_prepare_runtime(struct tinyui_app *app, struct tinyui_window *window)
{
    struct tinyui_runtime_host_state *state;
    struct tinyui_app *app_state;

    if (tinyui_runtime_host_prepare_runtime_state(app, &state) != 0) {
        return -1;
    }

    if (tinyui_runtime_host_prepare_runtime_scene(app, window, &app_state) != 0) {
        return -1;
    }

    state->auto_quit_ms = tinyui_runtime_host_parse_auto_quit_ms();

    if (tinyui_runtime_host_ensure_window(app, state) != 0) {
        return -1;
    }
    if (app_state->ld_scene->ptMsgQueue == NULL) {
        ldGuiSceneInit(app_state->ld_scene);
    }

    tinyui_runtime_host_log_runtime_ready(app, state);

    return 0;
}

int tinyui_runtime_host_step_app(struct tinyui_app *app)
{
    struct tinyui_runtime_host_state *state;
    struct tinyui_window *active_window;
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

    tinyui_app_pump_timers(app, tinyui_tick_get(app));
    tinyui_runtime_host_render(state, active_window);
    tinyui_os_delay(app, 16);

    tinyui_runtime_host_log_first_frame_benchmark(state);

    if (state->auto_quit_ms > 0 &&
        tinyui_tick_get(app) - state->start_ticks >= state->auto_quit_ms) {
        return 1;
    }

    return 0;
}

void tinyui_runtime_host_shutdown_app(struct tinyui_app *app)
{
    struct tinyui_app *app_state;
    struct tinyui_runtime_host_state *state;

    if (app == NULL) {
        return;
    }
    app_state = app;

    state = (struct tinyui_runtime_host_state *)app_state->runtime_state;
    if (state == NULL) {
        return;
    }

    free(state->present_pixels);
    free(state->real_pixels);
    state->present_pixels = NULL;
    state->real_pixels = NULL;
    if (state->texture != NULL) {
        SDL_DestroyTexture(state->texture);
        state->texture = NULL;
    }
    if (state->renderer != NULL) {
        SDL_DestroyRenderer(state->renderer);
        state->renderer = NULL;
    }
    if (state->window != NULL) {
        SDL_DestroyWindow(state->window);
        state->window = NULL;
    }
    SDL_Quit();
    free(state);
    app_state->runtime_state = NULL;
}
