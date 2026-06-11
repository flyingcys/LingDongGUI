#include "internal.h"
#include "runtime_bridge.h"
#include "../../../../src/gui/ldBase.h"
#include "../../../../src/gui/ldButton.h"
#include "../../../../src/gui/ldCheckBox.h"
#include "../../../../src/gui/ldList.h"
#include "../../../../src/gui/ldSlider.h"
#include "../../../../src/gui/ldSwitch.h"
#include "../../../../src/misc/ldMsg.h"

void ldBaseNodeRemove(arm_2d_control_node_t *ptNode);
int picoui_backend_widget_bind_ld_event_bridge(void *backend_widget,
                                               struct ld_scene_t *scene,
                                               void *sender);

static bool picoui_runtime_bridge_ld_event_bridge_slot(struct ld_scene_t *scene, ldMsg_t msg)
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

    picoui_backend_widget_dispatch_native_signal(backend, msg.signal, msg.value);
    return false;
}

static int picoui_runtime_bridge_connect_native_events(struct picoui_backend_widget *backend)
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
        if (assn->signal == primary_signal && assn->pFunc == picoui_runtime_bridge_ld_event_bridge_slot) {
            primary_signal = SIGNAL_NO_OPERATION;
            break;
        }
        assn = assn->ptNext;
    }
    if (primary_signal != SIGNAL_NO_OPERATION
        && !ldMsgConnect(sender, primary_signal, picoui_runtime_bridge_ld_event_bridge_slot)) {
        return -1;
    }
    if (secondary_signal != SIGNAL_NO_OPERATION) {
        assn = sender->ptAssn;
        while (assn != NULL) {
            if (assn->signal == secondary_signal
                && assn->pFunc == picoui_runtime_bridge_ld_event_bridge_slot) {
                secondary_signal = SIGNAL_NO_OPERATION;
                break;
            }
            assn = assn->ptNext;
        }
        if (secondary_signal != SIGNAL_NO_OPERATION
            && !ldMsgConnect(sender, secondary_signal, picoui_runtime_bridge_ld_event_bridge_slot)) {
            return -1;
        }
    }
    if (tertiary_signal != SIGNAL_NO_OPERATION) {
        assn = sender->ptAssn;
        while (assn != NULL) {
            if (assn->signal == tertiary_signal
                && assn->pFunc == picoui_runtime_bridge_ld_event_bridge_slot) {
                tertiary_signal = SIGNAL_NO_OPERATION;
                break;
            }
            assn = assn->ptNext;
        }
        if (tertiary_signal != SIGNAL_NO_OPERATION
            && !ldMsgConnect(sender, tertiary_signal, picoui_runtime_bridge_ld_event_bridge_slot)) {
            return -1;
        }
    }

    return 0;
}

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

int picoui_backend_widget_bind_host(void *backend_widget, struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_backend_app_state *app_state = NULL;

    if (backend == 0 || widget == 0) {
        return -1;
    }

    if (picoui_widget_bind_backend_host(widget, backend) != 0) {
        return -1;
    }
    backend->edit_result_on_finish = PICOUI_EDIT_RESULT_NONE;
    picoui_backend_widget_init_data_model(backend);
    app_state = picoui_runtime_bridge_backend_state(backend->owner);
    if (app_state != NULL && app_state->ld_scene != NULL && backend->ld_widget != NULL) {
        if (picoui_backend_widget_bind_ld_event_bridge(backend,
                                                       app_state->ld_scene,
                                                       backend->ld_widget) != 0) {
            return -1;
        }
    }
    return 0;
}

int picoui_backend_widget_bind_ld_event_bridge(void *backend_widget,
                                               struct ld_scene_t *scene,
                                               void *sender)
{
    struct picoui_backend_widget *backend = backend_widget;

    if (backend == 0 || scene == 0 || sender == 0) {
        return -1;
    }

    if (picoui_runtime_bridge_connect_native_events(backend) != 0) {
        return -1;
    }

    backend->ld_event_bridge_scene = scene;
    backend->ld_event_bridge_sender = sender;
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
