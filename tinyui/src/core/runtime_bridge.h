#ifndef PICOUI_RUNTIME_BRIDGE_H
#define PICOUI_RUNTIME_BRIDGE_H

struct picoui_app;
struct picoui_window;
struct picoui_backend_app_state;

int picoui_runtime_bridge_has_scene(const struct picoui_app *app);
struct picoui_backend_app_state *picoui_runtime_bridge_backend_state(struct picoui_app *app);
struct picoui_backend_app_state *picoui_runtime_bridge_backend_state_from_window(struct picoui_window *window);
int picoui_runtime_bridge_window_is_owned_by(const struct picoui_app *app,
                                             const struct picoui_window *window);
struct picoui_backend_app_state *picoui_runtime_bridge_backend_state_from_parent(void *backend_widget);
struct ld_scene_t *picoui_runtime_bridge_scene_from_parent(void *backend_widget);
uint16_t picoui_runtime_bridge_next_name_id(void *backend_widget);
int picoui_runtime_bridge_bind_theme(struct picoui_app *app, struct picoui_theme *theme);
void picoui_runtime_bridge_reset_window_switch(struct picoui_app *app);
void picoui_runtime_bridge_set_window_switch(struct picoui_app *app,
                                             int mode,
                                             unsigned int duration_ms);

#endif
