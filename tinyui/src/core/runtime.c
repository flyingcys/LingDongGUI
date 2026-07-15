#include "internal.h"
#include "core/runtime.h"
#include "core/timer.h"
#include "integration/input.h"
#include "runtime_bridge.h"
#include "widgets/background.h"
#include "widgets/window.h"

struct tinyui_background *tinyui_legacy_background_create(struct tinyui_app *app, const char *id);

#include <stdlib.h>
#include <string.h>

/* Arm-2D scene switch CFG values (mirrors arm_2d_helper_scene.h).
 * Avoid pulling the full Arm-2D helper header into core; keep a local
 * static map that matches ARM_2D_SCENE_SWITCH_CFG_* ordinals. */
enum {
    TINYUI_ARM2D_SWITCH_NONE = 0,
    TINYUI_ARM2D_SWITCH_USER = 1,
    TINYUI_ARM2D_SWITCH_FADE_WHITE = 2,
    TINYUI_ARM2D_SWITCH_FADE_BLACK = 3,
    TINYUI_ARM2D_SWITCH_SLIDE_LEFT = 4,
    TINYUI_ARM2D_SWITCH_SLIDE_RIGHT = 5,
    TINYUI_ARM2D_SWITCH_SLIDE_UP = 6,
    TINYUI_ARM2D_SWITCH_SLIDE_DOWN = 7,
    TINYUI_ARM2D_SWITCH_ERASE_LEFT = 8,
    TINYUI_ARM2D_SWITCH_ERASE_RIGHT = 9,
    TINYUI_ARM2D_SWITCH_ERASE_UP = 10,
    TINYUI_ARM2D_SWITCH_ERASE_DOWN = 11,
    TINYUI_ARM2D_SWITCH_FLY_IN_LEFT = 12,
    TINYUI_ARM2D_SWITCH_FLY_IN_RIGHT = 13,
    TINYUI_ARM2D_SWITCH_FLY_IN_TOP = 14,
    TINYUI_ARM2D_SWITCH_FLY_IN_BOTTOM = 15
};

static const int s_tinyui_screen_transition_to_arm2d[] = {
    [TINYUI_SCREEN_TRANSITION_NONE] = TINYUI_ARM2D_SWITCH_NONE,
    [TINYUI_SCREEN_TRANSITION_FADE_WHITE] = TINYUI_ARM2D_SWITCH_FADE_WHITE,
    [TINYUI_SCREEN_TRANSITION_FADE_BLACK] = TINYUI_ARM2D_SWITCH_FADE_BLACK,
    [TINYUI_SCREEN_TRANSITION_SLIDE_LEFT] = TINYUI_ARM2D_SWITCH_SLIDE_LEFT,
    [TINYUI_SCREEN_TRANSITION_SLIDE_RIGHT] = TINYUI_ARM2D_SWITCH_SLIDE_RIGHT,
    [TINYUI_SCREEN_TRANSITION_SLIDE_UP] = TINYUI_ARM2D_SWITCH_SLIDE_UP,
    [TINYUI_SCREEN_TRANSITION_SLIDE_DOWN] = TINYUI_ARM2D_SWITCH_SLIDE_DOWN,
    [TINYUI_SCREEN_TRANSITION_ERASE_LEFT] = TINYUI_ARM2D_SWITCH_ERASE_LEFT,
    [TINYUI_SCREEN_TRANSITION_ERASE_RIGHT] = TINYUI_ARM2D_SWITCH_ERASE_RIGHT,
    [TINYUI_SCREEN_TRANSITION_ERASE_UP] = TINYUI_ARM2D_SWITCH_ERASE_UP,
    [TINYUI_SCREEN_TRANSITION_ERASE_DOWN] = TINYUI_ARM2D_SWITCH_ERASE_DOWN,
    [TINYUI_SCREEN_TRANSITION_FLY_IN_LEFT] = TINYUI_ARM2D_SWITCH_FLY_IN_LEFT,
    [TINYUI_SCREEN_TRANSITION_FLY_IN_RIGHT] = TINYUI_ARM2D_SWITCH_FLY_IN_RIGHT,
    [TINYUI_SCREEN_TRANSITION_FLY_IN_TOP] = TINYUI_ARM2D_SWITCH_FLY_IN_TOP,
    [TINYUI_SCREEN_TRANSITION_FLY_IN_BOTTOM] = TINYUI_ARM2D_SWITCH_FLY_IN_BOTTOM,
};

