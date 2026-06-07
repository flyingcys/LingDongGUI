#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_PROGRESS_WHEEL_RENDER_READY (1u << 23)
#define PICOUI_NATIVE_PROGRESS_WHEEL_DOT_ENABLED (1u << 24)

static struct picoui_backend_widget *picoui_native_progress_wheel_backend(
    const struct picoui_progress_wheel *wheel)
{
    if (wheel == 0 || wheel->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)wheel->widget.backend_widget;
}

int picoui_backend_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent);
int picoui_backend_progress_wheel_set_dot_enabled(void *backend_widget, int enabled);

void picoui_native_progress_wheel_reset_render_state(struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend = picoui_native_progress_wheel_backend(wheel);

    if (backend == 0) {
        return;
    }

    backend->runtime_evidence_flags &= ~(PICOUI_NATIVE_PROGRESS_WHEEL_RENDER_READY
                                         | PICOUI_NATIVE_PROGRESS_WHEEL_DOT_ENABLED);
}

int picoui_native_progress_wheel_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_progress_wheel *wheel;
    unsigned int flags;
    struct picoui_progress_wheel *mutable_wheel;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL
        || backend->host_widget == 0) {
        return -1;
    }

    wheel = (const struct picoui_progress_wheel *)backend->host_widget;
    mutable_wheel = (struct picoui_progress_wheel *)backend->host_widget;
    if (picoui_backend_progress_wheel_set_percent(mutable_wheel, wheel->percent) != 0
        || picoui_backend_progress_wheel_set_dot_enabled(mutable_wheel->widget.backend_widget,
                                                         wheel->dot_enabled) != 0) {
        return -1;
    }
    flags = backend->runtime_evidence_flags & ~PICOUI_NATIVE_PROGRESS_WHEEL_DOT_ENABLED;
    flags |= PICOUI_NATIVE_PROGRESS_WHEEL_RENDER_READY;
    if (wheel->dot_enabled != 0) {
        flags |= PICOUI_NATIVE_PROGRESS_WHEEL_DOT_ENABLED;
    }
    ((struct picoui_backend_widget *)backend)->runtime_evidence_flags = flags;
    return 0;
}

int picoui_native_progress_wheel_get_rendered_state(const struct picoui_progress_wheel *wheel,
                                                    int *percent,
                                                    int *dot_enabled)
{
    const struct picoui_backend_widget *backend = picoui_native_progress_wheel_backend(wheel);

    if (percent == 0 || dot_enabled == 0) {
        return -1;
    }

    if (backend == 0
        || (backend->runtime_evidence_flags & PICOUI_NATIVE_PROGRESS_WHEEL_RENDER_READY) == 0) {
        return -1;
    }

    *percent = wheel->percent;
    *dot_enabled =
        (backend->runtime_evidence_flags & PICOUI_NATIVE_PROGRESS_WHEEL_DOT_ENABLED) != 0 ? 1 : 0;
    return 0;
}
