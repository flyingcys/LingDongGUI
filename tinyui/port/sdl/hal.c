/* tinyui/port/sdl/hal.c
 * SDL 平台适配层 — 对应 lv_port_pc_vscode 的 hal_init()。
 * 只做 SDL 调用：窗口生命周期、输入事件泵、像素呈现。
 * 不调用任何 LingDongGUI / arm_2d 渲染 API（如 arm_2d_helper_init、arm_2d_*绘制函数）。
 * arm_2d_tile_t 用于描述像素缓冲区格式（数据结构），不属于渲染调用。
 */
#include "host_internal.h"
#include "display/display.h"
#include "indev/indev.h"
#include "tick/tick.h"
#include "osal/osal.h"
#include "tinyui_sdl.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arm_2d_disp_adapter_0.h"

unsigned int tinyui_runtime_host_default_tick_source(void *user_data)
{
    (void)user_data;
    return (unsigned int)SDL_GetTicks();
}

void tinyui_runtime_host_default_delay(unsigned int ms, void *user_data)
{
    (void)user_data;
    SDL_Delay((Uint32)ms);
}

uint32_t tinyui_runtime_host_pixel_to_rgb888(COLOUR_INT pixel)
{
#if __DISP0_CFG_COLOUR_DEPTH__ == 16
    uint32_t red = ((uint32_t)pixel >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)pixel >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)pixel & 0x1FU;

    red <<= 3;
    green <<= 2;
    blue <<= 3;
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

void tinyui_runtime_host_copy_flush_pixels(const struct tinyui_area *area,
                                           const void *pixels,
                                           void *user_data)
{
    struct tinyui_runtime_host_state *state = (struct tinyui_runtime_host_state *)user_data;
    const COLOUR_INT *source = (const COLOUR_INT *)pixels;
    int src_x0;
    int src_y0;
    int dst_x0;
    int dst_y0;
    int copy_width;
    int copy_height;
    int row;

    if (area == NULL || source == NULL || state == NULL || state->real_pixels == NULL ||
        state->display_width <= 0 || state->display_height <= 0 ||
        area->width <= 0 || area->height <= 0) {
        return;
    }

    src_x0 = 0;
    src_y0 = 0;
    dst_x0 = area->x;
    dst_y0 = area->y;
    copy_width = area->width;
    copy_height = area->height;

    if (dst_x0 < 0) {
        src_x0 = -dst_x0;
        copy_width -= src_x0;
        dst_x0 = 0;
    }
    if (dst_y0 < 0) {
        src_y0 = -dst_y0;
        copy_height -= src_y0;
        dst_y0 = 0;
    }
    if (dst_x0 + copy_width > state->display_width) {
        copy_width = state->display_width - dst_x0;
    }
    if (dst_y0 + copy_height > state->display_height) {
        copy_height = state->display_height - dst_y0;
    }
    if (copy_width <= 0 || copy_height <= 0) {
        return;
    }

    for (row = 0; row < copy_height; ++row) {
        const COLOUR_INT *source_row =
            source + (size_t)(src_y0 + row) * (size_t)area->width + (size_t)src_x0;
        COLOUR_INT *dest_row =
            state->real_pixels + (size_t)(dst_y0 + row) * (size_t)state->display_width +
            (size_t)dst_x0;
        memcpy(dest_row, source_row, (size_t)copy_width * sizeof(*dest_row));
    }
}

int tinyui_runtime_host_ensure_window(struct tinyui_app *app,
                                      struct tinyui_runtime_host_state *state)
{
    struct tinyui_display_config display = {0};

    if (app == NULL || state == NULL) {
        return -1;
    }

    if (tinyui_display_get_config(app, &display) != 0) {
        return -1;
    }

    state->display_width = display.width;
    state->display_height = display.height;