_Static_assert(
    (sizeof(s_tinyui_screen_transition_to_arm2d) /
     sizeof(s_tinyui_screen_transition_to_arm2d[0])) ==
        ((size_t)TINYUI_SCREEN_TRANSITION_FLY_IN_BOTTOM + 1U),
    "screen transition map must cover full tinyui_screen_transition_t set");

/* File-static single-instance runtime (M2 Task 1). */
static struct tinyui_runtime_state s_tinyui_runtime;

#if TINYUI_ENABLE_THEME
extern void tinyui_internal_theme_reset(void);
#else
static void tinyui_internal_theme_reset(void)
{
}
#endif

struct tinyui_runtime_state *tinyui_runtime_state_get(void)
{
    return &s_tinyui_runtime;
}

void tinyui_runtime_timer_pool_clear(struct tinyui_runtime_state *rt)
{
    if (rt == 0) {
        return;
    }
    memset(&rt->timer_pool, 0, sizeof(rt->timer_pool));
}

void tinyui_runtime_event_cb_pool_clear(struct tinyui_runtime_state *rt)
{
    if (rt == 0) {
        return;
    }
    memset(&rt->event_cb_pool, 0, sizeof(rt->event_cb_pool));
}

void tinyui_runtime_set_last_result(tinyui_result_t result)
{
    s_tinyui_runtime.last_result = result;
}

tinyui_result_t tinyui_last_result(void)
{
    return s_tinyui_runtime.last_result;
}

const char *tinyui_last_error_message(void)
{
#if TINYUI_ENABLE_DIAGNOSTICS
    if (s_tinyui_runtime.diagnostic[0] != '\0') {
        return s_tinyui_runtime.diagnostic;
    }
#endif
    switch (s_tinyui_runtime.last_result) {
    case TINYUI_OK:
        return "TINYUI_OK";
    case TINYUI_ERROR_INVALID_ARG:
        return "TINYUI_ERROR_INVALID_ARG";
    case TINYUI_ERROR_INVALID_OBJECT:
        return "TINYUI_ERROR_INVALID_OBJECT";
    case TINYUI_ERROR_INVALID_STATE:
        return "TINYUI_ERROR_INVALID_STATE";
    case TINYUI_ERROR_NOT_SUPPORTED:
        return "TINYUI_ERROR_NOT_SUPPORTED";
    case TINYUI_ERROR_OUT_OF_RANGE:
        return "TINYUI_ERROR_OUT_OF_RANGE";
    case TINYUI_ERROR_NO_MEMORY:
        return "TINYUI_ERROR_NO_MEMORY";
    case TINYUI_ERROR_CAPACITY:
        return "TINYUI_ERROR_CAPACITY";
    case TINYUI_ERROR_BACKEND:
        return "TINYUI_ERROR_BACKEND";
    default:
        return "TINYUI_ERROR_UNKNOWN";
    }
}

tinyui_result_t tinyui_init(void)
{
    struct tinyui_app *app;

    if (s_tinyui_runtime.initialized) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }

    app = tinyui_runtime_internal_app_create();
    if (app == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_NO_MEMORY);
        return TINYUI_ERROR_NO_MEMORY;
    }

    memset(&s_tinyui_runtime, 0, sizeof(s_tinyui_runtime));
    s_tinyui_runtime.backend_app = app;
    s_tinyui_runtime.initialized = true;
    s_tinyui_runtime.last_result = TINYUI_OK;
    return TINYUI_OK;
}

