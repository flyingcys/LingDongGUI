#include "picoui/app.h"
#include "internal.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

extern void picoui_backend_test_pump_timers(struct picoui_app *app, unsigned int now_ticks);

static void require_condition(int condition)
{
    if (!condition) {
        abort();
    }
}

struct timer_callback_probe {
    int call_count;
    struct picoui_app *last_app;
    struct picoui_app_timer *last_timer;
    void *last_user_data;
};

struct timer_relink_chain_probe {
    int call_count;
    struct picoui_app_timer *resume_timer;
};

static void timer_probe_callback(struct picoui_app *app,
                                 struct picoui_app_timer *timer,
                                 void *user_data)
{
    struct timer_callback_probe *probe = (struct timer_callback_probe *)user_data;

    if (probe == NULL) {
        return;
    }

    probe->call_count += 1;
    probe->last_app = app;
    probe->last_timer = timer;
    probe->last_user_data = user_data;
}

static void timer_relink_chain_callback(struct picoui_app *app,
                                        struct picoui_app_timer *timer,
                                        void *user_data)
{
    struct timer_relink_chain_probe *probe = (struct timer_relink_chain_probe *)user_data;

    if (probe == NULL) {
        return;
    }

    probe->call_count += 1;
    if (app != NULL) {
        app->timers = probe->resume_timer;
    }
    if (probe->resume_timer != NULL) {
        probe->resume_timer->next = NULL;
    }
    picoui_app_timer_destroy(timer);
}

static void timer_must_not_fire_callback(struct picoui_app *app,
                                         struct picoui_app_timer *timer,
                                         void *user_data)
{
    (void)app;
    (void)timer;
    (void)user_data;
    abort();
}

static void test_timer_rejects_null_app(void)
{
    assert(picoui_app_timer_create(NULL) == NULL);
}

static void test_timer_callback_contract_shape(void)
{
    struct timer_callback_probe probe = {0};
    picoui_app_timer_cb_t callback = timer_probe_callback;

    callback(NULL, NULL, &probe);
    assert(probe.call_count == 1);
    assert(probe.last_app == NULL);
    assert(probe.last_timer == NULL);
    assert(probe.last_user_data == &probe);
}

static void test_timer_running_state_contract(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_app_timer *timer;

    assert(app != NULL);
    timer = picoui_app_timer_create(app);
    assert(timer != NULL);

    assert(picoui_app_timer_is_running(timer) == 0);
    require_condition(picoui_app_timer_start(timer, 100, 1, timer_probe_callback, NULL) == 0);
    require_condition(picoui_app_timer_is_running(timer) == 1);
    require_condition(picoui_app_timer_stop(timer) == 0);
    require_condition(picoui_app_timer_is_running(timer) == 0);

    picoui_app_timer_destroy(timer);
    picoui_app_destroy(app);
}

static void test_timer_start_rejects_invalid_arguments(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_app_timer *timer;

    assert(app != NULL);
    timer = picoui_app_timer_create(app);
    assert(timer != NULL);

    assert(picoui_app_timer_start(timer, 0, 1, timer_probe_callback, NULL) == -1);
    assert(picoui_app_timer_start(timer, 100, 1, NULL, NULL) == -1);

    picoui_app_timer_destroy(timer);
    picoui_app_destroy(app);
}

static void test_timer_destroy_after_stop_is_safe(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_app_timer *timer;
    struct timer_callback_probe probe = {0};

    assert(app != NULL);
    timer = picoui_app_timer_create(app);
    assert(timer != NULL);

    assert(picoui_app_timer_start(timer, 100, 1, timer_probe_callback, &probe) == 0);
    assert(picoui_app_timer_stop(timer) == 0);
    picoui_app_timer_destroy(timer);
    picoui_app_destroy(app);
}