    if (state->window != NULL && state->renderer != NULL && state->texture != NULL
        && state->real_pixels != NULL && state->present_pixels != NULL) {
        return 0;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "TINYUI runtime SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    state->window = SDL_CreateWindow("TINYUI Demo",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     state->display_width,
                                     state->display_height,
                                     SDL_WINDOW_SHOWN);
    if (state->window == NULL) {
        fprintf(stderr, "TINYUI runtime SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_ACCELERATED);
    if (state->renderer == NULL) {
        state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (state->renderer == NULL) {
        fprintf(stderr, "TINYUI runtime SDL_CreateRenderer failed: %s\n", SDL_GetError());
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
        fprintf(stderr, "TINYUI runtime SDL_CreateTexture failed: %s\n", SDL_GetError());
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

    state->start_ticks = tinyui_tick_get(app);
    state->screen_create_start_ticks = state->start_ticks;
    state->screen_create_end_ticks = state->start_ticks;
    state->benchmark_screen_create_logged = 0;
    return 0;
}

void tinyui_runtime_host_present_real_frame(struct tinyui_runtime_host_state *state)
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

int tinyui_runtime_host_pump_sdl_events(struct tinyui_app *app,
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
                printf("[TINYUI_TOUCH][SDL] type=down button=%u pos=(%d,%d)\n",
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
                printf("[TINYUI_TOUCH][SDL] type=up button=%u pos=(%d,%d)\n",
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
                printf("[TINYUI_TOUCH][SDL] type=motion buttons=0x%x pos=(%d,%d)\n",
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

/* ─── LVGL 式平台驱动入口 ───────────────────────────────────────────────────
 * 单窗口/单实例模型(匹配 disp adapter 的单实例 PFB)。core 的帧循环
 * (tinyui_backend_neutral_step)每帧回调下面的 read_cb / present_cb;
 * 窗口/缓冲生命周期由 window_create / quit 管理。
 * ──────────────────────────────────────────────────────────────────────── */
static struct tinyui_runtime_host_state s_sdl_state;

/* present_cb:一帧渲染完成后把 real_pixels 上屏(等价旧 step.c 的 render 尾段)。 */
static void tinyui_sdl_present_cb(void *user_data)
{
    struct tinyui_runtime_host_state *state = (struct tinyui_runtime_host_state *)user_data;

    if (state == NULL || state->renderer == NULL) {
        return;
    }

    SDL_SetRenderDrawColor(state->renderer, 0x2E, 0x34, 0x40, 0xFF);
    SDL_RenderClear(state->renderer);
    tinyui_runtime_host_present_real_frame(state);
    SDL_RenderPresent(state->renderer);

    state->rendered_frames += 1U;
    (void)tinyui_runtime_host_write_capture(state);
}

/* read_cb:泵 SDL 事件 → push 指针;SDL_QUIT 或到达 auto-quit 时限时请求退出。 */
static int tinyui_sdl_read_cb(struct tinyui_app *app, void *user_data)
{
    struct tinyui_runtime_host_state *state = (struct tinyui_runtime_host_state *)user_data;
    int event_result;

    event_result = tinyui_runtime_host_pump_sdl_events(app, state);
    if (event_result != 0) {
        return event_result;   /* SDL_QUIT → >0 */
    }

    if (state->auto_quit_ms > 0 &&
        tinyui_tick_get(app) - state->start_ticks >= state->auto_quit_ms) {
        return 1;
    }

    return 0;
}

int tinyui_sdl_window_create(int width, int height)
{
    struct tinyui_app *app = tinyui_app_current();
    struct tinyui_display_config cfg = {0};

    if (app == NULL || width <= 0 || height <= 0) {
        return -1;
    }

    /* 沿用已设显示配置(格式/PFB 行高),仅以入参锁定分辨率。 */
    (void)tinyui_display_get_config(app, &cfg);
    cfg.width = width;
    cfg.height = height;
    if (tinyui_display_set_config(app, &cfg) != 0) {
        return -1;
    }

    /* 先装 tick,ensure_window 里的 start_ticks 才有意义。 */
    if (app->tick_port.callback == NULL) {
        (void)tinyui_tick_set_source(app, tinyui_runtime_host_default_tick_source, NULL);
    }
    if (app->os_port.delay == NULL) {
        (void)tinyui_os_set_delay_callback(app, tinyui_runtime_host_default_delay, NULL);
    }

    s_sdl_state.display_width = width;
    s_sdl_state.display_height = height;
    if (tinyui_runtime_host_ensure_window(app, &s_sdl_state) != 0) {
        return -1;
    }
    s_sdl_state.auto_quit_ms = tinyui_runtime_host_parse_auto_quit_ms();

    (void)tinyui_display_set_flush_callback(app, tinyui_runtime_host_copy_flush_pixels, &s_sdl_state);
    (void)tinyui_display_set_present_callback(app, tinyui_sdl_present_cb, &s_sdl_state);
    return 0;
}

int tinyui_sdl_mouse_create(void)
{
    struct tinyui_app *app = tinyui_app_current();

    if (app == NULL) {
        return -1;
    }

    return tinyui_input_set_read_callback(app, tinyui_sdl_read_cb, &s_sdl_state);
}

void tinyui_sdl_quit(void)
{
    struct tinyui_runtime_host_state *state = &s_sdl_state;

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

    /* 复位整个单实例 state,回到程序启动时的零值。旧 step.c 的 shutdown_app
     * 会 free 掉 heap 上的 runtime_state,下次运行 calloc 出全新零值 state;
     * 现在 state 是文件静态单例,必须显式清零,否则 capture_written /
     * rendered_frames / *_logged 等运行期字段残留,导致 quit 后再次
     * window_create() 的二次运行不出图(write_capture 永久早退)。 */
    memset(state, 0, sizeof(*state));
}