void tinyui_deinit(void)
{
    struct tinyui_app *app;

    /*
     * Always drop the borrowed theme pointer. theme_set() is allowed before
     * init, so deinit must clear it even when the runtime was never started;
     * otherwise a dangling caller-owned descriptor remains visible via
     * tinyui_theme_get().
     */
    tinyui_internal_theme_reset();

    if (!s_tinyui_runtime.initialized) {
        return;
    }

    app = s_tinyui_runtime.backend_app;
    s_tinyui_runtime.active_screen = 0;
    s_tinyui_runtime.theme = 0;
    s_tinyui_runtime.processing = false;
    s_tinyui_runtime.delete_target = 0;
    s_tinyui_runtime.delete_pending = 0;
    tinyui_runtime_timer_pool_clear(&s_tinyui_runtime);
    tinyui_runtime_event_cb_pool_clear(&s_tinyui_runtime);

    if (app != 0) {
        /* internal destroy walks hosts / roots / scene; no public app_* path */
        tinyui_runtime_internal_app_destroy(app);
    }

    memset(&s_tinyui_runtime, 0, sizeof(s_tinyui_runtime));
    s_tinyui_runtime.last_result = TINYUI_OK;
}

struct tinyui_app *tinyui_runtime_internal_app_current(void)
{
    return s_tinyui_runtime.backend_app;
}

tinyui_obj_t *tinyui_screen_create(void)
{
    if (!s_tinyui_runtime.initialized || s_tinyui_runtime.backend_app == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return 0;
    }

    tinyui_runtime_bridge_begin_screen_create(s_tinyui_runtime.backend_app);
    return tinyui_window_create(0);
}

tinyui_obj_t *tinyui_screen_create_with_props(const tinyui_window_props_t *props)
{
    if (props == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return 0;
    }

    if (!s_tinyui_runtime.initialized || s_tinyui_runtime.backend_app == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return 0;
    }

    return tinyui_window_create_with_props(0, props);
}

tinyui_obj_t *tinyui_background_create(void)
{
    if (!s_tinyui_runtime.initialized || s_tinyui_runtime.backend_app == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return 0;
    }

    return (tinyui_obj_t *)tinyui_legacy_background_create(
        s_tinyui_runtime.backend_app,
        "background");
}

tinyui_obj_t *tinyui_background_create_with_props(const tinyui_background_props_t *props)
{
    tinyui_obj_t *obj;
    if (props == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return 0;
    }
    obj = tinyui_background_create();
    if (obj == 0) {
        return 0;
    }
    if ((props->fields & TINYUI_BACKGROUND_FIELD_SOURCE) != 0) {
        if (tinyui_background_set_source(obj, props->source) != 0) {
            (void)tinyui_obj_delete(obj);
            return 0;
        }
    }
    if ((props->fields & TINYUI_BACKGROUND_FIELD_COLOR) != 0) {
        if (tinyui_background_set_color(obj, props->color) != 0) {
            (void)tinyui_obj_delete(obj);
            return 0;
        }
    }
    if ((props->fields & (TINYUI_BACKGROUND_FIELD_OFFSET_X | TINYUI_BACKGROUND_FIELD_OFFSET_Y)) != 0) {
        int ox = ((props->fields & TINYUI_BACKGROUND_FIELD_OFFSET_X) != 0) ? props->offset_x : 0;
        int oy = ((props->fields & TINYUI_BACKGROUND_FIELD_OFFSET_Y) != 0) ? props->offset_y : 0;
        if (tinyui_background_set_offset(obj, ox, oy) != 0) {
            (void)tinyui_obj_delete(obj);
            return 0;
        }
    }
    (void)props->id;
    return obj;
}

static int tinyui_runtime_map_screen_transition(tinyui_screen_transition_t transition,
                                                int *mode_out)
{
    if (mode_out == 0) {
        return -1;
    }
    if ((unsigned int)transition >
        (unsigned int)TINYUI_SCREEN_TRANSITION_FLY_IN_BOTTOM) {
        return -1;
    }
    *mode_out = s_tinyui_screen_transition_to_arm2d[transition];
    return 0;
}

