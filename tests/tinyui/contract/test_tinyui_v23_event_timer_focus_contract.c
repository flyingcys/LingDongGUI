#include "core/event.h"
#include "core/focus.h"
#include "core/runtime.h"
#include "core/timer.h"
#include "integration/input.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static void test_event_callback(const tinyui_event_t *event)
{
    (void)event;
}

static void test_timer_callback(tinyui_timer_t *timer, void *user_data)
{
    (void)timer;
    (void)user_data;
}

static void test_public_contract(void)
{
    tinyui_result_t (*add_cb)(tinyui_obj_t *, uint32_t, tinyui_event_cb_t,
                              void *, tinyui_event_handle_t *) = tinyui_obj_add_event_cb;
    tinyui_result_t (*remove_cb)(tinyui_obj_t *, tinyui_event_handle_t) =
        tinyui_obj_remove_event_cb;
    tinyui_timer_t *(*create_timer)(uint32_t, bool, tinyui_timer_cb_t, void *) =
        tinyui_timer_create;
    tinyui_result_t (*start_timer)(tinyui_timer_t *) = tinyui_timer_start;
    tinyui_result_t (*stop_timer)(tinyui_timer_t *) = tinyui_timer_stop;
    tinyui_result_t (*set_interval)(tinyui_timer_t *, uint32_t) = tinyui_timer_set_interval;
    void (*delete_timer)(tinyui_timer_t *) = tinyui_timer_delete;
    tinyui_result_t (*set_focus)(tinyui_obj_t *) = tinyui_focus_set;
    tinyui_result_t (*clear_focus)(void) = tinyui_focus_clear;
    tinyui_result_t (*move_focus)(tinyui_focus_direction_t) = tinyui_focus_move;
    tinyui_obj_t *(*current_focus)(void) = tinyui_focus_current;
    tinyui_result_t (*send_key)(tinyui_key_t, bool) = tinyui_input_send_key;

    (void)add_cb;
    (void)remove_cb;
    (void)create_timer;
    (void)start_timer;
    (void)stop_timer;
    (void)set_interval;
    (void)delete_timer;
    (void)set_focus;
    (void)clear_focus;
    (void)move_focus;
    (void)current_focus;
    (void)send_key;

    _Static_assert(TINYUI_EVENT_CB_CAPACITY == 16, "event callback capacity changed");
    _Static_assert(TINYUI_TIMER_CAPACITY == 16, "timer capacity changed");
    _Static_assert(sizeof(tinyui_event_handle_t) == 4, "event handle must be 32-bit");
    _Static_assert(TINYUI_EVENT_DELETE == 7, "event code set changed");
    _Static_assert(TINYUI_KEY_PREVIOUS == 7, "key set changed");
    _Static_assert(TINYUI_FOCUS_DOWN == 5, "focus direction set changed");
    _Static_assert(TINYUI_EVENT_MASK_ALL == UINT32_MAX, "event mask all changed");

    (void)test_event_callback;
    (void)test_timer_callback;
}

static void test_fail_closed_and_backend_focus(void)
{
    tinyui_event_handle_t handle = 0xFFFFFFFFU;
    uint32_t next_ms = UINT32_MAX;
    tinyui_obj_t *screen;

    assert(tinyui_init() == TINYUI_OK);
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) == TINYUI_OK);

    /* M2 Task 4: fixed event callback pool is live. */
    assert(tinyui_obj_add_event_cb(screen, TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   test_event_callback, NULL, &handle) == TINYUI_OK);
    assert(handle != 0);
    assert((handle & UINT32_C(0xffff)) != 0U);
    assert(tinyui_last_result() == TINYUI_OK);
    assert(tinyui_obj_remove_event_cb(screen, handle) == TINYUI_OK);
    assert(tinyui_obj_remove_event_cb(screen, handle) == TINYUI_ERROR_INVALID_ARG);

    /* M2 Task 3: fixed timer pool is live. */
    {
        tinyui_timer_t *timer =
            tinyui_timer_create(10, true, test_timer_callback, NULL);
        assert(timer != NULL);
        assert(tinyui_last_result() == TINYUI_OK);
        assert(tinyui_timer_start(timer) == TINYUI_OK);
        assert(tinyui_timer_stop(timer) == TINYUI_OK);
        assert(tinyui_timer_set_interval(timer, 20) == TINYUI_OK);
        tinyui_timer_delete(timer);
        assert(tinyui_timer_start(NULL) == TINYUI_ERROR_INVALID_ARG);
        assert(tinyui_timer_stop(NULL) == TINYUI_ERROR_INVALID_ARG);
        assert(tinyui_timer_set_interval(NULL, 10) == TINYUI_ERROR_INVALID_ARG);
        tinyui_timer_delete(NULL);
    }

    assert(tinyui_focus_set(screen) == TINYUI_OK);
    assert(tinyui_focus_current() == screen);
    assert(tinyui_input_send_key(TINYUI_KEY_LEFT, true) == TINYUI_OK);
    assert(tinyui_focus_current() == screen);
    assert(tinyui_input_send_key(TINYUI_KEY_ENTER, false) == TINYUI_OK);
    assert(tinyui_focus_move(TINYUI_FOCUS_LEFT) == TINYUI_OK);
    assert(tinyui_focus_clear() == TINYUI_OK);
    assert(tinyui_focus_current() == NULL);
    assert(tinyui_input_send_key(TINYUI_KEY_NEXT, true) == TINYUI_ERROR_NOT_SUPPORTED);
    assert(tinyui_process(&next_ms) == TINYUI_OK);

    tinyui_deinit();
}

int main(void)
{
    test_public_contract();
    test_fail_closed_and_backend_focus();
    return 0;
}
