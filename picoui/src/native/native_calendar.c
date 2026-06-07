#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_CALENDAR_RENDER_MAX 32

struct picoui_native_calendar_render_state {
    const struct picoui_calendar *calendar;
    int year;
    int month;
    int day;
    int selected_year;
    int selected_month;
    int selected_day;
    int rendered;
};

static struct picoui_native_calendar_render_state
    g_picoui_native_calendar_render_states[PICOUI_NATIVE_CALENDAR_RENDER_MAX];

static struct picoui_native_calendar_render_state *picoui_native_calendar_find_render_state(
    const struct picoui_calendar *calendar)
{
    int i;

    if (calendar == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_CALENDAR_RENDER_MAX; ++i) {
        if (g_picoui_native_calendar_render_states[i].calendar == calendar) {
            return &g_picoui_native_calendar_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_calendar_render_state *picoui_native_calendar_alloc_render_state(
    const struct picoui_calendar *calendar)
{
    struct picoui_native_calendar_render_state *state;
    int i;

    state = picoui_native_calendar_find_render_state(calendar);
    if (state != 0) {
        return state;
    }

    if (calendar == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_CALENDAR_RENDER_MAX; ++i) {
        if (g_picoui_native_calendar_render_states[i].calendar == 0) {
            g_picoui_native_calendar_render_states[i].calendar = calendar;
            g_picoui_native_calendar_render_states[i].rendered = 0;
            return &g_picoui_native_calendar_render_states[i];
        }
    }

    return 0;
}

int picoui_native_calendar_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_calendar *calendar;
    struct picoui_native_calendar_render_state *state;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_CALENDAR
        || backend->host_widget == 0) {
        return -1;
    }

    calendar = (const struct picoui_calendar *)backend->host_widget;
    state = picoui_native_calendar_alloc_render_state(calendar);
    if (state == 0) {
        return -1;
    }

    state->year = calendar->year;
    state->month = calendar->month;
    state->day = calendar->day;
    if (picoui_backend_calendar_get_selected_date((void *)backend,
                                                  &state->selected_year,
                                                  &state->selected_month,
                                                  &state->selected_day) != 0) {
        return -1;
    }
    state->rendered = 1;
    return 0;
}

int picoui_native_calendar_get_rendered_date(const struct picoui_calendar *calendar,
                                             int *year,
                                             int *month,
                                             int *day)
{
    struct picoui_native_calendar_render_state *state;

    if (year == 0 || month == 0 || day == 0) {
        return -1;
    }

    state = picoui_native_calendar_find_render_state(calendar);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *year = state->year;
    *month = state->month;
    *day = state->day;
    return 0;
}

int picoui_native_calendar_get_rendered_selected_date(const struct picoui_calendar *calendar,
                                                      int *year,
                                                      int *month,
                                                      int *day)
{
    struct picoui_native_calendar_render_state *state;

    if (year == 0 || month == 0 || day == 0) {
        return -1;
    }

    state = picoui_native_calendar_find_render_state(calendar);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *year = state->selected_year;
    *month = state->selected_month;
    *day = state->selected_day;
    return 0;
}