tinyui_result_t tinyui_screen_load(tinyui_obj_t *screen,
                                   tinyui_screen_transition_t transition,
                                   uint32_t duration_ms)
{
    struct tinyui_app *app;
    struct tinyui_window *window;
    int mode = TINYUI_ARM2D_SWITCH_NONE;
    int switch_rc;

    if (!s_tinyui_runtime.initialized || s_tinyui_runtime.backend_app == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }
    if (screen == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_OBJECT);
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (tinyui_runtime_map_screen_transition(transition, &mode) != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_OUT_OF_RANGE);
        return TINYUI_ERROR_OUT_OF_RANGE;
    }

    app = s_tinyui_runtime.backend_app;
    window = (struct tinyui_window *)screen;
    if (!tinyui_runtime_bridge_window_is_owned_by(app, window)) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_OBJECT);
        return TINYUI_ERROR_INVALID_OBJECT;
    }

    /* Internal backend scene switch only — no public tinyui_app_* path. */
    if (transition == TINYUI_SCREEN_TRANSITION_NONE) {
        switch_rc = tinyui_runtime_internal_app_set_window(app, window);
        if (switch_rc == 0) {
            tinyui_runtime_bridge_reset_window_switch(app);
        }
    } else {
        switch_rc = tinyui_runtime_internal_app_switch_window(app,
                                                              window,
                                                              mode,
                                                              duration_ms);
    }

    if (switch_rc != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return TINYUI_ERROR_BACKEND;
    }

    s_tinyui_runtime.active_screen = screen;
    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

tinyui_obj_t *tinyui_screen_active(void)
{
    if (!s_tinyui_runtime.initialized || s_tinyui_runtime.backend_app == 0) {
        return 0;
    }
    return s_tinyui_runtime.active_screen;
}

static bool tinyui_time_reached(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}

static uint32_t tinyui_timer_clock_now(struct tinyui_runtime_state *rt)
{
    if (rt == 0) {
        return 0U;
    }
    if (rt->processing) {
        return rt->now_ms;
    }
    if (rt->backend_app != 0) {
        return (uint32_t)tinyui_tick_get(rt->backend_app);
    }
    return rt->now_ms;
}

static bool tinyui_timer_slot_is_valid(const tinyui_timer_t *timer)
{
    const struct tinyui_timer_pool *pool;
    uintptr_t base;
    uintptr_t addr;
    uintptr_t index;

    if (timer == 0 || !s_tinyui_runtime.initialized) {
        return false;
    }
    pool = &s_tinyui_runtime.timer_pool;
    base = (uintptr_t)(const void *)pool->slots;
    addr = (uintptr_t)(const void *)timer;
    if (addr < base) {
        return false;
    }
    if ((addr - base) % sizeof(pool->slots[0]) != 0U) {
        return false;
    }
    index = (addr - base) / sizeof(pool->slots[0]);
    if (index >= (uintptr_t)TINYUI_TIMER_CAPACITY) {
        return false;
    }
    return timer->allocated != 0U;
}

static void tinyui_timer_slot_release(tinyui_timer_t *timer)
{
    struct tinyui_timer_pool *pool = &s_tinyui_runtime.timer_pool;

    if (timer == 0 || timer->allocated == 0U) {
        return;
    }
    timer->allocated = 0U;
    timer->running = 0U;
    timer->deleting = 0U;
    timer->cb = 0;
    timer->user_data = 0;
    timer->interval_ms = 0U;
    timer->deadline_ms = 0U;
    timer->born_epoch = 0U;
    if (pool->active_count > 0U) {
        pool->active_count -= 1U;
    }
}

