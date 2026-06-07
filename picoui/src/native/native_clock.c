#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_CLOCK_RENDER_MAX 32

struct picoui_native_clock_render_state {
    const struct picoui_clock *clock;
    int hour;
    int minute;
    int second;
    int rendered;
};

static struct picoui_native_clock_render_state
    g_picoui_native_clock_render_states[PICOUI_NATIVE_CLOCK_RENDER_MAX];

static struct picoui_native_clock_render_state *picoui_native_clock_find_render_state(
    const struct picoui_clock *clock)
{
    int i;

    if (clock == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_CLOCK_RENDER_MAX; ++i) {
        if (g_picoui_native_clock_render_states[i].clock == clock) {
            return &g_picoui_native_clock_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_clock_render_state *picoui_native_clock_alloc_render_state(
    const struct picoui_clock *clock)
{
    struct picoui_native_clock_render_state *state;
    int i;

    state = picoui_native_clock_find_render_state(clock);
    if (state != 0) {
        return state;
    }

    if (clock == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_CLOCK_RENDER_MAX; ++i) {
        if (g_picoui_native_clock_render_states[i].clock == 0) {
            g_picoui_native_clock_render_states[i].clock = clock;
            g_picoui_native_clock_render_states[i].hour = 0;
            g_picoui_native_clock_render_states[i].minute = 0;
            g_picoui_native_clock_render_states[i].second = 0;
            g_picoui_native_clock_render_states[i].rendered = 0;
            return &g_picoui_native_clock_render_states[i];
        }
    }

    return 0;
}

int picoui_native_clock_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_clock *clock;
    struct picoui_native_clock_render_state *state;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_CLOCK
        || backend->host_widget == 0) {
        return -1;
    }

    clock = (const struct picoui_clock *)backend->host_widget;
    state = picoui_native_clock_alloc_render_state(clock);
    if (state == 0) {
        return -1;
    }

    state->hour = clock->hour;
    state->minute = clock->minute;
    state->second = clock->second;
    state->rendered = 1;
    return 0;
}

int picoui_native_clock_get_rendered_time(const struct picoui_clock *clock,
                                          int *hour,
                                          int *minute,
                                          int *second)
{
    struct picoui_native_clock_render_state *state;

    if (hour == 0 || minute == 0 || second == 0) {
        return -1;
    }

    state = picoui_native_clock_find_render_state(clock);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *hour = state->hour;
    *minute = state->minute;
    *second = state->second;
    return 0;
}
