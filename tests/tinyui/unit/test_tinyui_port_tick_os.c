#include "app.h"
#include "osal.h"
#include "tick.h"

#include <assert.h>
#include <stddef.h>

struct tick_probe {
    unsigned int value;
    int calls;
};

struct lock_probe {
    int enter_calls;
    int leave_calls;
};

struct delay_probe {
    unsigned int last_ms;
    int calls;
};

static unsigned int tick_source(void *user_data)
{
    struct tick_probe *probe = (struct tick_probe *)user_data;

    assert(probe != NULL);
    probe->calls += 1;
    return probe->value;
}

static void lock_enter(void *user_data)
{
    struct lock_probe *probe = (struct lock_probe *)user_data;

    assert(probe != NULL);
    probe->enter_calls += 1;
}

static void lock_leave(void *user_data)
{
    struct lock_probe *probe = (struct lock_probe *)user_data;

    assert(probe != NULL);
    probe->leave_calls += 1;
}

static void delay_callback(unsigned int ms, void *user_data)
{
    struct delay_probe *probe = (struct delay_probe *)user_data;

    assert(probe != NULL);
    probe->calls += 1;
    probe->last_ms = ms;
}

static void test_tick_defaults_and_custom_source(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tick_probe probe = {
        .value = 1234,
        .calls = 0,
    };

    assert(app != NULL);
    assert(tinyui_tick_get(app) == 0);
    assert(tinyui_tick_set_source(app, tick_source, &probe) == 0);
    assert(tinyui_tick_get(app) == 1234);
    assert(probe.calls == 1);

    tinyui_app_destroy(app);
}

static void test_os_lock_and_delay_callbacks(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct lock_probe lock_probe_state = {0};
    struct delay_probe delay_probe_state = {0};

    assert(app != NULL);
    assert(tinyui_os_set_lock_callbacks(app, lock_enter, lock_leave, &lock_probe_state) == 0);
    tinyui_os_enter(app);
    tinyui_os_leave(app);
    assert(lock_probe_state.enter_calls == 1);
    assert(lock_probe_state.leave_calls == 1);

    assert(tinyui_os_set_delay_callback(app, delay_callback, &delay_probe_state) == 0);
    tinyui_os_delay(app, 16);
    assert(delay_probe_state.calls == 1);
    assert(delay_probe_state.last_ms == 16);

    tinyui_app_destroy(app);
}

static void test_tick_os_reject_invalid_arguments(void)
{
    struct tinyui_app *app = tinyui_app_create();

    assert(app != NULL);
    assert(tinyui_tick_set_source(NULL, tick_source, NULL) == -1);
    assert(tinyui_tick_get(NULL) == 0);
    assert(tinyui_os_set_lock_callbacks(NULL, lock_enter, lock_leave, NULL) == -1);
    assert(tinyui_os_set_delay_callback(NULL, delay_callback, NULL) == -1);

    tinyui_app_destroy(app);
}

int main(void)
{
    test_tick_defaults_and_custom_source();
    test_os_lock_and_delay_callbacks();
    test_tick_os_reject_invalid_arguments();
    return 0;
}