/* Dispatch due timers and compute nearest remaining deadline. No heap. */
static void tinyui_timer_process(uint32_t now_ms, uint32_t *next_ms)
{
    struct tinyui_runtime_state *rt = &s_tinyui_runtime;
    struct tinyui_timer_pool *pool = &rt->timer_pool;
    uint16_t epoch;
    uint32_t nearest = UINT32_MAX;
    size_t i;

    rt->now_ms = now_ms;
    epoch = (uint16_t)(rt->dispatch_epoch + 1U);
    if (epoch == 0U) {
        epoch = 1U;
    }
    rt->dispatch_epoch = epoch;

    for (i = 0; i < (size_t)TINYUI_TIMER_CAPACITY; ++i) {
        tinyui_timer_t *timer = &pool->slots[i];
        uint16_t generation;
        tinyui_timer_cb_t cb;
        void *user_data;

        if (timer->allocated == 0U || timer->running == 0U || timer->cb == 0) {
            continue;
        }
        if (!(timer->born_epoch < epoch)) {
            continue;
        }
        if (!tinyui_time_reached(now_ms, timer->deadline_ms)) {
            continue;
        }

        generation = timer->generation;
        cb = timer->cb;
        user_data = timer->user_data;

        if (timer->repeat != 0U) {
            timer->deadline_ms = now_ms + timer->interval_ms;
        } else {
            timer->running = 0U;
        }

        cb(timer, user_data);

        /* Mutation-safe: stop/delete in callback must not reschedule. */
        if (timer->allocated == 0U || timer->generation != generation) {
            continue;
        }
        if (timer->deleting != 0U) {
            tinyui_timer_slot_release(timer);
            continue;
        }
        if (timer->running == 0U || timer->cb == 0) {
            continue;
        }
    }

    for (i = 0; i < (size_t)TINYUI_TIMER_CAPACITY; ++i) {
        const tinyui_timer_t *timer = &pool->slots[i];
        uint32_t remaining;

        if (timer->allocated == 0U || timer->running == 0U) {
            continue;
        }
        if (tinyui_time_reached(now_ms, timer->deadline_ms)) {
            nearest = 0U;
            break;
        }
        remaining = timer->deadline_ms - now_ms;
        if (remaining < nearest) {
            nearest = remaining;
        }
    }

    if (next_ms != 0) {
        *next_ms = nearest;
    }
}

tinyui_result_t tinyui_process(uint32_t *next_ms)
{
    int step;
    uint32_t now_ms;
    uint32_t timer_next = UINT32_MAX;

    if (!s_tinyui_runtime.initialized || s_tinyui_runtime.backend_app == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }
    if (s_tinyui_runtime.processing) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }

    s_tinyui_runtime.processing = true;
    now_ms = (uint32_t)tinyui_tick_get(s_tinyui_runtime.backend_app);
    s_tinyui_runtime.now_ms = now_ms;

    /* Backend frame step (no sleep / no alloc). */
    step = tinyui_runtime_bridge_step_app(s_tinyui_runtime.backend_app);

    /* Fixed-pool timer dispatch + nearest deadline (Task 3). */
    tinyui_timer_process(now_ms, &timer_next);

    /* Leave processing before flush so nested delete is synchronous; run the
     * same destroy helper as sync delete (not tinyui_obj_delete, which would
     * re-defer while a slot is still conceptually mid-dispatch). */
    s_tinyui_runtime.processing = false;
    if (s_tinyui_runtime.delete_pending != 0 && s_tinyui_runtime.delete_target != 0) {
        tinyui_obj_t *delete_target = s_tinyui_runtime.delete_target;
        struct tinyui_widget *delete_widget =
            (struct tinyui_widget *)(void *)delete_target;

        s_tinyui_runtime.delete_pending = 0;
        s_tinyui_runtime.delete_target = 0;
        (void)tinyui_runtime_internal_widget_destroy(delete_widget);
    } else {
        s_tinyui_runtime.delete_pending = 0;
        s_tinyui_runtime.delete_target = 0;
    }

    if (next_ms != 0) {
        *next_ms = timer_next;
    }

    if (step < 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return TINYUI_ERROR_BACKEND;
    }

    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

