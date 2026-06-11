#include "internal.h"
#include "runtime_bridge.h"
#include "../../../../src/gui/ldBase.h"

void ldBaseNodeRemove(arm_2d_control_node_t *ptNode);

struct picoui_backend_app_state *picoui_runtime_bridge_backend_state_from_parent(void *backend_widget)
{
    struct picoui_backend_widget *parent_widget = backend_widget;

    if (parent_widget == 0 || parent_widget->owner == 0) {
        return 0;
    }

    return picoui_runtime_bridge_backend_state(parent_widget->owner);
}

struct ld_scene_t *picoui_runtime_bridge_scene_from_parent(void *backend_widget)
{
    struct picoui_backend_app_state *app_state =
        picoui_runtime_bridge_backend_state_from_parent(backend_widget);

    if (app_state == 0) {
        return 0;
    }

    return app_state->ld_scene;
}

uint16_t picoui_runtime_bridge_next_name_id(void *backend_widget)
{
    struct picoui_backend_app_state *app_state =
        picoui_runtime_bridge_backend_state_from_parent(backend_widget);

    if (app_state == 0) {
        return 0;
    }

    return ++app_state->next_ld_name_id;
}

int picoui_runtime_bridge_bind_theme(struct picoui_app *app, struct picoui_theme *theme)
{
    struct picoui_backend_app_state *app_state = picoui_runtime_bridge_backend_state(app);

    if (app == 0 || theme == 0 || app_state == 0) {
        return -1;
    }

    app->theme = theme;
    app_state->theme = theme;
    return 0;
}

int picoui_backend_widget_unbind_host(void *backend_widget)
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

int picoui_backend_widget_detach_from_parent(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0) {
        return -1;
    }

    if (widget->ld_widget != 0) {
        ldBaseNodeRemove((arm_2d_control_node_t *)widget->ld_widget);
    }

    return picoui_widget_backend_detach(widget);
}

int picoui_runtime_bridge_has_scene(const struct picoui_app *app)
{
    return app != 0 && app->backend_app != 0;
}

struct picoui_backend_app_state *picoui_runtime_bridge_backend_state(struct picoui_app *app)
{
    if (app == 0 || app->backend_app == 0) {
        return 0;
    }

    return (struct picoui_backend_app_state *)app->backend_app;
}

struct picoui_backend_app_state *picoui_runtime_bridge_backend_state_from_window(struct picoui_window *window)
{
    const struct picoui_backend_widget *backend = 0;

    if (window == 0 || window->widget.backend_widget == 0) {
        return 0;
    }

    backend = (const struct picoui_backend_widget *)window->widget.backend_widget;
    return picoui_runtime_bridge_backend_state(backend->owner);
}

int picoui_runtime_bridge_window_is_owned_by(const struct picoui_app *app,
                                             const struct picoui_window *window)
{
    const struct picoui_backend_widget *backend = 0;

    if (app == 0 || window == 0 || window->widget.backend_widget == 0) {
        return 0;
    }

    backend = (const struct picoui_backend_widget *)window->widget.backend_widget;
    return backend->owner == app;
}

void picoui_runtime_bridge_reset_window_switch(struct picoui_app *app)
{
    struct picoui_backend_app_state *app_state = picoui_runtime_bridge_backend_state(app);

    if (app_state == 0) {
        return;
    }

    app_state->last_window_switch_mode = 0;
    app_state->last_window_switch_duration_ms = 0;
}

void picoui_runtime_bridge_set_window_switch(struct picoui_app *app,
                                             int mode,
                                             unsigned int duration_ms)
{
    struct picoui_backend_app_state *app_state = picoui_runtime_bridge_backend_state(app);

    if (app_state == 0) {
        return;
    }

    app_state->last_window_switch_mode = mode;
    app_state->last_window_switch_duration_ms = duration_ms;
}