static void test_app_destroy_cleans_residual_timers(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_app_timer *timer1;
    struct picoui_app_timer *timer2;
    struct timer_callback_probe probe = {0};

    assert(app != NULL);
    timer1 = picoui_app_timer_create(app);
    timer2 = picoui_app_timer_create(app);
    assert(timer1 != NULL);
    assert(timer2 != NULL);

    require_condition(picoui_app_timer_start(timer1, 100, 1, timer_probe_callback, &probe) == 0);
    require_condition(picoui_app_timer_is_running(timer1) == 1);
    require_condition(picoui_app_timer_is_running(timer2) == 0);
    require_condition(app->timers != NULL);

    picoui_app_destroy(app);
}

static void test_repeating_timer_pump_keeps_running(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_app_timer *timer;
    struct timer_callback_probe probe = {0};

    assert(app != NULL);
    timer = picoui_app_timer_create(app);
    assert(timer != NULL);

    require_condition(picoui_app_timer_start(timer, 50, 1, timer_probe_callback, &probe) == 0);

    assert(probe.call_count == 0);
    picoui_backend_test_pump_timers(app, 1000);
    picoui_backend_test_pump_timers(app, 1050);
    assert(probe.call_count == 1);
    assert(picoui_app_timer_is_running(timer) == 1);

    picoui_backend_test_pump_timers(app, 1100);
    assert(probe.call_count == 2);
    assert(picoui_app_timer_is_running(timer) == 1);

    picoui_app_timer_destroy(timer);
    picoui_app_destroy(app);
}

static void test_one_shot_timer_pump_stops_after_fire(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_app_timer *timer;
    struct timer_callback_probe probe = {0};

    assert(app != NULL);
    timer = picoui_app_timer_create(app);
    assert(timer != NULL);

    require_condition(picoui_app_timer_start(timer, 50, 0, timer_probe_callback, &probe) == 0);

    assert(probe.call_count == 0);
    picoui_backend_test_pump_timers(app, 2000);
    picoui_backend_test_pump_timers(app, 2050);
    assert(probe.call_count == 1);
    assert(picoui_app_timer_is_running(timer) == 0);

    picoui_backend_test_pump_timers(app, 2100);
    assert(probe.call_count == 1);
    assert(picoui_app_timer_is_running(timer) == 0);

    picoui_app_timer_destroy(timer);
    picoui_app_destroy(app);
}

static void test_timer_pump_skips_detached_successor_after_callback_relink(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_app_timer *tail_timer;
    struct picoui_app_timer *middle_timer;
    struct picoui_app_timer *head_timer;
    struct timer_callback_probe tail_probe = {0};
    struct timer_relink_chain_probe head_probe = {0};

    assert(app != NULL);
    tail_timer = picoui_app_timer_create(app);
    middle_timer = picoui_app_timer_create(app);
    head_timer = picoui_app_timer_create(app);
    assert(tail_timer != NULL);
    assert(middle_timer != NULL);
    assert(head_timer != NULL);

    head_probe.resume_timer = tail_timer;

    require_condition(picoui_app_timer_start(tail_timer, 50, 0, timer_probe_callback, &tail_probe) == 0);
    require_condition(picoui_app_timer_start(middle_timer, 50, 0, timer_must_not_fire_callback, NULL) == 0);
    require_condition(picoui_app_timer_start(head_timer, 50, 0, timer_relink_chain_callback, &head_probe) == 0);

    picoui_backend_test_pump_timers(app, 3000);
    picoui_backend_test_pump_timers(app, 3050);

    assert(head_probe.call_count == 1);
    assert(tail_probe.call_count == 0);

    picoui_backend_test_pump_timers(app, 3100);
    assert(tail_probe.call_count == 1);

    picoui_app_timer_destroy(middle_timer);
    picoui_app_timer_destroy(tail_timer);
    picoui_app_destroy(app);
}

int main(void)
{
    test_timer_rejects_null_app();
    test_timer_callback_contract_shape();
    test_timer_running_state_contract();
    test_timer_start_rejects_invalid_arguments();
    test_timer_destroy_after_stop_is_safe();
    test_app_destroy_cleans_residual_timers();
    test_repeating_timer_pump_keeps_running();
    test_one_shot_timer_pump_stops_after_fire();
    test_timer_pump_skips_detached_successor_after_callback_relink();
    return 0;
}
