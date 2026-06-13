#ifndef PICOUI_RUNTIME_BRIDGE_H
#define PICOUI_RUNTIME_BRIDGE_H

#include <stdint.h>

struct picoui_app;
struct picoui_theme;
struct picoui_window;
struct picoui_backend_app_state;

int tinyui_runtime_bridge_has_scene(const struct picoui_app *app);
struct picoui_backend_app_state *tinyui_runtime_bridge_backend_state(struct picoui_app *app);
struct picoui_backend_app_state *tinyui_runtime_bridge_backend_state_from_window(struct picoui_window *window);
int tinyui_runtime_bridge_window_is_owned_by(const struct picoui_app *app,
                                             const struct picoui_window *window);
struct picoui_backend_app_state *tinyui_runtime_bridge_backend_state_from_parent(void *backend_widget);
struct ld_scene_t *tinyui_runtime_bridge_scene_from_parent(void *backend_widget);
uint16_t tinyui_runtime_bridge_next_name_id(void *backend_widget);
int tinyui_runtime_bridge_bind_theme(struct picoui_app *app, struct picoui_theme *theme);
void tinyui_runtime_bridge_reset_window_switch(struct picoui_app *app);
void tinyui_runtime_bridge_set_window_switch(struct picoui_app *app,
                                             int mode,
                                             unsigned int duration_ms);
int tinyui_runtime_bridge_init_app(struct picoui_app *app);
int tinyui_runtime_bridge_run_app(struct picoui_app *app, struct picoui_window *window);
void tinyui_runtime_bridge_shutdown_app(struct picoui_app *app);
void tinyui_runtime_bridge_begin_screen_create(struct picoui_app *app);
int tinyui_runtime_bridge_ensure_window(struct picoui_app *app);
int tinyui_runtime_bridge_step_app(struct picoui_app *app);
int16_t tinyui_runtime_bridge_map_pointer_axis(int value,
                                               int window_extent,
                                               int target_extent);
int tinyui_runtime_bridge_bridge_pointer_from_port(struct picoui_app *app,
                                                   int window_width,
                                                   int window_height);
int tinyui_runtime_bridge_commit_pointer_event(struct picoui_app *app,
                                               int window_width,
                                               int window_height,
                                               int x,
                                               int y,
                                               int pressed);

#endif
