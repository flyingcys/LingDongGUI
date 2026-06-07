#include "runtime_state.h"
#include "picoui/core.h"
#include "picoui/screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static struct picoui_runtime_state g_picoui_runtime_state;
static int g_picoui_v1_1_ready_logged;
static int g_picoui_v1_1_auto_quit_enabled;
static unsigned long g_picoui_v1_1_auto_quit_ms;
static struct timespec g_picoui_v1_1_start_ticks;

int picoui_v1_1_render_active_screen(void);

static unsigned long picoui_v1_1_parse_auto_quit_ms(void)
{
    const char *value = getenv("PICOUI_DEMO_AUTO_QUIT_MS");
    char *end = 0;
    unsigned long parsed;

    if (value == 0 || value[0] == '\0') {
        return 0UL;
    }

    parsed = strtoul(value, &end, 10);
    if (end == value || (end != 0 && *end != '\0')) {
        return 0UL;
    }

    if (parsed > 60000UL) {
        parsed = 60000UL;
    }
    return parsed;
}

static int picoui_v1_1_now(struct timespec *value)
{
    if (value == 0) {
        return -1;
    }
#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, value) == 0) {
        return 0;
    }
#endif
    return clock_gettime(CLOCK_REALTIME, value);
}

static unsigned long picoui_v1_1_elapsed_ms(struct timespec start_ticks, struct timespec now_ticks)
{
    time_t delta_sec;
    long delta_nsec;

    if (now_ticks.tv_sec < start_ticks.tv_sec
        || (now_ticks.tv_sec == start_ticks.tv_sec && now_ticks.tv_nsec <= start_ticks.tv_nsec)) {
        return 0UL;
    }

    delta_sec = now_ticks.tv_sec - start_ticks.tv_sec;
    delta_nsec = now_ticks.tv_nsec - start_ticks.tv_nsec;
    if (delta_nsec < 0) {
        delta_sec -= 1;
        delta_nsec += 1000000000L;
    }

    return (unsigned long)delta_sec * 1000UL + (unsigned long)(delta_nsec / 1000000L);
}

struct picoui_runtime_state *picoui_runtime_state(void)
{
    return &g_picoui_runtime_state;
}

int picoui_init(void)
{
    struct picoui_runtime_state *state = picoui_runtime_state();
    struct timespec now = { 0, 0 };

    memset(state, 0, sizeof(*state));
    if (picoui_v1_1_now(&now) != 0) {
        return -1;
    }

    g_picoui_v1_1_ready_logged = 0;
    g_picoui_v1_1_auto_quit_ms = picoui_v1_1_parse_auto_quit_ms();
    g_picoui_v1_1_auto_quit_enabled = g_picoui_v1_1_auto_quit_ms > 0UL;
    g_picoui_v1_1_start_ticks = now;
    state->initialized = 1;
    return 0;
}

void picoui_deinit(void)
{
    struct picoui_runtime_state *state = picoui_runtime_state();

    memset(state, 0, sizeof(*state));
    g_picoui_v1_1_ready_logged = 0;
    g_picoui_v1_1_auto_quit_enabled = 0;
    g_picoui_v1_1_auto_quit_ms = 0UL;
    g_picoui_v1_1_start_ticks.tv_sec = 0;
    g_picoui_v1_1_start_ticks.tv_nsec = 0;
}

int picoui_timer_handler(void)
{
    struct timespec now = { 0, 0 };

    if (!picoui_runtime_state()->initialized) {
        return -1;
    }

    if (!g_picoui_v1_1_ready_logged) {
        printf("PICOUI_RUNTIME_READY\n");
        fflush(stdout);
        g_picoui_v1_1_ready_logged = 1;
    }

    if (picoui_v1_1_render_active_screen() != 0) {
        return -1;
    }

    if (!g_picoui_v1_1_auto_quit_enabled) {
        return 0;
    }
    if (picoui_v1_1_now(&now) != 0) {
        return -1;
    }
    if (picoui_v1_1_elapsed_ms(g_picoui_v1_1_start_ticks, now) >= g_picoui_v1_1_auto_quit_ms) {
        return 1;
    }

    return 0;
}
