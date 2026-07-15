#include "core/event.h"
#include "core/runtime.h"
#include "core/timer.h"
#include "internal.h"
#include "tick/tick.h"
#include "tinyui.h"

#include "../../../src/gui/ldBase.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

struct tinyui_test_allocator_stats {
    size_t alloc_calls;
    size_t calloc_calls;
    size_t realloc_calls;
    size_t free_calls;
    size_t bytes_requested;
};

void tinyui_test_allocator_reset(void);
struct tinyui_test_allocator_stats tinyui_test_allocator_snapshot(void);
void *tinyui_test_allocator_malloc(uint32_t size);
void *tinyui_test_allocator_calloc(uint32_t num, uint32_t size);
void *tinyui_test_allocator_realloc(void *ptr, uint32_t size);
void tinyui_test_allocator_free(void *ptr);

void *ldMalloc(uint32_t size)
{
    return tinyui_test_allocator_malloc(size);
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    return tinyui_test_allocator_calloc(num, size);
}

void *ldRealloc(void *ptr, uint32_t size)
{
    return tinyui_test_allocator_realloc(ptr, size);
}

void ldFree(void *ptr)
{
    tinyui_test_allocator_free(ptr);
}

static uint32_t s_fake_now_ms;
static unsigned int s_timer_calls;
static unsigned int s_event_calls;

static unsigned int fake_tick_get(void *user_data)
{
    (void)user_data;
    return (unsigned int)s_fake_now_ms;
}

static void install_fake_clock(void)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_current();

    assert(app != NULL);
    s_fake_now_ms = 0U;
    assert(tinyui_tick_set_source(app, fake_tick_get, NULL) == 0);
}

static void set_now(uint32_t now_ms)
{
    s_fake_now_ms = now_ms;
}

static void assert_zero_allocations(const struct tinyui_test_allocator_stats *stats)
{
    assert(stats->alloc_calls == 0U);
    assert(stats->calloc_calls == 0U);
    assert(stats->realloc_calls == 0U);
    assert(stats->free_calls == 0U);
    assert(stats->bytes_requested == 0U);
}

static void assert_zero_alloc_delta(const struct tinyui_test_allocator_stats *before,
                                    const struct tinyui_test_allocator_stats *after)
{
    assert(after->alloc_calls == before->alloc_calls);
    assert(after->calloc_calls == before->calloc_calls);
    assert(after->realloc_calls == before->realloc_calls);
    assert(after->free_calls == before->free_calls);
    assert(after->bytes_requested == before->bytes_requested);
}

static void timer_count_cb(tinyui_timer_t *timer, void *user_data)
{
    unsigned int *counter = (unsigned int *)user_data;

    (void)timer;
    assert(counter != NULL);
    *counter += 1U;
}

static void event_count_cb(const tinyui_event_t *event)
{
    (void)event;
    s_event_calls += 1U;
}

static void inject_clicked(tinyui_obj_t *obj)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)obj;

    assert(widget != NULL);
    assert(widget->ld_widget != NULL);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(widget,
                                                                  SIGNAL_PRESS,
                                                                  0) == 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(widget,
                                                                  SIGNAL_RELEASE,
                                                                  0) == 0);
}

static void assert_process_rounds_zero_delta(uint32_t *next_ms, unsigned int rounds)
{
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;
    unsigned int round;

    before = tinyui_test_allocator_snapshot();
    for (round = 0U; round < rounds; ++round) {
        assert(tinyui_process(next_ms) == TINYUI_OK);
        after = tinyui_test_allocator_snapshot();
        assert_zero_alloc_delta(&before, &after);
    }
}

int main(void)
{
    tinyui_obj_t *screen;
    tinyui_obj_t *label;
    tinyui_obj_t *button;
    tinyui_obj_t *checkbox;
    tinyui_obj_t *slider;
    tinyui_timer_t *timer;
    tinyui_event_handle_t handle = 0U;
    struct tinyui_test_allocator_stats stats;
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;
    uint32_t next_ms = UINT32_MAX;
    unsigned int round;

    /* ── idle process after screen load ─────────────────────────────────── */
    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) == TINYUI_OK);
    assert(tinyui_process(&next_ms) == TINYUI_OK);

    tinyui_test_allocator_reset();
    for (round = 0U; round < 100U; ++round) {
        assert(tinyui_process(&next_ms) == TINYUI_OK);
        stats = tinyui_test_allocator_snapshot();
        assert_zero_allocations(&stats);
    }
    printf("TINYUI_STEADY_STATE_IDLE_PROCESS=0\n");

    /* ── process with four sample widgets already created ───────────────── */
    label = tinyui_label_create(screen);
    button = tinyui_button_create(screen);
    checkbox = tinyui_checkbox_create(screen);
    slider = tinyui_slider_create(screen);
    assert(label != NULL);
    assert(button != NULL);
    assert(checkbox != NULL);
    assert(slider != NULL);

    tinyui_test_allocator_reset();
    assert_process_rounds_zero_delta(&next_ms, 50U);
    printf("TINYUI_STEADY_STATE_WIDGETS_PROCESS=0\n");

    /* ── timer expire path ──────────────────────────────────────────────── */
    s_timer_calls = 0U;
    timer = tinyui_timer_create(10U, true, timer_count_cb, &s_timer_calls);
    assert(timer != NULL);
    assert(tinyui_timer_start(timer) == TINYUI_OK);

    set_now(0U);
    assert(tinyui_process(&next_ms) == TINYUI_OK);
    assert(s_timer_calls == 0U);

    tinyui_test_allocator_reset();
    set_now(10U);
    before = tinyui_test_allocator_snapshot();
    assert(tinyui_process(&next_ms) == TINYUI_OK);
    after = tinyui_test_allocator_snapshot();
    assert_zero_alloc_delta(&before, &after);
    assert(s_timer_calls >= 1U);

    set_now(20U);
    before = tinyui_test_allocator_snapshot();
    assert(tinyui_process(&next_ms) == TINYUI_OK);
    after = tinyui_test_allocator_snapshot();
    assert_zero_alloc_delta(&before, &after);
    assert(s_timer_calls >= 2U);
    printf("TINYUI_STEADY_STATE_TIMER_DISPATCH=0\n");

    /* ── event dispatch path ────────────────────────────────────────────── */
    s_event_calls = 0U;
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   event_count_cb,
                                   NULL,
                                   &handle) == TINYUI_OK);
    assert(handle != 0U);

    /* First click may promote born_epoch; measure after callbacks are live. */
    inject_clicked(button);
    assert(s_event_calls >= 1U);

    tinyui_test_allocator_reset();
    before = tinyui_test_allocator_snapshot();
    inject_clicked(button);
    after = tinyui_test_allocator_snapshot();
    assert_zero_alloc_delta(&before, &after);
    assert(s_event_calls >= 2U);

    before = tinyui_test_allocator_snapshot();
    inject_clicked(button);
    after = tinyui_test_allocator_snapshot();
    assert_zero_alloc_delta(&before, &after);
    assert(s_event_calls >= 3U);
    printf("TINYUI_STEADY_STATE_EVENT_DISPATCH=0\n");

    stats = tinyui_test_allocator_snapshot();
    assert_zero_allocations(&stats);
    printf("TINYUI_STEADY_STATE_ALLOCATIONS=0\n");

    tinyui_timer_delete(timer);
    tinyui_deinit();
    return 0;
}
