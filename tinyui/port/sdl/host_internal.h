/* tinyui/port/sdl/host_internal.h
 * 私有头，只由 hal.c / observe.c / step.c 引用，不对外安装。
 */
#ifndef TINYUI_PORT_SDL_HOST_INTERNAL_H
#define TINYUI_PORT_SDL_HOST_INTERNAL_H

#include "internal.h"
#include "runtime_bridge.h"
#include <SDL.h>
#include "arm_2d.h"
#include "ldGui.h"

/* 共享 state — 每个 tinyui_app 实例一个，存在 app_state->runtime_state */
struct tinyui_runtime_host_state {
    SDL_Window    *window;
    SDL_Renderer  *renderer;
    SDL_Texture   *texture;
    COLOUR_INT    *real_pixels;
    uint32_t      *present_pixels;
    arm_2d_tile_t  real_tile;
    Uint32         start_ticks;
    Uint32         screen_create_start_ticks;
    Uint32         screen_create_end_ticks;
    Uint32         auto_quit_ms;
    int            display_width;
    int            display_height;
    int            ready_logged;
    int            capture_written;
    int            static_mapping_logged;
    int            real_widget_ids_logged;
    int            fallback_boundary_logged;
    int            temporary_smoke_logged;
    int            smoke_layout_used;
    int            smoke_layout_marker_logged;
    int            benchmark_screen_create_logged;
    int            benchmark_first_frame_logged;
};

/* ---- hal.c → 跨文件声明 ---- */
unsigned int tinyui_runtime_host_default_tick_source(void *user_data);
void         tinyui_runtime_host_default_delay(unsigned int ms, void *user_data);
uint32_t     tinyui_runtime_host_pixel_to_rgb888(COLOUR_INT pixel);
int          tinyui_runtime_host_ensure_window(struct tinyui_app *app,
                                               struct tinyui_runtime_host_state *state);
void         tinyui_runtime_host_present_real_frame(struct tinyui_runtime_host_state *state);
int          tinyui_runtime_host_pump_sdl_events(struct tinyui_app *app,
                                                 struct tinyui_runtime_host_state *state);

/* ---- observe.c → 跨文件声明 ---- */
int    tinyui_runtime_host_touch_log_enabled(void);
void   tinyui_runtime_host_log_screen_create_benchmark(struct tinyui_runtime_host_state *state);
void   tinyui_runtime_host_log_first_frame_benchmark(struct tinyui_runtime_host_state *state);
int    tinyui_runtime_host_window_has_real_layout(const struct tinyui_widget *widget);
void   tinyui_runtime_host_log_mapping_markers(struct tinyui_runtime_host_state *state,
                                               const struct tinyui_widget *root_widget);
void   tinyui_runtime_host_log_image_source_marker(const struct tinyui_widget *widget);
Uint32 tinyui_runtime_host_parse_auto_quit_ms(void);
int    tinyui_runtime_host_write_capture(struct tinyui_runtime_host_state *state);
void   tinyui_runtime_host_log_runtime_ready(struct tinyui_app *app,
                                             struct tinyui_runtime_host_state *state);
void   tinyui_runtime_host_log_smoke_layout_marker(struct tinyui_runtime_host_state *state);

#endif /* TINYUI_PORT_SDL_HOST_INTERNAL_H */
