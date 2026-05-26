#include "backend.h"
#include "../../../../src/gui/ldBase.h"

static void picoui_backend_emit_ld_event_bridge(struct picoui_backend_widget *backend,
                                                enum picoui_backend_signal signal,
                                                int value)
{
    if (backend == 0 || backend->ld_event_bridge_scene == 0 || backend->ld_event_bridge_sender == 0) {
        return;
    }

    if (signal != PICOUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        return;
    }

    if (backend->ld_event_bridge_scene->ptMsgQueue == 0) {
        return;
    }

    ldMsgEmit(backend->ld_event_bridge_scene->ptMsgQueue,
              backend->ld_event_bridge_sender,
              SIGNAL_VALUE_CHANGED,
              (uint64_t)value);
}

void picoui_backend_emit_value_changed(picoui_value_changed_cb cb,
                                       struct picoui_widget *widget,
                                       int value,
                                       void *user_data)
{
    if (cb != 0) {
        cb(widget, value, user_data);
    }
}

void picoui_backend_emit_event(picoui_event_cb cb,
                               struct picoui_widget *widget,
                               void *user_data)
{
    if (cb != 0) {
        cb(widget, user_data);
    }
}

int picoui_backend_widget_bind_ld_event_bridge(void *backend_widget,
                                               struct ld_scene_t *scene,
                                               void *sender)
{
    struct picoui_backend_widget *backend = backend_widget;

    if (backend == 0 || scene == 0 || sender == 0) {
        return -1;
    }

    backend->ld_event_bridge_scene = scene;
    backend->ld_event_bridge_sender = sender;
    return 0;
}

int picoui_backend_widget_dispatch_signal(void *backend_widget,
                                          enum picoui_backend_signal signal,
                                          int value,
                                          picoui_value_changed_cb cb,
                                          struct picoui_widget *widget,
                                          void *user_data)
{
    struct picoui_backend_widget *backend = backend_widget;

    if (backend == 0) {
        return -1;
    }

    if (signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED) {
        if (backend->value == value) {
            return 0;
        }

        backend->value = value;
        backend->last_signal = signal;
        backend->dispatch_count++;
        picoui_backend_emit_ld_event_bridge(backend, signal, value);
        picoui_backend_emit_value_changed(cb, widget, value, user_data);
        return 0;
    }

    return -1;
}

int picoui_backend_widget_dispatch_event(void *backend_widget,
                                         enum picoui_backend_signal signal,
                                         picoui_event_cb cb,
                                         struct picoui_widget *widget,
                                         void *user_data)
{
    struct picoui_backend_widget *backend = backend_widget;

    if (backend == 0) {
        return -1;
    }

    if (signal == PICOUI_BACKEND_SIGNAL_PRESSED || signal == PICOUI_BACKEND_SIGNAL_RELEASED) {
        backend->last_signal = signal;
        backend->dispatch_count++;
        picoui_backend_emit_event(cb, widget, user_data);
        return 0;
    }

    return -1;
}

int picoui_backend_widget_update_value(void *backend_widget,
                                       int value,
                                       picoui_value_changed_cb cb,
                                       struct picoui_widget *widget,
                                       void *user_data)
{
    return picoui_backend_widget_dispatch_signal(backend_widget,
                                                 PICOUI_BACKEND_SIGNAL_VALUE_CHANGED,
                                                 value,
                                                 cb,
                                                 widget,
                                                 user_data);
}

void picoui_backend_emit_clicked(picoui_event_cb cb,
                                 struct picoui_widget *widget,
                                 void *user_data)
{
    if (cb != 0) {
        cb(widget, user_data);
    }
}