tinyui_timer_t *tinyui_timer_create(uint32_t interval_ms,
                                    bool repeat,
                                    tinyui_timer_cb_t cb,
                                    void *user_data)
{
    struct tinyui_timer_pool *pool;
    tinyui_timer_t *slot = 0;
    size_t i;
    uint16_t generation;

    if (!s_tinyui_runtime.initialized) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return NULL;
    }
    if (interval_ms == 0U || cb == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return NULL;
    }

    pool = &s_tinyui_runtime.timer_pool;
    for (i = 0; i < (size_t)TINYUI_TIMER_CAPACITY; ++i) {
        if (pool->slots[i].allocated == 0U) {
            slot = &pool->slots[i];
            break;
        }
    }
    if (slot == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return NULL;
    }

    generation = (uint16_t)(pool->next_generation + 1U);
    if (generation == 0U) {
        generation = 1U;
    }
    pool->next_generation = generation;

    memset(slot, 0, sizeof(*slot));
    slot->interval_ms = interval_ms;
    slot->deadline_ms = 0U;
    slot->cb = cb;
    slot->user_data = user_data;
    slot->generation = generation;
    slot->born_epoch = s_tinyui_runtime.dispatch_epoch;
    slot->allocated = 1U;
    slot->running = 0U;
    slot->repeat = repeat ? 1U : 0U;
    slot->deleting = 0U;
    pool->active_count = (uint16_t)(pool->active_count + 1U);

    tinyui_runtime_set_last_result(TINYUI_OK);
    return slot;
}

tinyui_result_t tinyui_timer_start(tinyui_timer_t *timer)
{
    uint32_t now_ms;

    if (!tinyui_timer_slot_is_valid(timer) || timer->cb == 0 ||
        timer->interval_ms == 0U) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return TINYUI_ERROR_INVALID_ARG;
    }

    now_ms = tinyui_timer_clock_now(&s_tinyui_runtime);
    /*
     * Outside process: first fire after a full interval.
     * During process (e.g. create-in-callback): arm as already due so the
     * current round reports next_ms=0, but born_epoch blocks same-round fire.
     */
    if (s_tinyui_runtime.processing) {
        timer->deadline_ms = now_ms;
    } else {
        timer->deadline_ms = now_ms + timer->interval_ms;
    }
    timer->running = 1U;
    timer->deleting = 0U;
    timer->born_epoch = s_tinyui_runtime.dispatch_epoch;

    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

tinyui_result_t tinyui_timer_stop(tinyui_timer_t *timer)
{
    if (!tinyui_timer_slot_is_valid(timer)) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return TINYUI_ERROR_INVALID_ARG;
    }

    timer->running = 0U;
    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

tinyui_result_t tinyui_timer_set_interval(tinyui_timer_t *timer, uint32_t interval_ms)
{
    if (!tinyui_timer_slot_is_valid(timer)) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return TINYUI_ERROR_INVALID_ARG;
    }
    if (interval_ms == 0U) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return TINYUI_ERROR_INVALID_ARG;
    }

    timer->interval_ms = interval_ms;
    if (timer->running != 0U) {
        uint32_t now_ms = tinyui_timer_clock_now(&s_tinyui_runtime);
        if (s_tinyui_runtime.processing) {
            timer->deadline_ms = now_ms;
        } else {
            timer->deadline_ms = now_ms + interval_ms;
        }
    }

    tinyui_runtime_set_last_result(TINYUI_OK);
    return TINYUI_OK;
}

void tinyui_timer_delete(tinyui_timer_t *timer)
{
    if (!tinyui_timer_slot_is_valid(timer)) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return;
    }

    if (s_tinyui_runtime.processing) {
        /* Safe mid-dispatch: mark and release after callback re-check, or
         * release immediately if not the timer currently in cb (other slots). */
        timer->running = 0U;
        timer->deleting = 1U;
        timer->cb = 0;
        tinyui_timer_slot_release(timer);
    } else {
        tinyui_timer_slot_release(timer);
    }
    tinyui_runtime_set_last_result(TINYUI_OK);
}

int tinyui_runtime_internal_timer_handler(void)
{
    int step;

    if (!s_tinyui_runtime.initialized || s_tinyui_runtime.backend_app == 0) {
        return -1;
    }

    step = tinyui_runtime_bridge_step_app(s_tinyui_runtime.backend_app);
    if (step < 0) {
        return -1;
    }
    if (step > 0) {
        return 1;
    }
    return 0;
}
