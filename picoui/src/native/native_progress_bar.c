#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_PROGRESS_BAR_RENDER_READY (1u << 23)
#define PICOUI_NATIVE_PROGRESS_BAR_HORIZONTAL (1u << 24)
#define PICOUI_NATIVE_PROGRESS_BAR_INVERTED (1u << 25)

static struct picoui_backend_widget *picoui_native_progress_bar_backend(
    const struct picoui_progress_bar *bar)
{
    if (bar == 0 || bar->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)bar->widget.backend_widget;
}

int picoui_backend_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent);
int picoui_backend_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal);
int picoui_backend_progress_bar_set_inverted(void *backend_widget, int inverted);

void picoui_native_progress_bar_reset_render_state(struct picoui_progress_bar *bar)
{
    struct picoui_backend_widget *backend = picoui_native_progress_bar_backend(bar);

    if (backend == 0) {
        return;
    }

    backend->runtime_evidence_flags &= ~(PICOUI_NATIVE_PROGRESS_BAR_RENDER_READY
                                         | PICOUI_NATIVE_PROGRESS_BAR_HORIZONTAL
                                         | PICOUI_NATIVE_PROGRESS_BAR_INVERTED);
}

int picoui_native_progress_bar_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_progress_bar *bar;
    unsigned int flags;
    struct picoui_progress_bar *mutable_bar;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_PROGRESS_BAR
        || backend->host_widget == 0) {
        return -1;
    }

    bar = (const struct picoui_progress_bar *)backend->host_widget;
    mutable_bar = (struct picoui_progress_bar *)backend->host_widget;
    if (picoui_backend_progress_bar_set_percent(mutable_bar, bar->percent) != 0
        || picoui_backend_progress_bar_set_horizontal(mutable_bar, bar->horizontal) != 0
        || picoui_backend_progress_bar_set_inverted(mutable_bar->widget.backend_widget, bar->inverted) != 0) {
        return -1;
    }
    flags = backend->runtime_evidence_flags
        & ~(PICOUI_NATIVE_PROGRESS_BAR_HORIZONTAL | PICOUI_NATIVE_PROGRESS_BAR_INVERTED);
    flags |= PICOUI_NATIVE_PROGRESS_BAR_RENDER_READY;
    if (bar->horizontal != 0) {
        flags |= PICOUI_NATIVE_PROGRESS_BAR_HORIZONTAL;
    }
    if (bar->inverted != 0) {
        flags |= PICOUI_NATIVE_PROGRESS_BAR_INVERTED;
    }
    ((struct picoui_backend_widget *)backend)->runtime_evidence_flags = flags;
    return 0;
}

int picoui_native_progress_bar_get_rendered_state(const struct picoui_progress_bar *bar,
                                                  int *percent,
                                                  int *horizontal,
                                                  int *inverted)
{
    const struct picoui_backend_widget *backend = picoui_native_progress_bar_backend(bar);

    if (percent == 0 || horizontal == 0 || inverted == 0) {
        return -1;
    }

    if (backend == 0
        || (backend->runtime_evidence_flags & PICOUI_NATIVE_PROGRESS_BAR_RENDER_READY) == 0) {
        return -1;
    }

    *percent = bar->percent;
    *horizontal =
        (backend->runtime_evidence_flags & PICOUI_NATIVE_PROGRESS_BAR_HORIZONTAL) != 0 ? 1 : 0;
    *inverted =
        (backend->runtime_evidence_flags & PICOUI_NATIVE_PROGRESS_BAR_INVERTED) != 0 ? 1 : 0;
    return 0;
}
